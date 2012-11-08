// MtrlAlpha.cpp
//

#include "StdAfx.h"
#include "MtrlAlpha.h"
#include "RenderMtrlAlpha.h"
#include "MainFrame.h"

////////////////////////////////////////////////////////////////////////////////
// 名字空间
////////////////////////////////////////////////////////////////////////////////

using std::auto_ptr;

////////////////////////////////////////////////////////////////////////////////
// 全局量
////////////////////////////////////////////////////////////////////////////////

CMtrlAlphaApp g_MtrlAlphaApp;   // 应用程序实例

////////////////////////////////////////////////////////////////////////////////
// 应用程序类 CMtrlAlphaApp
////////////////////////////////////////////////////////////////////////////////

BOOL CMtrlAlphaApp::InitInstance()
{
    CWinApp::InitInstance();

    // 如果装配文件 (manifest) 中指定使用 Windows 通用控件 ComCtl32.dll 6.0+ 版本, 则在 Windows XP 下需要调用 InitCommonControlsEx(), 否则窗口创建将失败
    // 设置程序中能够使用的通用控件类, ICC_WIN95_CLASSES 表示所有 Win95 通用控件
    INITCOMMONCONTROLSEX initCtrls;
    initCtrls.dwSize = sizeof(initCtrls);
    initCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&initCtrls);

    InitLocale();

    // 计算主模块全路径, 设置工作目录
    InitModulePath(MAX_PATH);
    InitWorkDir(GetModuleDir());

    // NOTE:
    // 保存应用设置选项的注册表键, 通常命名为开发公司, 组织, 团队的名字
    // 该函数设定 m_pszRegistryKey, 并影响 GetProfileInt 等函数
    // 保存应用设置的注册表键:
    // HKCU\Software\<company name>\<application name>\<section name>\<value name>
    // 当不使用注册表而使用 ini 文件时, 请去掉 SetRegistryKey
    SetRegistryKey(_T(MODULE_NAME));

    try {
        CMainFrame* pFrmWnd = new CMainFrame();

        // 由共享资源 IDR_MAINFRAME 创建主框架窗口, IDR_MAINFRAME 可同时标识: 菜单(必需), 快捷键, 主图标, 标题栏字符串
        pFrmWnd->LoadFrame(IDR_MAINFRAME);

        // NOTE: 在 Run 中显示窗口
        pFrmWnd->ShowWindow(SW_HIDE);

        m_pMainWnd = pFrmWnd;
    }
    catch (std::exception& e) {
        MYTRACEA("std::exception: what: %s, type: %s", e.what(), typeid(e).name());
        return FALSE;
    }
    catch (...) {
        MYTRACE("unknown exception");
        return FALSE;
    }

    return TRUE;
}

// NOTE: 进入 ExitInstance 前, m_pMainWnd 窗口对象已被释放掉了
int CMtrlAlphaApp::Exit(int exitCode)
{
    _tchdir(m_OldWorkDir);  // 恢复旧的工作目录
    CWinApp::ExitInstance();
    return exitCode;
}

void CMtrlAlphaApp::InitLocale()
{
#define _USE_CRT_LOCALE

#if _MSC_VER > 1400
#define _USE_IOS_LOCALE
#endif

// CRT 全局 locale 和 iostream imbue locale 冲突
// VC 2005 问题 (_MSC_VER = 1400)
// VC 2010 正常 (_MSC_VER = 1600)
#if _MSC_VER <= 1400 && defined(_USE_CRT_LOCALE) && defined(_USE_IOS_LOCALE)
#error cannot use CRT global locale and iostream imbue locale at the same time, when _MSC_VER <= 1400
#endif

#ifdef _USE_CRT_LOCALE
    _tsetlocale(LC_ALL, _T(""));
#endif

#ifdef _USE_IOS_LOCALE
    // 设置 C++ iostream 标准 IO 流 locale
    std::locale loc(std::locale(""), std::locale::classic(), std::locale::numeric);
    std::cout.imbue(loc);
    std::cerr.imbue(loc);
    std::cin.imbue(loc);
#endif
}

BOOL CMtrlAlphaApp::InitModulePath(DWORD size)
{
    TCHAR* buf = new TCHAR[size];
    DWORD ret = GetModuleFileName(m_hInstance, buf, size);
    if (ret == 0) {
        MYTRACE("GetModuleFileName failed: %lu", GetLastError());
        delete[] buf;
        return FALSE;
    }
    m_ModulePath = buf;
    delete[] buf;
    return TRUE;
}

BOOL CMtrlAlphaApp::InitWorkDir(const _TCHAR* dir)
{
    _TCHAR* oldDir = _tgetcwd(0, 0);
    if (oldDir == 0)
        return FALSE;
    m_OldWorkDir = oldDir;
    free(oldDir);
    return (_tchdir(dir) == 0);
}

CString CMtrlAlphaApp::GetModuleDir() const
{
    int i = m_ModulePath.ReverseFind(_T('\\'));
    return (i == -1 ? _T("") : m_ModulePath.Left(i + 1));   // 没找到 '\' 时, 返回 ""
}

int CMtrlAlphaApp::Run()
{
    HWND hwnd = m_pMainWnd->GetSafeHwnd();
    BOOL windowed = TRUE;
    auto_ptr<RenderMtrlAlpha> render(new RenderMtrlAlpha());
    if (!render->Init(WIN_WIDTH, WIN_HEIGHT, hwnd, windowed, D3DDEVTYPE_HAL))
        return Exit(-1);

    auto_ptr<MtrlAlphaInput> input(new MtrlAlphaInput());
    m_GameMain.reset(new SGL::Main());
    m_GameMain->Init(render.get(), input.get());

    // 调整窗口大小
    CMainFrame* pFrmWnd = (CMainFrame*) m_pMainWnd;
    pFrmWnd->SetGameMain(m_GameMain.get());
    if (windowed)
        m_GameMain->AdjustWindowed(pFrmWnd->GetStyle(), pFrmWnd->GetExStyle(), pFrmWnd->GetMenu()->GetSafeHmenu());
    else
        m_GameMain->AdjustFullscreen();

    // 更新窗口显示
    m_pMainWnd->ShowWindow(m_nCmdShow);
    m_pMainWnd->UpdateWindow();

    // 消息循环
    int exitCode = m_GameMain->StartLoop(hwnd, SGL::MFCIdle, SGL::MFCPreTransMessage);

    // NOTE: 按照约定应调用 ExitInstance, 重载 Run 时, 不自动调用 ExitInstance
    return Exit(exitCode);
}
