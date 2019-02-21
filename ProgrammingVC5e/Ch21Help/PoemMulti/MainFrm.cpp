// MainFrm.cpp : implementation of the CMainFrame class
//

#include "StdAfx.h"
#include "PoemMulti.h"

#include "PoemDoc.h"
#include "StringView.h"
#include "HexView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    ON_WM_CREATE()
    // Global help commands
    ON_COMMAND(ID_HELP_FINDER, &CMDIFrameWnd::OnHelpFinder)
    ON_COMMAND(ID_HELP, &CMDIFrameWnd::OnHelp)
    ON_COMMAND(ID_CONTEXT_HELP, &CMDIFrameWnd::OnContextHelp)
    ON_COMMAND(ID_DEFAULT_HELP, &CMDIFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_WINDOW_NEWSTRINGWINDOW, &CMainFrame::OnWindowNewStringWindow)
	ON_COMMAND(ID_WINDOW_NEWHEXWINDOW, &CMainFrame::OnWindowNewHexWindow)
END_MESSAGE_MAP()

static UINT indicators[] = {
    ID_SEPARATOR,           // status line indicator
    ID_INDICATOR_CAPS,
    ID_INDICATOR_NUM,
    ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    // TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
                               | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
            !m_wndToolBar.LoadToolBar(IDR_MAINFRAME)) {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if (!m_wndStatusBar.Create(this) ||
            !m_wndStatusBar.SetIndicators(indicators,
                                          sizeof(indicators)/sizeof(UINT))) {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    // TODO: Delete these three lines if you don't want the toolbar to be dockable
    m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
    EnableDocking(CBRS_ALIGN_ANY);
    DockControlBar(&m_wndToolBar);

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CMDIFrameWnd::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnWindowNewStringWindow()
{
    OnWindowNew();
}

void CMainFrame::OnWindowNewHexWindow()
{
    CMDIChildWnd* child = MDIGetActive();
    CDocument* doc;
    if (child == NULL || (doc = child->GetActiveDocument()) == NULL) {
        MYTRACE("No active document for WindowNew command");
        AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
        return;
    }

    CDocTemplate* docTemp = ((CPoemMultiApp*) AfxGetApp())->m_TemplateHex;
    ASSERT_VALID(docTemp);
    CFrameWnd* frame = docTemp->CreateNewFrame(doc, child);
    if (frame == NULL) {
        MYTRACE("Failed to create new frame");
        AfxMessageBox(AFX_IDP_COMMAND_FAILURE);
        return;
    }

    docTemp->InitialUpdateFrame(frame, doc);
}