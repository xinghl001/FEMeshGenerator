#include "stdafx.h"
#include "FEMeshGenerator.h"
#include "FEMeshGeneratorDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFEMeshGeneratorDlg 对话框

CFEMeshGeneratorDlg::CFEMeshGeneratorDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FEMESHGENERATOR_DIALOG, pParent)
	, m_L1(200.0)
	, m_L2(150.0)
	, m_W1(100.0)
	, m_W2(80.0)
	, m_t1(10.0)
	, m_t2(8.0)
	, m_t3(5.0)
	, m_R(20.0)
	, m_d(30.0)
	, m_HW(6.0)
	, m_LW(6.0)
	, m_k(2.0)
	, m_bDoubleWeld(TRUE)
	, m_weldDensity(2.0)
	, m_transitionDensity(3.0)
	, m_plateDensity(5.0)
	, m_templatePath(_T("data\\yao.inp"))
	, m_outputPath(_T("output_mesh.inp"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFEMeshGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
	// 几何参数 / Geometry Parameters
	DDX_Text(pDX, IDC_EDIT_L1, m_L1);
	DDX_Text(pDX, IDC_EDIT_L2, m_L2);
	DDX_Text(pDX, IDC_EDIT_W1, m_W1);
	DDX_Text(pDX, IDC_EDIT_W2, m_W2);
	DDX_Text(pDX, IDC_EDIT_T1, m_t1);
	DDX_Text(pDX, IDC_EDIT_T2, m_t2);
	DDX_Text(pDX, IDC_EDIT_T3, m_t3);
	DDX_Text(pDX, IDC_EDIT_R, m_R);
	DDX_Text(pDX, IDC_EDIT_D, m_d);
	
	// 焊缝参数 / Weld Parameters
	DDX_Text(pDX, IDC_EDIT_HW, m_HW);
	DDX_Text(pDX, IDC_EDIT_LW, m_LW);
	DDX_Text(pDX, IDC_EDIT_K, m_k);
	DDX_Check(pDX, IDC_CHECK_DOUBLE, m_bDoubleWeld);
	
	// 网格密度 / Mesh Density
	DDX_Text(pDX, IDC_EDIT_WELD_DENSITY, m_weldDensity);
	DDX_Text(pDX, IDC_EDIT_TRANS_DENSITY, m_transitionDensity);
	DDX_Text(pDX, IDC_EDIT_PLATE_DENSITY, m_plateDensity);
	
	// 文件路径 / File Paths
	DDX_Text(pDX, IDC_EDIT_TEMPLATE, m_templatePath);
	DDX_Text(pDX, IDC_EDIT_OUTPUT, m_outputPath);
}

BEGIN_MESSAGE_MAP(CFEMeshGeneratorDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BTN_GENERATE, &CFEMeshGeneratorDlg::OnBnClickedGenerateMesh)
	ON_BN_CLICKED(IDC_BTN_EXPORT, &CFEMeshGeneratorDlg::OnBnClickedExportInp)
	ON_BN_CLICKED(IDC_BTN_LOAD_TEMPLATE, &CFEMeshGeneratorDlg::OnBnClickedLoadTemplate)
	ON_BN_CLICKED(IDC_BTN_RESET, &CFEMeshGeneratorDlg::OnBnClickedResetParams)
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CFEMeshGeneratorDlg::OnBnClickedBrowseTemplate)
END_MESSAGE_MAP()

// CFEMeshGeneratorDlg 消息处理程序

BOOL CFEMeshGeneratorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 加载默认参数 / Load default parameters
	LoadDefaultParameters();

	// 创建OpenGL视图 / Create OpenGL view
	CRect rect;
	GetClientRect(&rect);
	rect.right = rect.right * 2 / 3;  // 左侧2/3区域 / Left 2/3 area
	m_glView.CreateGLWindow(this, rect, IDC_OPENGL_VIEW);
	m_glView.SetMeshGenerator(&m_meshGenerator);

	// 调整控件位置 / Adjust control positions
	ResizeControls();

	// 更新状态栏 / Update status bar
	UpdateStatusBar();

	return TRUE;
}

void CFEMeshGeneratorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CFEMeshGeneratorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFEMeshGeneratorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	
	if (m_glView.GetSafeHwnd() != NULL)
	{
		ResizeControls();
	}
}

void CFEMeshGeneratorDlg::ResizeControls()
{
	CRect rect;
	GetClientRect(&rect);
	
	if (rect.Width() > 0 && rect.Height() > 0)
	{
		// 调整OpenGL视图大小 / Resize OpenGL view
		int glWidth = rect.Width() * 2 / 3;
		m_glView.MoveWindow(0, 0, glWidth, rect.Height());
	}
}

void CFEMeshGeneratorDlg::LoadDefaultParameters()
{
	// 默认参数已在构造函数中设置 / Default parameters set in constructor
	UpdateData(FALSE);
}

void CFEMeshGeneratorDlg::UpdateMeshGenerator()
{
	UpdateData(TRUE);

	// 设置几何参数 / Set geometry parameters
	CWeldMeshGenerator::GeometryParams geomParams;
	geomParams.L1 = m_L1;
	geomParams.L2 = m_L2;
	geomParams.W1 = m_W1;
	geomParams.W2 = m_W2;
	geomParams.t1 = m_t1;
	geomParams.t2 = m_t2;
	geomParams.t3 = m_t3;
	geomParams.R = m_R;
	geomParams.d = m_d;
	m_meshGenerator.SetGeometryParams(geomParams);

	// 设置焊缝参数 / Set weld parameters
	CWeldMeshGenerator::WeldParams weldParams;
	weldParams.HW = m_HW;
	weldParams.LW = m_LW;
	weldParams.k = m_k;
	weldParams.isDouble = (m_bDoubleWeld != FALSE);
	m_meshGenerator.SetWeldParams(weldParams);

	// 设置网格密度 / Set mesh density
	CWeldMeshGenerator::MeshDensity meshDensity;
	meshDensity.weld = m_weldDensity;
	meshDensity.transition = m_transitionDensity;
	meshDensity.plate = m_plateDensity;
	m_meshGenerator.SetMeshDensity(meshDensity);
}

void CFEMeshGeneratorDlg::UpdateStatusBar()
{
	CString status;
	status.Format(_T("节点数: %d  单元数: %d"), 
		m_meshGenerator.GetNodeCount(), 
		m_meshGenerator.GetElementCount());
	SetDlgItemText(IDC_STATIC_STATUS, status);
}

void CFEMeshGeneratorDlg::OnBnClickedGenerateMesh()
{
	UpdateMeshGenerator();

	// 生成网格 / Generate mesh
	if (m_meshGenerator.GenerateMesh())
	{
		m_glView.RefreshView();
		UpdateStatusBar();
		MessageBox(_T("网格生成成功！"), _T("提示"), MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(_T("网格生成失败！"), _T("错误"), MB_OK | MB_ICONERROR);
	}
}

void CFEMeshGeneratorDlg::OnBnClickedExportInp()
{
	UpdateData(TRUE);

	if (m_meshGenerator.GetNodeCount() == 0)
	{
		MessageBox(_T("请先生成网格！"), _T("提示"), MB_OK | MB_ICONWARNING);
		return;
	}

	// 导出INP文件 / Export INP file
	if (m_meshGenerator.ExportToINP(m_outputPath))
	{
		CString msg;
		msg.Format(_T("INP文件已导出到: %s"), m_outputPath);
		MessageBox(msg, _T("提示"), MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(_T("INP文件导出失败！"), _T("错误"), MB_OK | MB_ICONERROR);
	}
}

void CFEMeshGeneratorDlg::OnBnClickedLoadTemplate()
{
	UpdateData(TRUE);

	// 加载模板文件 / Load template file
	if (m_meshGenerator.LoadWeldTemplate(m_templatePath))
	{
		MessageBox(_T("模板文件加载成功！"), _T("提示"), MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(_T("模板文件加载失败！"), _T("错误"), MB_OK | MB_ICONERROR);
	}
}

void CFEMeshGeneratorDlg::OnBnClickedResetParams()
{
	LoadDefaultParameters();
	MessageBox(_T("参数已重置为默认值！"), _T("提示"), MB_OK | MB_ICONINFORMATION);
}

void CFEMeshGeneratorDlg::OnBnClickedBrowseTemplate()
{
	CFileDialog dlg(TRUE, _T("inp"), NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("INP文件 (*.inp)|*.inp|所有文件 (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		m_templatePath = dlg.GetPathName();
		UpdateData(FALSE);
	}
}
