#ifdef WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif
#include "YXLNetProtocolUtils.h"



namespace YXL
{
	NetProtocolUtils::Package::Package()
	{
		this->mFlag = 0xff11ddaa;
		this->mMaxPackageSize = 100;
		this->mMessageID = 0;
	}

	std::vector<std::vector<unsigned char> > NetProtocolUtils::Package::sliceMessage(void* pData, int dataLength)
	{
		std::vector<std::vector<unsigned char> > packages;
		int packageSize = this->mMaxPackageSize;
		int currentOffset = 0;
		int wholePackageCount = dataLength / packageSize;
		int subPackageSize = dataLength%packageSize;
		for (int i = 0; i < wholePackageCount; i++)
		{
			packages.push_back(createPackage((char*)pData + currentOffset, packageSize));
			currentOffset += packageSize;
		}
		if (subPackageSize > 0)
		{
			packages.push_back(createPackage((char*)pData + currentOffset, subPackageSize));
		}

		for (int i = 0; i < (int)packages.size(); i++)
		{
			std::vector<unsigned char> &data = packages[i];
			PackageHeader *pPH = (PackageHeader*)&data[0];
			pPH->msgID = mMessageID;
			pPH->msgSliceIndex = i;
			pPH->msgSliceCount = (int)packages.size();
			pPH->size = (int)data.size();
			pPH->flag = mFlag;
			
		}
		this->mMessageID++;
		return packages;

	}
	std::vector<unsigned char> NetProtocolUtils::Package::pushPackage(PackageHeader *pPackage)
	{
		std::vector<unsigned char> ret;

		std::vector<unsigned char> packageData(pPackage->size);
		memcpy(&packageData[0], pPackage, pPackage->size);

		std::map<int, std::vector<unsigned char> > &msgPackages = this->mReceivePackages[pPackage->msgID];
		msgPackages[pPackage->msgSliceIndex] = packageData;
		if (pPackage->msgSliceCount > (int)msgPackages.size())
		{
			return ret;
		}
		for (auto it = msgPackages.begin(); it != msgPackages.end(); it++)
		{
			ret.insert(ret.end(), it->second.begin() + sizeof(PackageHeader), it->second.end());
		}

		msgPackages.clear();
		this->mReceivePackages.erase(pPackage->msgID);

		return ret;
	}

	void NetProtocolUtils::Package::setMessageID(int msgID)
	{
		this->mMessageID = msgID;
	}
	void NetProtocolUtils::Package::setMaxPackageSize(int maxPackageSize)
	{
		this->mMaxPackageSize = maxPackageSize;
	}

	std::vector<unsigned char > NetProtocolUtils::Package::createPackage(void* pData, int dataLength)
	{
		std::vector<unsigned char> data(sizeof(PackageHeader) + dataLength);
		PackageHeader *pPH = (PackageHeader*)&data[0];
		memcpy(pPH + 1, pData, dataLength);
		return data;
	}

	NetProtocolUtils::NetProtocolUtils()
	{

		
	}

	NetProtocolUtils::~NetProtocolUtils()
	{

	}

	

	std::string NetProtocolUtils::getIPAddressString(SOCKET sock)
	{
		sockaddr_in6 sa;
#ifdef _WIN32
		int length = sizeof(sa);
#else
        socklen_t length=sizeof(sa);
#endif
		getsockname(sock, (sockaddr*)&sa, &length);

		sockaddr *pSA = (sockaddr*)&sa;
		sockaddr_in *pSAv4 = (sockaddr_in*)&sa;
		sockaddr_in6 *pSAv6 = (sockaddr_in6*)&sa;
		if (pSA->sa_family == AF_INET)
		{
			//char name_out[256];
			const char* name = inet_ntoa(pSAv4->sin_addr);
		//	const char* name = inet_ntop(AF_INET, &pSAv4->sin_addr, name_out, sizeof(name_out));
			return name;
		}
		else if (pSA->sa_family == AF_INET6)
		{
			std::string ret;
		//	const char* name = inet_ntop(AF_INET6, &pSAv6->sin6_addr, name_out, sizeof(name_out));
			return ret;
		}
		else
		{
			return "";
		}
	}


	sockaddr* NetProtocolUtils::getSockAddr(const char*host, int port)
	{
		addrinfo aiHint = { 0 };
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


	void NetProtocolUtils::closeSocket(SOCKET socket)
	{
		if (socket != INVALID_SOCKET)
		{
			
#ifdef WIN32
			shutdown(socket, 2);
			closesocket(socket);
#else
			shutdown(socket, SHUT_RDWR);
			close(socket);
#endif	
		}
	}
	int NetProtocolUtils::getLastError()
	{
		return m_lastError;
	}
	void NetProtocolUtils::setLastError(int errorCode)
	{
		this->m_lastError = errorCode;
	}
#ifdef WIN32
	void NetProtocolUtils::initalizeNetWork()
	{
#ifdef _WINDOWS_
		WORD wVersionRequested;
		WSADATA wsaData;
		int err;

		wVersionRequested = MAKEWORD(1, 1);

		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.   */
			return;
		}

		/* Confirm that the WinSock DLL supports 1.1.*/
		/* Note that if the DLL supports versions greater */
		/* than 1.1 in addition to 1.1, it will still return */
		/* 1.1 in wVersion since that is the version we */
		/* requested.   */

		if (LOBYTE(wsaData.wVersion) != 1 ||
			HIBYTE(wsaData.wVersion) != 1) {
			/* Tell the user that we could not find a usable */
			/* WinSock DLL.   */
			WSACleanup();
			return;
		}
#else
		AfxSocketInit();
#endif
	}
#endif

	

	
}