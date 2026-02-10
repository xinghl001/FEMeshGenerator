#include "stdafx.h"
#include "FEMeshGenerator.h"
#include "FEMeshGeneratorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFEMeshGeneratorApp

BEGIN_MESSAGE_MAP(CFEMeshGeneratorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CFEMeshGeneratorApp 构造

CFEMeshGeneratorApp::CFEMeshGeneratorApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

// 唯一的 CFEMeshGeneratorApp 对象

CFEMeshGeneratorApp theApp;

// CFEMeshGeneratorApp 初始化

BOOL CFEMeshGeneratorApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// 标准初始化
	SetRegistryKey(_T("FEMeshGenerator"));

	CFEMeshGeneratorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}
