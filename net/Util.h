// @file    Util.h
// @author  axunzzz
// @date    2019/10/31
// @Email   469826336@qq.com
#pragma once
#include<cstdlib>
#include <string>

ssize_t readn(int fd,std::string &inBuffer,bool &zero);
ssize_t writen(int fd,void *buff,size_t n);
ssize_t writen(int fd,std::string &sbuff);
int socket_bind_listen(int port);
void handle_for_sigpipe();
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
//void setSocketNoLinger(int fd)
//void shutDownWR(int fd)
