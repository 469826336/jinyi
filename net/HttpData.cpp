// @file    HttpData.cpp
// @author  axunzzz
// @date    2019/11/10
// @Email   469826336@qq.com
#include "HttpData.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Util.h"
#include "Timer.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
std::unordered_map<std::string,std::string> MimeType::mime;

const __uint32_t DEFAULT_EVENT = EPOLLIN | EPOLLET | EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000; //ms
const int DEFAULT_KEEP_ALIVE_TIME = 5 * 60 * 1000; //ms

void MimeType::init()
{
	mime[".html"] = "text/html";
	mime[".avi"] = "video/x-msvideo";
	mime[".bmp"] = "image/bmp";
	mime[".c"] = "text/plain";
	mime[".doc"] = "application/msword";
	mime[".gif"] = "image/gif";
	mime[".gz"] = "application/x-gzip";
	mime[".htm"] = "text/html";
	mime[".ico"] = "image/x-icon";
	mime[".jpg"] = "image/jpeg";
	mime[".png"] = "image/png";
	mime[".txt"] = "text/plain";
	mime[".mp3"] = "audio/mp3";
	mime[".default"] = "text/html";
}


HttpData::HttpData(EventLoop *loop,int connfd)
	:loop_(loop),
	channel_(new Channel(loop,connfd)),
	fd_(connfd),
	error_(false),
	connectionState_(CONNECTED),
	method_(METHOD_GET),
	HttpVersion_(HTTP_1_1),
	nowReadPos_(0),
	state_(STATE_PARSE_URI),
	hState_(H_START),
	keepAlive_(false)
{
	channel_->setReadHandle(bind(&HttpData::handleRead,this));
	channel_->setWriteHandle(bind(&HttpData::handleWrite,this));
	channel_->setConnHandler(bind(&HttpData::handleConn,this));
}



void HttpData::handleRead()
{
	__uint32_t &events_ = channel_->getEvents();
	do 
	{
		bool zero = false;// 判断读数据是否正常
		int read_num = readn(fd_,inBuffer_,zero);

		if (connectionState_ == DISCONNECTING)
		{
			inBuffer_.clear();
			break;
		}
		if (read_num < 0)
		{
			perror("1");;
			error_ = true;
			handleError(fd_,400,"Bad Request");
			break;
		}
		else if (zero)
		{
			// 有请求出现但是读不到数据，可能是Request Aborted,
			// 或者来自网络的数据没有达到等原因
			// 最可能是对端已经关闭了，统一按照对端已经关闭处理
			connectionState_ = DISCONNECTING;
			// 可能一开始有读到数据
			if (read_num == 0)
			{
				break;
			}
		}

		if (state_ == STATE_PARSE_URI)
		{
			URIState flag = this->parseURI();
			if(flag == PARSE_URI_ERROR)
			{
				break;
			}
			else if (flag == PARSE_URI_ERROR)
			{
				perror("2");

				inBuffer_.clear();
				error_ = true;
				handleError(fd_,400,"Bad Request");
				break;
			} 
			else
			{
				state_ = STATE_PARSE_HEADERS;
			}
		}
		if (state_ == STATE_PARSE_HEADERS)
		{
			HeaderState flag = this->parseHeaders();
			if (flag ==PARSE_HEADER_AGAIN)
			{
				break;
			}
			else if (flag ==PARSE_HEADER_ERROR)
			{
				perror("3");
				error_ = true;
				handleError(fd_,400,"Bad Request");
				break;
			}
			state_ = STATE_ANALYSIS;
		}
		if (state_ == STATE_ANALYSIS)
		{
			AnalysisState flag = this->analysisRequest();
			if (flag == ANALYSIS_SUCCESS)
			{
				state_ = STATE_FINISH;
				break;
			}
			else
			{
				error_ = true;
				break;
			}
		}
	} while (false);
	
	if (!error_)
	{
		if (outBuffer_.size() > 0)
		{
			handleWrite();
		}
		// error_ may change
		if(!error_ && state_ == STATE_FINISH)
		{
			this->reset();
			if (inBuffer_.size() > 0)
			{
				if (connectionState_ != DISCONNECTING)
				{
					handleRead();
				}
			}
		}
		else if (!error_ && connectionState_ != DISCONNECTED)
		{
			events_ |= EPOLLIN;
		}
	}
}

void HttpData::handleWrite()
{
	if (!error_ && connectionState_ != DISCONNECTED)
	{
		__uint32_t &events_ = channel_->getEvents();
		if (writen(fd_,outBuffer_) < 0)
		{
			perror("writen");
			events_ = 0;
			error_ = true;
		}
		if (outBuffer_.size() > 0)
		{
			events_ |= EPOLLOUT;
		}
	}
}


void HttpData::handleConn()
{
	seperateTimer();
	__uint32_t &events_ = channel_->getEvents();
	if (!error_ && connectionState_ == CONNECTED)
	{
		if (events_ != 0)
		{
			int timeout = DEFAULT_EXPIRED_TIME;
			if (keepAlive_)
			{
				timeout = DEFAULT_KEEP_ALIVE_TIME;
			}
			if ((events_ & EPOLLIN) && (events_ & EPOLLOUT))
			{
				events_ = __uint32_t(0);
				events_ |= EPOLLOUT;
			}
			events_ |= EPOLLET;
			loop_->updatePoller(channel_,timeout);
		}
		else if (keepAlive_)
		{
			events_ |= (EPOLLIN | EPOLLET);
			int timeout = DEFAULT_KEEP_ALIVE_TIME;
			loop_->updatePoller(channel_,timeout);
		}
		else
		{
			// 这个你想怎么处理都行，我个人的理解是用户不设置，也不一定就是期望马上关闭连接的——林亚
			// events_ |= (EPOLLIN | EPOLLET);
			// int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);
			// loop_->updatePoller(channel_, timeout);
			loop_->shutdown(channel_);
			loop_->runInLoop(bind(&HttpData::handleClose,shared_from_this()));
		}
	}
	else if (!error_ && connectionState_ == DISCONNECTING && (events_ & EPOLLOUT))
	{
		events_ = (EPOLLOUT | EPOLLLET);
	}
	else
	{
		loop_->runInLoop(bind(&HttpData::handleClose,shared_from_this()));
	}
}

void HttpData::reset()
{
	fileName_.clear();
	path_.clear();
	nowReadPos_ = 0;
	state_ = STATE_PARSE_URI;
	hState_ = H_START;
	headers_.clear();
	if (timer_.lock())
	{
		shared_ptr<Timer> my_timer(timer_.lock());
		my_timer->clearReq();
		timer_.reset();
	}
}

void HttpData::seperateTimer()
{
	if (timer_.lock())
	{
		shared_ptr<Timer> my_timer(timer_.lock());
		my_timer->clearReq();
		timer_.reset();
	}
}

URIState HttpData::parseURI()
{
	string &str = inBuffer_;
	string cop =str;
	// 读到完整的请求行再开始解析请求
	size_t pos = str.find('\r',nowReadPos_);
	if (str.find('\r',nowReadPos_) == string::npos)
	{
		return PARSE_URI_AGAIN;
	}
	string request_line = str.substr(0,pos);
	// 去掉请求行所占的空间，节省空间
	if (str.size() > pos + 1)
	{
		str = str.substr(pos + 1);
	} 
	else
	{
		str.clear();
	}
	// Method
	int posGet = request_line.find("GET");
	int posHead = request_line.find("HEAD");

	if (posGet >= 0)
	{
		pos = posGet;
		method_ = METHOD_GET;
	}
	else if (posHead >= 0)
	{
		pos = posHead;
		method_ = METHOD_HEAD;
	}
	else 
	{
		return PARSE_URI_ERROR;// 只支持GET和HEAD方法
	}

	// filename
	pos = request_line.find("/",pos);
	if (request_line.find("/",pos) == string::npos)
	{
		fileName_ = "index.html";
		HttpVersion_ = HTTP_1_1;
		return PARSE_URI_SUCCESS;
	} 
	else
	{
		size_t _pos = request_line.find(' ',pos);
		if (request_line.find(' ',pos) == string::npos)
		{
			return PARSE_URI_ERROR;
		} 
		else
		{
			if (_pos - pos > 1)
			{
				fileName_ = request_line.substr(pos + 1,_pos -pos -1);
				size_t __pos = fileName_.find('?');
				if (__pos >= 0)
				{
					fileName_ = fileName_.substr(0,__pos);
				}
			}
			else
			{
				fileName_ = "index.html";
			}

		}
		pos = _pos;
	}

	// HTTP版本号
	pos = request_line.find("/",pos);
	if (request_line.find("/",pos) == string::npos)
	{
		return PARSE_URI_ERROR;
	}
	else
	{
		if (request_line.size() -pos <= 3)
		{
			return PARSE_URI_ERROR;
		}
		else
		{
			string ver = request_line.substr(pos + 1,3);
			if (ver == "1.0")
			{
				HttpVersion_ = HTTP_1_0;
			} 
			else if(ver == "1.1")
			{
				HttpVersion_ = HTTP_1_1;
			}
			else
			{
				return PARSE_URI_ERROR;
			}
		}
	}
	return PARSE_URI_SUCCESS;
}

HeaderState HttpData::parseHeaders()
{
	string &str = inBuffer_;
	int key_start = -1,key_end = -1,value_start = -1,value_end = -1;
	int now_read_line_begin = 0;
	bool notFinish = true;
	size_t i = 0;
	for (;i < str.size() && notFinish;++i)
	{
		switch(hState_)
		{
		case H_START:
			{
				if (str[i] == '\n' || str[i] == '\r')
				{
					break;
				}
				hState_ = H_KEY;
				key_start = i;
				now_read_line_begin = i;
				break;
			}
		case H_KEY:
			{
				if (str[i] == ':')
				{
					key_end = i;
					if (key_end - key_start <= 0)
					{
						return PARSE_HEADER_ERROR;
					}
					hState_ = H_COLON;
				}
				else if (str[i] == '\n' || str[i] == '\r')
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
		case H_COLON:
			{
				if (str[i] == ' ')
				{
					hState_ = H_SPACES_AFTER_COLON;
				}
				else
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
		case H_SPACES_AFTER_COLON:
			{
				hState_ = H_VALUE;
				value_start = i;
				break;
			}
		case H_VALUE:
			{
				if (str[i] == '\r')
				{
					hState_ = H_CR;
					value_end = i;
					if (value_end - value_start <=0)
					{
						return PARSE_HEADER_ERROR;
					}
				}
				else if (i - value_start > 255)
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
		case H_CR:
			{
				if (str[i] == '\n')
				{
					hState_ = H_LF;
					string key(str.begin() + key_start,str.begin() + key_end);
					string value(str.begin() + value_start,str.begin() + value_end);
					headers_[key] = value;
					now_read_line_begin = i;
				}
				else
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
		case H_LF:
			{
				if (str[i] == '\r')
				{
					hState_ = H_BLANK_LINE_CR;
				}
				else
				{
					key_start = i;
					hState_ = H_KEY;
				}
				break;
			}
		case H_BLANK_LINE_CR:
			{
				if (str[i] == '\n')
				{
					hState_ = H_BLANK_LINE_LF;
				}
				else
				{
					return PARSE_HEADER_ERROR;
				}
				break;
			}
		case  H_BLANK_LINE_LF:
			{
				notFinish = false;
				break;
			}
		}
	}
	if (hState_ == H_BLANK_LINE_LF)
	{
		str = str.substr(i);
		return PARSE_HEADER_SUCCESS;
	}
	str = str.substr(now_read_line_begin);
	return PARSE_HEADER_AGAIN;
}

AnalysisState HttpData::analysisRequest()
{
	if (method_ == METHOD_GET || method_ == METHOD_HEAD)
	{
		string header;
		header += "HTTP/1.1 200 OK\r\n";
		if (headers_.find("Connection") != headers_.end() && 
			(headers_["Connection"] == "Keep-Alive" || 
			headers_["Connection"] == "Keep-alive"))
		{
			keepAlive_ = true;
			header += string("Connection: Keep-Alive\r\n") + "Keep-Alive: timeout=" + 
				to_string(DEFAULT_KEEP_ALIVE_TIME) + "\r\n";
		}
		int dot_pos = fileName_.find('.');
		string filetype;
		if (dot_pos < 0)
		{
			filetype = MimeType::getMime("default");
		}
		else
		{
			filetype = MimeType::getMime(fileName_.substr(dot_pos));
		}

		if (fileName_ == "favicon.ico")
		{
			header += "Content-Type: image/png\r\n";
			header += "Content-Length: " + to_string(sizeof favicon) + "\r\n";
			header += "Server: axunzzz's web server\r\n";

			header += "\r\n";
			outBuffer_ += header;
			outBuffer_ += string(favicon,favicon + sizeof favicon);
			return ANALYSIS_SUCCESS;
		}

		struct stat fileInformation;
		if (stat(fileName_.c_str(),&fileInformation) < 0)
		{
			header.clear();
			handleError(fd_,404,"Not Found!");
			return ANALYSIS_ERROR;
		}
		header += "Content-Type: " + filetype + "\r\n";
		header += "Content-Length: " + to_string(fileInformation.st_size) + "\r\n";
		header += "Server: axunzzz's web server\r\n";
		// 头部结束
		header += "\r\n";
		outBuffer_ += header;
		if (method_ == METHOD_HEAD)
		{
			return ANALYSIS_SUCCESS;
		}
		// 常规文件操作需要从磁盘到页缓存再到用户主存的两次数据拷贝。而mmap操控文件，只需要从磁盘到用户主存的一次数据拷贝过程
		int src_fd = open(fileName_.c_str(),O_RDONLY,0);
		if (src_fd < 0)
		{
			outBuffer_.clear();
			handleError(fd_,404,"Not Found!");
			return ANALYSIS_ERROR;
		}
		void *mmapRet = mmap(NULL,fileInformation.st_size,PROT_READ,MAP_PRIVATE,src_fd,0);
		close(src_fd);
		if (mmapRet == (void *)-1)
		{
			munmap(mmapRet,fileInformation.st_size);
			outBuffer_.clear();
			handleError(fd_,404,"Not Found!");
			return ANALYSIS_ERROR;
		}
		char *src_addr = static_cast<char *>(mmapRet);
		outBuffer_ += string(src_addr,src_addr + fileInformation.st_size);
		munmap(mmapRet,fileInformation.st_size);
		return ANALYSIS_SUCCESS;
	}
	return ANALYSIS_ERROR;
}

void HttpData::handleError(int fd,int err_num,string short_msg)
{
	short_msg = " " + short_msg;
	char send_buff[4096];
	string request_buff,body_buff,header_buff;
	body_buff += "<html><title>唉~出错了</title>";
	body_buff += "body bgcolor=\"ffffff\">";
	body_buff += to_string(err_num) + short_msg;
	body_buff += "<hr><em> axunzzz's web server</em>\n</body></html>";

	header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
	header_buff += "Content-Type: text/html\r\n";
	header_buff += "Connection: Close\r\n";
	header_buff += "Content-Length: " +to_string(body_buff.size()) + "\r\n";
	header_buff += "Server: axunzzz's web server\r\n";
	header_buff += "\r\n";

	// 错误处理不考虑writen不完的情况
	request_buff = header_buff + body_buff;
	sprintf(send_buff,"%s",request_buff.c_str());
	writen(fd,send_buff,strlen(send_buff));
}

void HttpData::newEvent()
{
	channel_->setEvents(DEFAULT_EVENT);
	loop_->addToPoller(channel_,DEFAULT_EXPIRED_TIME);
}

void HttpData::handleClose()
{
	connectionState_ = DISCONNECTED;
	shared_ptr<HttpData> guard(shared_from_this());
	loop_->removeFromPoller(channel_);
}







