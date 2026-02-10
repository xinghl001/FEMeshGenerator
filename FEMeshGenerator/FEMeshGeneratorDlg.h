#pragma once

#include "OpenGLView.h"
#include "WeldMeshGenerator.h"

// CFEMeshGeneratorDlg 对话框
class CFEMeshGeneratorDlg : public CDialogEx
{
// 构造
public:
	CFEMeshGeneratorDlg(CWnd* pParent = nullptr);

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FEMESHGENERATOR_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedGenerateMesh();
	afx_msg void OnBnClickedExportInp();
	afx_msg void OnBnClickedLoadTemplate();
	afx_msg void OnBnClickedResetParams();
	afx_msg void OnBnClickedBrowseTemplate();
	DECLARE_MESSAGE_MAP()

private:
	// OpenGL视图 / OpenGL View
	COpenGLView m_glView;
	
	// 网格生成器 / Mesh Generator
	CWeldMeshGenerator m_meshGenerator;

	// 控件变量 / Control Variables
	// 几何参数 / Geometry Parameters
	double m_L1, m_L2, m_W1, m_W2;
	double m_t1, m_t2, m_t3;
	double m_R, m_d;
	
	// 焊缝参数 / Weld Parameters
	double m_HW, m_LW, m_k;
	BOOL m_bDoubleWeld;
	
	// 网格密度 / Mesh Density
	double m_weldDensity;
	double m_transitionDensity;
	double m_plateDensity;
	
	// 文件路径 / File Paths
	CString m_templatePath;
	CString m_outputPath;

	// 辅助方法 / Helper Methods
	void UpdateStatusBar();
	void LoadDefaultParameters();
	void ResizeControls();
	void UpdateMeshGenerator();
};
