#pragma once
#include "NetworkDefines.h"

namespace YXL
{
	class NetProtocolUtils
	{

	public:
		NetProtocolUtils();
		virtual ~NetProtocolUtils();
	public:
#pragma pack(push)
#pragma pack(1)
		typedef int MSGID;
		struct PackageHeader
		{
			int flag;
			int size;
			int check;
			MSGID msgID;
			int msgSliceIndex;
			int msgSliceCount;
		};
		class Package
		{
		public:
			Package();
			std::vector<std::vector<unsigned char> > sliceMessage(void* pData,int dataLength);
			std::vector<unsigned char> pushPackage(PackageHeader *pPackage);
			void setMessageID(int msgID);
			void setMaxPackageSize(int maxPackageSize);
		private:
			std::vector<unsigned char > createPackage(void* pData, int dataLength);
			std::map<MSGID, time_t> mReceiveTime;
			std::vector<unsigned char> mLastMessageData;
			
			int mFlag;
			int mMaxPackageSize;
			MSGID mMessageID;
			std::map<MSGID, std::map<int, std::vector<unsigned char> > > mReceivePackages;
		};
#pragma pack(pop)
	protected:
		std::string getIPAddressString(SOCKET sock);
		virtual sockaddr* getSockAddr(const char*host, int port);

		void closeSocket(SOCKET socket);

		int getLastError();
	
		void setLastError(int errorCode);
	private:
		int m_lastError;
	public:
#ifdef WIN32
		static void initalizeNetWork();
#endif
	};
}