#pragma once

#include <gl/GL.h>
#include <gl/GLU.h>

class CWeldMeshGenerator;

// OpenGL 3D视图类 / OpenGL 3D View Class
class COpenGLView : public CWnd
{
public:
	COpenGLView();
	virtual ~COpenGLView();

	// 设置网格生成器 / Set mesh generator
	void SetMeshGenerator(CWeldMeshGenerator* pGenerator);
	
	// 刷新视图 / Refresh view
	void RefreshView();
	
	// 创建OpenGL窗口 / Create OpenGL window
	BOOL CreateGLWindow(CWnd* pParent, const CRect& rect, UINT nID);

protected:
	DECLARE_MESSAGE_MAP()
	
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

private:
	// OpenGL初始化和清理 / OpenGL initialization and cleanup
	BOOL InitializeOpenGL();
	void CleanupOpenGL();
	
	// 渲染方法 / Rendering methods
	void RenderScene();
	void DrawMesh();
	void DrawAxes();
	void SetupViewport();
	void SetupProjection();

private:
	// OpenGL上下文 / OpenGL context
	HGLRC m_hRC;           // 渲染上下文 / Rendering context
	CDC* m_pDC;            // 设备上下文 / Device context

	// 视图参数 / View parameters
	float m_rotateX;       // X轴旋转角度 / X-axis rotation
	float m_rotateY;       // Y轴旋转角度 / Y-axis rotation
	float m_zoom;          // 缩放因子 / Zoom factor
	
	// 鼠标交互 / Mouse interaction
	BOOL m_bDragging;      // 是否正在拖动 / Is dragging
	CPoint m_lastPoint;    // 上次鼠标位置 / Last mouse position

	// 网格数据 / Mesh data
	CWeldMeshGenerator* m_pMeshGenerator;
};
