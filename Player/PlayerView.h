
// PlayerView.h : CPlayerView 类的接口
//

#pragma once

#include "YXLThreadNetwork\YXLThreadNetWorkInclude.h"
#include <vector>
using namespace std;
class NetworkListener :public YXL::NetTCPClient::Listener
{
public:
	NetworkListener(CWnd *pWnd);
	virtual void asyncReceive(YXL::NetTCPClient *pTCPClient, void *pData, int dataLength);
	virtual void asyncConnectResult(YXL::NetTCPClient *pTCPClient, bool result) ;
	virtual void asyncClosed(YXL::NetTCPClient *pTCPClient);
	virtual void asyncSendResult(YXL::NetTCPClient *pTCPClient, int result);
	virtual void asyncNew(YXL::NetTCPClient *pTCPClient) ;
	virtual void asyncDelete(YXL::NetTCPClient *pTCPClient) ;
private:
	CWnd *m_pWnd;
	YXL::NetTCPClient *m_pCurrentClient;
	vector<char> mReceivedData;
};


class CPlayerView : public CView
{
protected: // 仅从序列化创建
	CPlayerView();
	DECLARE_DYNCREATE(CPlayerView)

// 特性
public:
	CPlayerDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
// 实现
public:
	virtual ~CPlayerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	YXL::NetTCPService *m_pTCPService;
	NetworkListener *m_pListener;
protected:
	CDC m_dcFrameBitmap;
	
	HBITMAP m_hFrameBitmap;
// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // PlayerView.cpp 中的调试版本
inline CPlayerDoc* CPlayerView::GetDocument() const
   { return reinterpret_cast<CPlayerDoc*>(m_pDocument); }
#endif

