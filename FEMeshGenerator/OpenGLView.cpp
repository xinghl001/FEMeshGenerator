#include "stdafx.h"
#include "OpenGLView.h"
#include "WeldMeshGenerator.h"
#include <cmath>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

BEGIN_MESSAGE_MAP(COpenGLView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

COpenGLView::COpenGLView()
	: m_hRC(NULL)
	, m_pDC(NULL)
	, m_rotateX(30.0f)
	, m_rotateY(45.0f)
	, m_zoom(1.0f)
	, m_bDragging(FALSE)
	, m_pMeshGenerator(NULL)
{
}

COpenGLView::~COpenGLView()
{
	CleanupOpenGL();
}

BOOL COpenGLView::CreateGLWindow(CWnd* pParent, const CRect& rect, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
		NULL, (HBRUSH)GetStockObject(BLACK_BRUSH), NULL);

	return CreateEx(0, className, _T("OpenGLView"), 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		rect, pParent, nID);
}

int COpenGLView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!InitializeOpenGL())
	{
		MessageBox(_T("无法初始化OpenGL"), _T("错误"), MB_OK | MB_ICONERROR);
		return -1;
	}

	return 0;
}

void COpenGLView::OnDestroy()
{
	CleanupOpenGL();
	CWnd::OnDestroy();
}

BOOL COpenGLView::InitializeOpenGL()
{
	m_pDC = GetDC();
	if (m_pDC == NULL)
		return FALSE;

	// 设置像素格式 / Set pixel format
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,         // 颜色深度 / Color depth
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		32,         // 深度缓冲 / Depth buffer
		0, 0,
		PFD_MAIN_PLANE,
		0, 0, 0, 0
	};

	int pixelFormat = ChoosePixelFormat(m_pDC->m_hDC, &pfd);
	if (pixelFormat == 0)
		return FALSE;

	if (!SetPixelFormat(m_pDC->m_hDC, pixelFormat, &pfd))
		return FALSE;

	// 创建渲染上下文 / Create rendering context
	m_hRC = wglCreateContext(m_pDC->m_hDC);
	if (m_hRC == NULL)
		return FALSE;

	if (!wglMakeCurrent(m_pDC->m_hDC, m_hRC))
		return FALSE;

	// 设置OpenGL状态 / Set OpenGL states
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
	glClearDepth(1.0f);
	
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	
	glShadeModel(GL_SMOOTH);

	return TRUE;
}

void COpenGLView::CleanupOpenGL()
{
	if (m_hRC != NULL)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(m_hRC);
		m_hRC = NULL;
	}

	if (m_pDC != NULL)
	{
		ReleaseDC(m_pDC);
		m_pDC = NULL;
	}
}

void COpenGLView::OnPaint()
{
	CPaintDC dc(this);
	
	if (m_hRC != NULL && m_pDC != NULL)
	{
		wglMakeCurrent(m_pDC->m_hDC, m_hRC);
		RenderScene();
		SwapBuffers(m_pDC->m_hDC);
	}
}

void COpenGLView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (m_hRC != NULL && cy > 0)
	{
		wglMakeCurrent(m_pDC->m_hDC, m_hRC);
		SetupViewport();
	}
}

void COpenGLView::SetupViewport()
{
	CRect rect;
	GetClientRect(&rect);

	glViewport(0, 0, rect.Width(), rect.Height());
	SetupProjection();
}

void COpenGLView::SetupProjection()
{
	CRect rect;
	GetClientRect(&rect);
	
	double aspect = (double)rect.Width() / (double)rect.Height();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, aspect, 1.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}

void COpenGLView::RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// 设置相机位置 / Set camera position
	glTranslatef(0.0f, 0.0f, -200.0f * m_zoom);
	
	// 应用旋转 / Apply rotation
	glRotatef(m_rotateX, 1.0f, 0.0f, 0.0f);
	glRotatef(m_rotateY, 0.0f, 1.0f, 0.0f);

	// 绘制坐标轴 / Draw axes
	DrawAxes();

	// 绘制网格 / Draw mesh
	DrawMesh();
}

void COpenGLView::DrawAxes()
{
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	
	// X轴 - 红色 / X-axis - Red
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(50.0f, 0.0f, 0.0f);
	
	// Y轴 - 绿色 / Y-axis - Green
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 50.0f, 0.0f);
	
	// Z轴 - 蓝色 / Z-axis - Blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 50.0f);
	
	glEnd();
	glLineWidth(1.0f);
}

void COpenGLView::DrawMesh()
{
	if (m_pMeshGenerator == NULL)
		return;

	const std::vector<Node3D>& nodes = m_pMeshGenerator->GetNodes();
	const std::vector<Element3D>& elements = m_pMeshGenerator->GetElements();

	if (nodes.empty() || elements.empty())
		return;

	// 绘制单元边缘 / Draw element edges
	glColor3f(0.7f, 0.7f, 0.7f);
	glLineWidth(1.0f);

	for (const auto& elem : elements)
	{
		if (elem.nodeIds.size() == 8)  // C3D8R六面体 / C3D8R hexahedron
		{
			// 查找节点坐标 / Find node coordinates
			std::vector<Node3D> elemNodes;
			for (int nodeId : elem.nodeIds)
			{
				for (const auto& node : nodes)
				{
					if (node.id == nodeId)
					{
						elemNodes.push_back(node);
						break;
					}
				}
			}

			if (elemNodes.size() == 8)
			{
				// 绘制12条边 / Draw 12 edges
				glBegin(GL_LINES);
				
				// 底面4条边 / Bottom 4 edges
				for (int i = 0; i < 4; i++)
				{
					int next = (i + 1) % 4;
					glVertex3f((float)elemNodes[i].x, (float)elemNodes[i].y, (float)elemNodes[i].z);
					glVertex3f((float)elemNodes[next].x, (float)elemNodes[next].y, (float)elemNodes[next].z);
				}
				
				// 顶面4条边 / Top 4 edges
				for (int i = 4; i < 8; i++)
				{
					int next = (i == 7) ? 4 : (i + 1);
					glVertex3f((float)elemNodes[i].x, (float)elemNodes[i].y, (float)elemNodes[i].z);
					glVertex3f((float)elemNodes[next].x, (float)elemNodes[next].y, (float)elemNodes[next].z);
				}
				
				// 垂直4条边 / 4 vertical edges
				for (int i = 0; i < 4; i++)
				{
					glVertex3f((float)elemNodes[i].x, (float)elemNodes[i].y, (float)elemNodes[i].z);
					glVertex3f((float)elemNodes[i + 4].x, (float)elemNodes[i + 4].y, (float)elemNodes[i + 4].z);
				}
				
				glEnd();
			}
		}
	}

	// 绘制节点 / Draw nodes
	glColor3f(1.0f, 1.0f, 0.0f);
	glPointSize(3.0f);
	glBegin(GL_POINTS);
	for (const auto& node : nodes)
	{
		glVertex3f((float)node.x, (float)node.y, (float)node.z);
	}
	glEnd();
}

void COpenGLView::SetMeshGenerator(CWeldMeshGenerator* pGenerator)
{
	m_pMeshGenerator = pGenerator;
	RefreshView();
}

void COpenGLView::RefreshView()
{
	if (m_hRC != NULL && m_pDC != NULL)
	{
		Invalidate(FALSE);
	}
}

void COpenGLView::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bDragging = TRUE;
	m_lastPoint = point;
	SetCapture();
	CWnd::OnLButtonDown(nFlags, point);
}

void COpenGLView::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDragging = FALSE;
	ReleaseCapture();
	CWnd::OnLButtonUp(nFlags, point);
}

void COpenGLView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDragging)
	{
		int dx = point.x - m_lastPoint.x;
		int dy = point.y - m_lastPoint.y;

		m_rotateY += dx * 0.5f;
		m_rotateX += dy * 0.5f;

		// 限制X轴旋转范围 / Limit X-axis rotation range
		if (m_rotateX > 89.0f) m_rotateX = 89.0f;
		if (m_rotateX < -89.0f) m_rotateX = -89.0f;

		m_lastPoint = point;
		RefreshView();
	}

	CWnd::OnMouseMove(nFlags, point);
}

BOOL COpenGLView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// 调整缩放 / Adjust zoom
	m_zoom -= zDelta / 1200.0f;
	
	// 限制缩放范围 / Limit zoom range
	if (m_zoom < 0.1f) m_zoom = 0.1f;
	if (m_zoom > 5.0f) m_zoom = 5.0f;

	RefreshView();
	return TRUE;
}

BOOL COpenGLView::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;  // 防止闪烁 / Prevent flickering
}
