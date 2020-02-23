#include "YXLNetTCPClient.h"
#include <map>

using namespace std;

namespace YXL
{
	MessageQueue NetTCPClient::s_threadFunctionMessageQueue;
	MultipleThreadObjectTemplate<bool> NetTCPClient::s_bIsThreadRunning;
	NetTCPClient::TCPClientAppClass NetTCPClient::s_tcpClientApp;

	
}


namespace YXL
{

	NetTCPClient::Listener::Listener()
	{
	}

	NetTCPClient::Listener::~Listener()
	{

	}

	NetTCPClient::SyncListener::SyncListener()
	{}
	NetTCPClient::SyncListener::~SyncListener()
	{
		while (mMessageQueue.hasMessage())
		{
			Message *pMsg = (Message*)mMessageQueue.popWaitMessage();
			delete pMsg;
		}
	}
	void NetTCPClient::SyncListener::poll()
	{
		while (mMessageQueue.hasMessage())
		{
			Message *pMsg = (Message*)mMessageQueue.popWaitMessage();
			MessageType type = pMsg->type;
			if (type == MessageType_Closed)
			{
				this->onClosed(pMsg->pTCPClient);
			}
			else if (type == MessageType_ConnectResult)
			{
				this->onConnectResult(pMsg->pTCPClient, pMsg->connectResult);
			}
			else if (type == MessageType_Receive)
			{
				if (pMsg->data.size() == 0)
				{
					this->onReceive(pMsg->pTCPClient, NULL, 0);
				}
				else
				{
					this->onReceive(pMsg->pTCPClient, &pMsg->data[0], (int)pMsg->data.size());
				}
			}
			else if (type == MessageType_SendResult)
			{
				this->onSendResult(pMsg->pTCPClient, pMsg->sendResult);
			}
			else if (type == MessageType_New)
			{
				this->onNew(pMsg->pTCPClient);
			}
			else if (type == MessageType_Delete)
			{
				this->onDelete(pMsg->pTCPClient);
				this->mDeleteSignal.sendSignal();
			}
			delete pMsg;
		}
	}


	void NetTCPClient::SyncListener::asyncReceive(NetTCPClient *pTCPClient, void *pData, int dataLength)
	{
		Message *pMsg = new Message;
		pMsg->type = MessageType_Receive;
		pMsg->data.resize(dataLength);
		if (dataLength > 0)
		{
			memcpy(&pMsg->data[0], pData, dataLength);
		}
		pMsg->pTCPClient = pTCPClient;

		mMessageQueue.pushMessage(pMsg);
	}
	void NetTCPClient::SyncListener::asyncConnectResult(NetTCPClient *pTCPClient, bool result)
	{

		Message *pMsg = new Message;
		pMsg->type = MessageType_ConnectResult;
		pMsg->connectResult = result;
		pMsg->pTCPClient = pTCPClient;
		mMessageQueue.pushMessage(pMsg);
	}
	void NetTCPClient::SyncListener::asyncClosed(NetTCPClient *pTCPClient)
	{
		Message *pMsg = new Message;
		pMsg->type = MessageType_Closed;
		pMsg->pTCPClient = pTCPClient;
		mMessageQueue.pushMessage(pMsg);
	}
	void NetTCPClient::SyncListener::asyncSendResult(NetTCPClient *pTCPClient, int result)
	{
		Message *pMsg = new Message;
		pMsg->type = MessageType_SendResult;
		pMsg->sendResult = result;
		pMsg->pTCPClient = pTCPClient;
		mMessageQueue.pushMessage(pMsg);
	}

	void NetTCPClient::SyncListener::asyncNew(NetTCPClient *pTCPClient)
	{
		Message *pMsg = new Message;
		pMsg->type = MessageType_New;
		pMsg->pTCPClient = pTCPClient;
		mMessageQueue.pushMessage(pMsg);
	}
	void NetTCPClient::SyncListener::asyncDelete(NetTCPClient *pTCPClient)
	{
		Message *pMsg = new Message;
		pMsg->type = MessageType_Delete;
		pMsg->pTCPClient = pTCPClient;
		mDeleteSignal.emptySignal();
		mMessageQueue.pushMessage(pMsg);
		mDeleteSignal.waitSignal();
	}

	NetTCPClient::NetTCPClient()
	{
		m_pListener = NULL;
		m_socket = INVALID_SOCKET;
		m_threadFlag.emptySignal();
		pushNewObject(this);

		s_tcpClientApp.startThread();
	}

	NetTCPClient::NetTCPClient(Listener *pListener)
	{
		m_pListener = pListener;
		m_socket = INVALID_SOCKET;
		m_threadFlag.emptySignal();
		pushNewObject(this);

		s_tcpClientApp.startThread();
	}

	NetTCPClient::~NetTCPClient()
	{
		this->closeSocket(m_socket);
		m_socket = INVALID_SOCKET;
		pushDeleteObject(this);
		m_threadFlag.waitSignal();

	}

	void NetTCPClient::setAsyncListener(Listener *pListener)
	{
		this->m_pListener = pListener;
	}

	void NetTCPClient::asyncConnect(const char* host, int port)
	{
		pushThreadMessage(this, TMT_Connect, NULL, 0, host, port);
	}

	void NetTCPClient::asyncAttachSocket(SOCKET sock)
	{
		pushThreadMessage(this, TMT_AttachSocket, (void*)&sock,sizeof(sock));
	}

	void NetTCPClient::asyncClose()
	{
		pushThreadMessage(this, TMT_Close);
	}
	void NetTCPClient::asyncSend(void *buffer, int bufferLength)
	{
		pushThreadMessage(this, TMT_Send, buffer, bufferLength);
	}

	void NetTCPClient::asyncSend(std::list<NetTCPClient*> targetList, void*butter, int bufferLength)
	{

		ThreadMessage *pTM = new ThreadMessage;
		
		pTM->targetClients = targetList;
		pTM->pTCPClient = NULL;
		pTM->messageType = TMT_SendToGroup;
		pTM->host = "";
		pTM->port = 0;

		if (butter && bufferLength > 0)
		{
			pTM->data.resize(bufferLength);
			memcpy(&pTM->data[0], butter, bufferLength);
		}
		s_threadFunctionMessageQueue.pushMessage(pTM);
	}


	std::string NetTCPClient::getHostName()
	{
		return getIPAddressString(this->m_socket);
	}

	bool NetTCPClient::connect(const char* host, int port)
	{
		sockaddr *pSA;
		if (!(pSA=this->getSockAddr(host, port)))
		{
			this->setLastError(1);
			return false;
		}
		this->closeSocket(m_socket);
		m_socket = socket(pSA->sa_family, SOCK_STREAM, IPPROTO_TCP);
		if (m_socket == INVALID_SOCKET)
		{
			this->setLastError(2);
			return false;
		}
		
		int ret = ::connect(m_socket, pSA, pSA->sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6));
		if (ret == 0)
		{
			return true;
		}
		else
		{
			this->setLastError(3);
			return false;
		}
	}

	void NetTCPClient::close()
	{
		this->closeSocket(m_socket);
		m_socket = INVALID_SOCKET;
		
	}

	int NetTCPClient::send(void *buffer, int bufferLength)
	{
		return ::send(m_socket, (char*)buffer, bufferLength, 0);
	}

	int NetTCPClient::recv(void *buffer, int bufferLength)
	{
		return ::recv(m_socket, (char*)buffer, bufferLength, 0);
	}


	sockaddr* NetTCPClient::getSockAddr(const char*host, int port)
	{
		addrinfo aiHint = { 0 };
		aiHint.ai_family = AF_UNSPEC;
		aiHint.ai_flags = AI_ADDRCONFIG;
		aiHint.ai_socktype = SOCK_STREAM;
		addrinfo *pAI;
		int result = getaddrinfo(host, NULL, &aiHint, &pAI);

		if (result != 0)
		{
			return NULL;
		}

		if (pAI->ai_family == AF_INET)
		{
			((sockaddr_in*)pAI->ai_addr)->sin_port = (u_short)htons(port);
		}
		else if (pAI->ai_family == AF_INET6)
		{
			((sockaddr_in6*)pAI->ai_addr)->sin6_port = (u_short)htons(port);
		}
		else
		{
			return NULL;
		}
		return pAI->ai_addr;
	}

	NetTCPClient::TCPClientAppClass::TCPClientAppClass()
	{
		
	}

	NetTCPClient::TCPClientAppClass::~TCPClientAppClass()
	{
		s_bIsThreadRunning.lock();
		if (!s_bIsThreadRunning.value)
		{
			s_bIsThreadRunning.unlock();
			return;
		}
		s_bIsThreadRunning = false;
		s_bIsThreadRunning.unlock();
		s_bIsThreadRunning.waitSignal();
	}

	void NetTCPClient::TCPClientAppClass::startThread()
	{

		s_bIsThreadRunning.lock();
		if (s_bIsThreadRunning.value)
		{
			s_bIsThreadRunning.unlock();
			return;
		}
		s_bIsThreadRunning = true;
		s_bIsThreadRunning.emptySignal();
		s_bIsThreadRunning.unlock();
		ThreadEntity::runThread(NetTCPClient::threadFunction, NULL);
	}


	void NetTCPClient::pushNewObject(NetTCPClient *pTCPClient)
	{
		pushThreadMessage(pTCPClient, TMT_NewObject);
	}
	void NetTCPClient::pushDeleteObject(NetTCPClient *pTCPClient)
	{
		pushThreadMessage(pTCPClient, TMT_DeleteObject);
	}

	void NetTCPClient::pushThreadMessage(NetTCPClient *pTCPClient, ThreadMessageType messageType, void*pData, int dataLength, const char*host, int port)
	{
		ThreadMessage *pTM = new ThreadMessage;
		pTM->pTCPClient = pTCPClient;
		pTM->messageType = messageType;
		pTM->host = host ? host : "";
		pTM->port = port;
		
		if (pData && dataLength > 0)
		{
			pTM->data.resize(dataLength);
			memcpy(&pTM->data[0], pData, dataLength);
		}
		s_threadFunctionMessageQueue.pushMessage(pTM);
	}

#if 1
	struct __TCPClientStatus
	{
		bool isConnected;
		vector<char> sendBufferRest;
	};
	void NetTCPClient::threadFunction(void*)
	{
		map<NetTCPClient*, __TCPClientStatus*> clients;
		vector<char> recvBuffer(1024*1024);
		while (true)
		{
			ThreadEntity::sleep(1);
			s_bIsThreadRunning.lock();
			if (!s_bIsThreadRunning.value)
			{
				s_bIsThreadRunning.unlock();
				break;
			}
			s_bIsThreadRunning.unlock();
			if (s_threadFunctionMessageQueue.hasMessage())
			{
				ThreadMessage* pTM = (ThreadMessage*)s_threadFunctionMessageQueue.popWaitMessage();
				NetTCPClient *pTCPClient = pTM->pTCPClient;
				Listener *pListener = NULL;
				if (pTCPClient)
				{
					pListener=pTCPClient->m_pListener;
				}
				
				if (pTM->messageType == TMT_NewObject)
				{
					clients[pTCPClient] = new __TCPClientStatus;
					clients[pTCPClient]->isConnected = false;
					if (pListener)
					{
						pListener->asyncNew(pTCPClient);
					}
				}
				else if (pTM->messageType == TMT_DeleteObject)
				{
					if (pListener)
					{
						pListener->asyncDelete(pTCPClient);
					}
					delete clients[pTCPClient];
					clients.erase(pTCPClient);
					pTCPClient->m_threadFlag.sendSignal();
				}
				else if (clients.find(pTCPClient) == clients.end() && pTM->messageType != TMT_SendToGroup)
				{

				}
				else if (pTM->messageType == TMT_Connect)
				{
					clients[pTCPClient]->isConnected=pTCPClient->connect(pTM->host.c_str(), pTM->port);
					clients[pTCPClient]->sendBufferRest.clear();
					if (clients[pTCPClient]->isConnected)
					{
#ifdef _WIN32
						unsigned long bFIONBIO=1;
						ioctlsocket(pTCPClient->m_socket,FIONBIO,&bFIONBIO);
#else
                        int flags =fcntl(pTCPClient->m_socket, F_GETFL, 0);
                        fcntl(pTCPClient->m_socket,F_SETFL,flags |O_NONBLOCK);
#endif
					}
					if (pListener)
					{
						pListener->asyncConnectResult(pTCPClient, clients[pTCPClient]->isConnected);
					}
				}
				else if (pTM->messageType==TMT_AttachSocket)
				{
					
					pTCPClient->m_socket = *(SOCKET*)&pTM->data[0];
					clients[pTCPClient]->isConnected = true;
					clients[pTCPClient]->sendBufferRest.clear();
				
#ifdef _WIN32
					unsigned long bFIONBIO = 1;
					ioctlsocket(pTCPClient->m_socket, FIONBIO, &bFIONBIO);
#else
					int flags = fcntl(pTCPClient->m_socket, F_GETFL, 0);
					fcntl(pTCPClient->m_socket, F_SETFL, flags | O_NONBLOCK);
#endif
					if (pListener)
					{
						pListener->asyncConnectResult(pTCPClient, clients[pTCPClient]->isConnected);
					}
				}
				else if (pTM->messageType == TMT_Close)
				{
					clients[pTCPClient]->isConnected = false;
					clients[pTCPClient]->sendBufferRest.clear();
					pTCPClient->close();
					if (pListener)
					{
						pListener->asyncClosed(pTCPClient);
					}
				}
				else if (pTM->messageType == TMT_Send)
				{
					if (clients.find(pTCPClient) != clients.end())
					{
						__TCPClientStatus *pStatus = clients[pTCPClient];
						pStatus->sendBufferRest.insert(pStatus->sendBufferRest.end(), pTM->data.begin(), pTM->data.end());
					}
					/*int length=pTCPClient->send(&pTM->data[0],(int)pTM->data.size());
					if (pListener)
					{
						pListener->asyncSendResult(pTCPClient,length);
					}*/
				}
				else if (pTM->messageType == TMT_SendToGroup)
				{
					for (auto it = pTM->targetClients.begin(); it != pTM->targetClients.end(); it++)
					{
						NetTCPClient *pClient = *it;
						if (clients.find(pClient) != clients.end())
						{
							__TCPClientStatus *pStatus = clients[pClient];
							pStatus->sendBufferRest.insert(pStatus->sendBufferRest.end(), pTM->data.begin(), pTM->data.end());
						}
					}
				}

				delete pTM;
			}
			else
			{
				for (auto it = clients.begin(); it != clients.end(); it++)
				{
					if (it->second->isConnected&&it->second->sendBufferRest.size())
					{
						vector<char>&buf = it->second->sendBufferRest;
						int sendLength=it->first->send(&buf[0],(int)buf.size());
						if (sendLength > 0)
						{
							buf.erase(buf.begin(), buf.begin() + sendLength);
							if (it->first->m_pListener)
							{
								it->first->m_pListener->asyncSendResult(it->first, sendLength);
							}
						}
						else if (sendLength == 0)
						{
							pushThreadMessage(it->first, TMT_Close);
						}
						else
						{
							//printf("time out");
						}

					}

					if (it->second->isConnected)
					{
						int ret = it->first->recv(&recvBuffer[0], (int)recvBuffer.size());
						if (ret > 0)
						{
							if (it->first->m_pListener)
							{
								it->first->m_pListener->asyncReceive(it->first, &recvBuffer[0], ret);
							}
						}
#ifdef _WIN32
						else
						{
							DWORD lastError= GetLastError();
							if(lastError==10035)
							{
								
							}
							else 
							{//if (lastError == 10038/*非套接字操作*/ || lastError == 10054/*远程连接关闭*/)
							
								pushThreadMessage(it->first, TMT_Close);
							}
						}
#else
						else if (ret==0)
						{
							pushThreadMessage(it->first, TMT_Close);
						}
						else
						{
						//	printf("timeout");
						}
#endif
					}
				}
			}
		}
		for (auto it = clients.begin(); it != clients.end(); it++)
		{
			delete it->second;
			it->first->m_threadFlag.sendSignal();
		}
		clients.clear();
		while(s_threadFunctionMessageQueue.hasMessage())
		{
			ThreadMessage* pTM = (ThreadMessage*)s_threadFunctionMessageQueue.popWaitMessage();
			delete pTM;
		}
		s_bIsThreadRunning.sendSignal();
	}

#elif defined _WIN32
	void NetTCPClient::threadFunction(void*)
	{
		HANDLE hCompletionIO;
		ULONG_PTR ulCompletioinKey = 0;
		hCompletionIO = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, ulCompletioinKey, 0);
		while (true)
		{
			s_bIsThreadRunning.lock();
			if (!s_bIsThreadRunning.value)
			{
				s_bIsThreadRunning.unlock();
				break;
			}
			s_bIsThreadRunning.unlock();
			if (s_threadFunctionMessageQueue.hasMessage())
			{
				ThreadMessage* pTM = (ThreadMessage*)s_threadFunctionMessageQueue.popWaitMessage();
				NetTCPClient *pTCPClient = pTM->pTCPClient;
				if (pTM->messageType == TMT_Connect)
				{
					pTCPClient->m_socket = socket(pTM->sa.sa_family, SOCK_STREAM, IPPROTO_TCP);
					HANDLE hTemp=CreateIoCompletionPort((HANDLE)pTCPClient->m_socket, hCompletionIO, ulCompletioinKey, 0);
					if (hTemp != hCompletionIO)
					{
						continue;
					}
					WSAConnect(pTCPClient->m_socket, &pTM->sa, sizeof(pTM->sa), NULL, NULL, NULL, NULL);
				}
				else if (pTM->messageType == TMT_Send)
				{
				}

				delete pTM;
			}
			else
			{
				

			}
		}
		while(s_threadFunctionMessageQueue.hasMessage())
		{
			ThreadMessage* pTM = (ThreadMessage*)s_threadFunctionMessageQueue.popWaitMessage();
			delete pTM;
		}
		s_bIsThreadRunning.sendSignal();
	}
#else
	void NetTCPClient::threadFunction(void*)
	{
		map<NetTCPClient*, bool> clients;
		while (true)
		{
			s_bIsThreadRunning.lock();
			if (!s_bIsThreadRunning.value)
			{
				s_bIsThreadRunning.unlock();
				break;
			}
			s_bIsThreadRunning.unlock();
			if (s_threadFunctionMessageQueue.hasMessage())
			{
				ThreadMessage* pTM = (ThreadMessage*)s_threadFunctionMessageQueue.popWaitMessage();
				if (pTM->messageType == TMT_Connect)
				{

				}
			}
			else
			{


			}
		}
		s_bIsThreadRunning.sendSignal();
}
#endif

}