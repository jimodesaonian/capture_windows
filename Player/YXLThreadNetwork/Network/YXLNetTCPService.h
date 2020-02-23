#pragma once
#include "YXLNetProtocolUtils.h"
#include "YXLNetTCPClient.h"
#include <functional>

namespace YXL
{
	class NetTCPService:public NetProtocolUtils
	{
	public:
		

	public:
		NetTCPService();
		~NetTCPService();
		bool startService(const char*host,int port);
		void setClientListener(NetTCPClient::Listener *pClientListener);
	private:

		class AcceptThread;
		class ServiceClientListener :public NetTCPClient::Listener
		{
		public:
			ServiceClientListener(NetTCPService *pNetTCPService);
			virtual ~ServiceClientListener();
			virtual void asyncReceive(NetTCPClient *pTCPClient, void *pData, int dataLength);
			virtual void asyncConnectResult(NetTCPClient *pTCPClient, bool result);
			virtual void asyncClosed(NetTCPClient *pTCPClient);
			virtual void asyncSendResult(NetTCPClient *pTCPClient, int result);
			virtual void asyncNew(NetTCPClient *pTCPClient);
			virtual void asyncDelete(NetTCPClient *pTCPClient);
		private:
			NetTCPService *pNetTCPService;
		};

		
		
		SOCKET m_socket;
		SOCKET m_socket6;
		SignalObject m_acceptThreadSignal;
		SignalObject m_acceptThreadSignal6;
		SignalObject m_netTCPClientSignal;
		MultipleThreadObjectTemplate<bool> m_bIsNetTCPClientThreadRunning;

		NetTCPClient::Listener *m_pClientListener;
		ServiceClientListener *m_pServiceClientListener;
		MultipleThreadObjectTemplate<std::map<NetTCPClient*, bool> > m_pNetTCPClients;
		
	//	AcceptThread *m_pAcceptThread;
	//  AcceptThread *m_pAcceptThread6;

		void runAcceptThread(SOCKET *pSocekt, SignalObject *pSignalObject);
		void acceptThreadFunction(void*pParam);
		void netTCPThreadFunction(void*pParam);
		//void 
		
		
	};

}