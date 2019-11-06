// @file    HttpData.h
// @author  axunzzz
// @date    2019/10/31
// @Email   469826336@qq.com
#pragma once
#include <unordered_map>
#include <pthread.h>
enum ProcessState
{
	STATE_PARSE_URI = 1,
	STATE_PARSE_HEADERS,
	STATE_RECV_BODY,
	STATE_ANALYSIS,
	STATE_FINISH
};

enum URIState
{
	PARSE_URI_AGAIN = 1,
	PARSE_URI_ERROR,
	PARSE_URI_SUCCESS
};

enum HeaderState
{
	PARSE_HEADER_SUCCESS = 1,
	PARSE_HEADER_AGAIN,
	PARSE_HEADER_ERROR
};

enum AnalysisState
{
	ANALYSIS_SUCCESS = 1,
	ANALYSIS_ERROR
};

//header分析状态
enum ParseState
{
	H_START = 0,
	H_KEY,
	H_COLON,
	H_SPACES_AFTER_COLON,
	H_VALUE,
	H_CR,
	H_LF,
	H_END_CR,
	H_END_LF
};

//不知道ing在哪会用到
enum ConnectionState
{
	CONNECTED = 0,
	DISCONNECTING,
	DISCONNECTED
};

enum HttpMethod
{
	METHOD_GET = 1,
	METHOD_HEAD
};

enum HttpVersion
{
	HTTP_1_0 = 1,
	HTTP_1_1
};


class MimeType
{
private:
	static void init();
	static std::unordered_map<std::string,std::string> mime;
	MimeType();
	MimeType(const MimeType &m);

public:
	static std::string getMime(const std::string &suffix);

private:
	static pthread_once_t once_control;
};
class HttpData;
