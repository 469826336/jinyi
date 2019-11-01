// @file    Util.h
// @author  axunzzz
// @date    2019/10/31
// @Email   469826336@qq.com
#pragma once

int socket_bind_listen(int port);
void handle_for_sigpipe();
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
