#pragma once

#include "YXLNetProtocolUtils.h"

namespace YXL
{
	class NetTCPClient :public NetProtocolUtils
	{
	public:
		class Listener
		{
		public:
			Listener();
			virtual ~Listener();

			virtual void asyncReceive(NetTCPClient *pTCPClient, void *pData, int dataLength) = 0;
			virtual void asyncConnectResult(NetTCPClient *pTCPClient, bool result) = 0;
			virtual void asyncClosed(NetTCPClient *pTCPClient) = 0;
			virtual void asyncSendResult(NetTCPClient *pTCPClient, int result) = 0;
			virtual void asyncNew(NetTCPClient *pTCPClient) = 0;
			virtual void asyncDelete(NetTCPClient *pTCPClient) = 0;
		private:

		};

		class SyncListener:public Listener
		{
		public:
			SyncListener();
			virtual ~SyncListener();
			void poll();
			virtual void onReceive(NetTCPClient *pTCPClient, void *pData, int dataLength) = 0;
			virtual void onConnectResult(NetTCPClient *pTCPClient, bool result) = 0;
			virtual void onClosed(NetTCPClient *pTCPClient) = 0;
			virtual void onSendResult(NetTCPClient *pTCPClient, int result) = 0;
			virtual void onNew(NetTCPClient *pTCPClient)=0;
			virtual void onDelete(NetTCPClient *pTCPClient)=0;
		private:
			virtual void asyncReceive(NetTCPClient *pTCPClient, void *pData, int dataLength);
			virtual void asyncConnectResult(NetTCPClient *pTCPClient, bool result);
			virtual void asyncClosed(NetTCPClient *pTCPClient);
			virtual void asyncSendResult(NetTCPClient *pTCPClient, int result);
			virtual void asyncNew(NetTCPClient *pTCPClient);
			virtual void asyncDelete(NetTCPClient *pTCPClient);
			enum MessageType
			{
				MessageType_Receive,
				MessageType_ConnectResult,
				MessageType_Closed,
				MessageType_SendResult,
				MessageType_New,
				MessageType_Delete
			};
			struct Message
			{
				MessageType type;
				NetTCPClient *pTCPClient;
				std::vector<unsigned char> data;
				bool connectResult;
				int sendResult;
			};
			SignalObject mDeleteSignal;
			MessageQueue mMessageQueue;
		};

	public:

		NetTCPClient();
		NetTCPClient(Listener *pListener);
		~NetTCPClient();
		void setAsyncListener(Listener *pListener);

		void asyncConnect(const char* host, int port);
		void asyncAttachSocket(SOCKET sock);
		void asyncClose();
		void asyncSend(void *buffer, int bufferLength);
		static void asyncSend(std::list<NetTCPClient*> targetList,void*butter,int bufferLength);

		std::string getHostName();

		bool connect(const char* host,int port);
		void close();
		int send(void *buffer,int bufferLength);
		int recv(void *buffer,int bufferLength);
	private:
		virtual sockaddr* getSockAddr(const char*host, int port);
		SOCKET m_socket;
		SignalObject m_threadFlag;
		Listener *m_pListener;
	private:
		
		class TCPClientAppClass
		{
		public:
			TCPClientAppClass();
			~TCPClientAppClass();
			void startThread();
		};

		
		enum ThreadMessageType
		{
			TMT_NewObject,
			TMT_DeleteObject,
			TMT_AttachSocket,
			TMT_Connect,
			TMT_Close,
			TMT_Send,
			TMT_SendToGroup
		};


		struct ThreadMessage
		{
			std::list<NetTCPClient*> targetClients;
			NetTCPClient *pTCPClient;
			ThreadMessageType messageType;
			std::vector<char> data;
			std::string host;
			int port;
		};
		static void pushNewObject(NetTCPClient *pTCPClient);
		static void pushDeleteObject(NetTCPClient *pTCPClient);
		static void pushThreadMessage(NetTCPClient *pTCPClient, ThreadMessageType messageType, void*pData = NULL, int dataLength = 0, const char*host = NULL, int port = 0);

		static void threadFunction(void*);

		static MessageQueue s_threadFunctionMessageQueue;
		static MultipleThreadObjectTemplate<bool> s_bIsThreadRunning;
		static TCPClientAppClass s_tcpClientApp;
	};

}