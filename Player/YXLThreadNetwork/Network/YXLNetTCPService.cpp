#ifdef WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include "YXLNetTCPService.h"

namespace YXL
{

	NetTCPService::NetTCPService()
	{
		this->m_acceptThreadSignal.sendSignal();
		this->m_acceptThreadSignal6.sendSignal();
		
		this->m_socket = INVALID_SOCKET;
		this->m_socket6 = INVALID_SOCKET;

		this->m_pClientListener = NULL;

		this->m_pServiceClientListener = new ServiceClientListener(this);

		this->m_netTCPClientSignal.emptySignal();
		this->m_bIsNetTCPClientThreadRunning = true;
		ThreadEntity::runThread(std::bind(&NetTCPService::netTCPThreadFunction, this, std::placeholders::_1), NULL);
	}

	NetTCPService::~NetTCPService()
	{
		this->closeSocket(m_socket);
		this->closeSocket(m_socket6);

		this->m_acceptThreadSignal.waitSignal();
		this->m_acceptThreadSignal6.waitSignal();

		this->m_bIsNetTCPClientThreadRunning.lock();
		this->m_bIsNetTCPClientThreadRunning = false;
		this->m_bIsNetTCPClientThreadRunning.unlock();
		this->m_bIsNetTCPClientThreadRunning.sendSignal();
		this->m_netTCPClientSignal.waitSignal();

		delete m_pServiceClientListener;
	}

	bool NetTCPService::startService(const char*host, int port)
	{
		if (this->m_socket != INVALID_SOCKET&&this->m_socket6!=INVALID_SOCKET)
		{
			return false;
		}

		bool bIPv6 = false;
		bool bIPv4 = false;
		if (host)
		{
			sockaddr *pSockAddr = this->getSockAddr(host, port);
			int ret = 0;
			if (pSockAddr->sa_family = AF_INET6)
			{
				this->m_socket6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
				ret = ::bind(this->m_socket6, pSockAddr, sizeof(sockaddr_in6));
				bIPv6 = true;
			}
			else if (pSockAddr->sa_family == AF_INET)
			{
				this->m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				ret = ::bind(this->m_socket, pSockAddr, sizeof(sockaddr_in));
				bIPv4 = true;
			}
			else
			{
				return false;
			}
			if (ret != 0)
			{
				return false;
			}
			
		}
		else
		{
			sockaddr sa = { 0 };
			sockaddr_in *pSA = (sockaddr_in*)&sa;
			pSA->sin_addr.s_addr=htonl(INADDR_ANY);
			pSA->sin_family = AF_INET;
			pSA->sin_port = htons(port);
			this->m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			int ret = ::bind(this->m_socket, &sa, sizeof(sa));
			if (ret == 0)
			{
				bIPv4 = true;
			}
			else
			{
				this->closeSocket(this->m_socket);
			}
			

			sockaddr_in6 sa6 = { 0 };
			sa6.sin6_addr = in6addr_any;
			sa6.sin6_family = AF_INET6;
			sa6.sin6_port = htons(port);
			this->m_socket6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
			ret = ::bind(this->m_socket6, (sockaddr*)&sa6, sizeof(sa6));
			if (ret == 0)
			{
				bIPv6 = true;
			}
			else
			{
				this->closeSocket(this->m_socket6);
			}
		}

		if (!bIPv4&&!bIPv6)
		{
			return false;
		}

		if (bIPv4)
		{
			std::string ipName = this->getIPAddressString(this->m_socket);
			//printf(ipName.c_str());
			::listen(this->m_socket, 5);
			this->runAcceptThread(&this->m_socket, &this->m_acceptThreadSignal);
		}
		if (bIPv6)
		{
			std::string ipName = this->getIPAddressString(this->m_socket6);
			//printf(ipName.c_str());
			::listen(this->m_socket6, 5);
			this->runAcceptThread(&this->m_socket6, &this->m_acceptThreadSignal6);
		}
		return true;
	}

	void NetTCPService::setClientListener(NetTCPClient::Listener *pClientListener)
	{
		this->m_pClientListener = pClientListener;
	}

	void NetTCPService::runAcceptThread(SOCKET *pSocekt, SignalObject *pSignalObject)
	{
		void**pParams = new void*[2];
		pParams[0] = pSocekt;
		pParams[1] = pSignalObject;
		pSignalObject->emptySignal();

		ThreadEntity::runThread(::std::bind(&NetTCPService::acceptThreadFunction, this, std::placeholders::_1), pParams);
	}

	void NetTCPService::acceptThreadFunction(void*pParam)
	{
		void**pParams = (void**)pParam;
		SOCKET *pSocket = (SOCKET*)pParams[0];
		SignalObject *pSignalObject = (SignalObject*)pParams[1];
		delete pParams;
		pParam = NULL;
		pParams = NULL;

		SOCKET clientSocket = INVALID_SOCKET;
		do
		{
			sockaddr_in6 sa;
#ifdef _WIN32
			int saLength = sizeof(sa);
#else
            socklen_t saLength=sizeof(sa);
#endif
			clientSocket = accept(*pSocket, (sockaddr*)&sa, &saLength);
			if (clientSocket != INVALID_SOCKET)
			{
				NetTCPClient *pNetTCPClient = new NetTCPClient(m_pServiceClientListener);

				m_pNetTCPClients.lock();
				m_pNetTCPClients.value[pNetTCPClient] = true;
				m_pNetTCPClients.unlock();

				pNetTCPClient->asyncAttachSocket(clientSocket);
			}
		} while (clientSocket != INVALID_SOCKET);
		pSignalObject->sendSignal();
	}

	void NetTCPService::netTCPThreadFunction(void*pParam)
	{
		while (true)
		{
			m_bIsNetTCPClientThreadRunning.waitSignal();
			m_bIsNetTCPClientThreadRunning.emptySignal();
			m_bIsNetTCPClientThreadRunning.lock();
			if (!this->m_bIsNetTCPClientThreadRunning.value)
			{
				break;
			}
			m_bIsNetTCPClientThreadRunning.unlock();


			std::vector<NetTCPClient*> pClients;
			m_pNetTCPClients.lock();
			for (auto it = m_pNetTCPClients.value.begin(); it != m_pNetTCPClients.value.end(); it++)
			{
				if (!it->second)
				{
					pClients.push_back(it->first);
				}
			}
			for (int i = 0; i < (int)pClients.size(); i++)
			{
				m_pNetTCPClients.value.erase(pClients[i]);
			}
			m_pNetTCPClients.unlock();

			for (int i = 0; i < (int)pClients.size(); i++)
			{
				delete pClients[i];
			}
		}

		m_bIsNetTCPClientThreadRunning.unlock();

		m_pNetTCPClients.lock();
		for (auto it = m_pNetTCPClients.value.begin(); it != m_pNetTCPClients.value.end(); it++)
		{
			delete it->first;
		}
		m_pNetTCPClients.value.clear();
		m_pNetTCPClients.unlock();

		m_netTCPClientSignal.sendSignal();
	}

	NetTCPService::ServiceClientListener::ServiceClientListener(NetTCPService *pNetTCPService)
	{
		this->pNetTCPService = pNetTCPService;
	}
	NetTCPService::ServiceClientListener::~ServiceClientListener()
	{
	}
	void NetTCPService::ServiceClientListener::asyncReceive(NetTCPClient *pTCPClient, void *pData, int dataLength)
	{
		if (this->pNetTCPService->m_pClientListener)
		{
			this->pNetTCPService->m_pClientListener->asyncReceive(pTCPClient, pData, dataLength);
		}
	}
	void NetTCPService::ServiceClientListener::asyncConnectResult(NetTCPClient *pTCPClient, bool result)
	{
		if (this->pNetTCPService->m_pClientListener)
		{
			this->pNetTCPService->m_pClientListener->asyncConnectResult(pTCPClient, result);
		}
	}
	void NetTCPService::ServiceClientListener::asyncClosed(NetTCPClient *pTCPClient)
	{
		if (this->pNetTCPService->m_pClientListener)
		{
			this->pNetTCPService->m_pClientListener->asyncClosed(pTCPClient);
		}

		this->pNetTCPService->m_pNetTCPClients.lock();
		this->pNetTCPService->m_pNetTCPClients.value[pTCPClient] = false;
		this->pNetTCPService->m_pNetTCPClients.unlock();
		this->pNetTCPService->m_bIsNetTCPClientThreadRunning.sendSignal();
	}
	void NetTCPService::ServiceClientListener::asyncSendResult(NetTCPClient *pTCPClient, int result)
	{
		if (this->pNetTCPService->m_pClientListener)
		{
			this->pNetTCPService->m_pClientListener->asyncSendResult(pTCPClient, result);
		}
	}

	void NetTCPService::ServiceClientListener::asyncNew(NetTCPClient *pTCPClient)
	{
		if (this->pNetTCPService->m_pClientListener)
		{
			this->pNetTCPService->m_pClientListener->asyncNew(pTCPClient);
		}
	}
	void NetTCPService::ServiceClientListener::asyncDelete(NetTCPClient *pTCPClient)
	{
		if (this->pNetTCPService->m_pClientListener)
		{
			this->pNetTCPService->m_pClientListener->asyncDelete(pTCPClient);
		}
	}

}