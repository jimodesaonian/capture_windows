
// PlayerView.cpp : CPlayerView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "Player.h"
#endif

#include "PlayerDoc.h"
#include "PlayerView.h"

#include <gdiplus.h>
using namespace Gdiplus;

#define UM_VIDEO_MESSAGE	(WM_USER+3)
#define UM_AUDIO_MESSAGE	(WM_USER+4)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


NetworkListener::NetworkListener(CWnd *pWnd)
{
	this->m_pWnd = pWnd;
	this->m_pCurrentClient = NULL;
}
void NetworkListener::asyncReceive(YXL::NetTCPClient *pTCPClient, void *pData, int dataLength)
{
	if (this->m_pCurrentClient != pTCPClient)
	{
		return;
	}
	vector<byte> data(dataLength);
	memcpy(&data[0], pData, dataLength);
	this->mReceivedData.insert(this->mReceivedData.end(), data.begin(), data.end());
	if (this->mReceivedData.size() >= 4)
	{
		int dataSize = this->mReceivedData.size();
		int *pSize = (int*)&this->mReceivedData[0];
		if (*pSize <= dataSize - 4)
		{
			vector<char> imgData;
			imgData.insert(imgData.end(), this->mReceivedData.begin() + 4, this->mReceivedData.begin() + *pSize + 4);
			this->mReceivedData.erase(this->mReceivedData.begin(), this->mReceivedData.begin() + *pSize + 4);
			m_pWnd->SendMessage(UM_VIDEO_MESSAGE, (WPARAM)imgData.size(), (LPARAM)&imgData[0]);
		}
	}
}
void NetworkListener::asyncConnectResult(YXL::NetTCPClient *pTCPClient, bool result)
{
	if (this->m_pCurrentClient == NULL&&result)
	{
		this->m_pCurrentClient = pTCPClient;
	}
}
void NetworkListener::asyncClosed(YXL::NetTCPClient *pTCPClient)
{
	if (this->m_pCurrentClient == pTCPClient)
	{
		this->m_pCurrentClient = NULL;
		this->mReceivedData.clear();
	}
}
void NetworkListener::asyncSendResult(YXL::NetTCPClient *pTCPClient, int result)
{
}
void NetworkListener::asyncNew(YXL::NetTCPClient *pTCPClient)
{
}
void NetworkListener::asyncDelete(YXL::NetTCPClient *pTCPClient)
{
}


// CPlayerView

IMPLEMENT_DYNCREATE(CPlayerView, CView)

BEGIN_MESSAGE_MAP(CPlayerView, CView)
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CPlayerView ����/����

CPlayerView::CPlayerView()
{
	// TODO:  �ڴ˴���ӹ������
	m_hFrameBitmap = NULL;
	YXL::NetProtocolUtils::initalizeNetWork();
	m_pTCPService = new YXL::NetTCPService;
	m_pListener = new NetworkListener(this);
	m_pTCPService->setClientListener(m_pListener);
	m_pTCPService->startService(NULL, 9999);
}

CPlayerView::~CPlayerView()
{
	m_pTCPService->setClientListener(NULL);
	delete m_pListener;
	delete m_pTCPService;
}

BOOL CPlayerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CPlayerView ����

void CPlayerView::OnDraw(CDC* pDC)
{
	//this->OnWndMsg
	CPlayerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;
	//gp.DrawImage()
	
	// TODO:  �ڴ˴�Ϊ����������ӻ��ƴ���
}


BOOL CPlayerView::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_CREATE)
	{
		CDC *pDC = GetDC();
		m_dcFrameBitmap.CreateCompatibleDC(pDC);
	}
	else if (message == UM_VIDEO_MESSAGE)
	{
		CComPtr<IStream> stream;
		stream.Attach(SHCreateMemStream((BYTE*)lParam, wParam));
		Gdiplus::Bitmap frameBitmap(stream);
	

		int bmpWidth = frameBitmap.GetWidth();
		int bmpHeight = frameBitmap.GetHeight();
		if (bmpWidth == 0 || bmpHeight == 0)
		{
			return 0;
		}
		Gdiplus::Color color(0xffffffff);
		//	HBITMAP temphBitmap;
		frameBitmap.GetHBITMAP(color, &m_hFrameBitmap);
		HGDIOBJ oldhBitmap = m_dcFrameBitmap.SelectObject(m_hFrameBitmap);
		DeleteObject(oldhBitmap);

		CRect windowSize;
		GetClientRect(windowSize);
		float winWidth= windowSize.Size().cx;
		float winHeight = windowSize.Size().cy;
		float ratioW = winWidth / winHeight;
		float ratioBmp = (float)bmpWidth / (float)bmpHeight;
		int dstWidth = 0;
		int dstHeight = 0;
		float ratio = 1;
		if (ratioW > ratioBmp)
		{
			ratio=(winHeight / bmpHeight);
		}
		else
		{
			ratio = (winWidth / bmpWidth);
		}
		dstWidth = bmpWidth*ratio;
		dstHeight = bmpHeight*ratio;

		CClientDC dc(this);
		
		dc.StretchBlt(0, 0, dstWidth, dstHeight, &m_dcFrameBitmap, 0,0,bmpWidth, bmpHeight, SRCCOPY);
	}
	return CView::OnWndMsg(message, wParam, lParam, pResult);
}


// CPlayerView ���

#ifdef _DEBUG
void CPlayerView::AssertValid() const
{
	CView::AssertValid();
}

void CPlayerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CPlayerDoc* CPlayerView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPlayerDoc)));
	return (CPlayerDoc*)m_pDocument;

}
#endif //_DEBUG


// CPlayerView ��Ϣ�������
