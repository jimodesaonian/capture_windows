#pragma once

#pragma once
#ifdef WIN32
#ifndef _WINDOWS_
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#endif
#elif defined(ANDROID)||defined(__APPLE__)

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
typedef int SOCKET;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  (SOCKET)(~0)
#endif
#endif

#ifdef __APPLE__
#include "ThreadInclude.h"
#include "YXLStream.h"
#else
#include "../Thread/ThreadInclude.h"
#endif

#include <deque>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
