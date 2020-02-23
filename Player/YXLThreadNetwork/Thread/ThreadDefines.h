#pragma once

#pragma once
#ifdef WIN32
#ifndef _WINDOWS_
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#endif
#include <time.h>
#elif defined(ANDROID)||defined(__APPLE__)
#include <pthread.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif