# FEMeshGenerator API 文档

## 目录

1. [类结构概览](#类结构概览)
2. [CWeldMeshGenerator 类](#cweldmeshgenerator-类)
3. [COpenGLView 类](#copenglview-类)
4. [CFEMeshGeneratorDlg 类](#cfemeshgeneratordlg-类)
5. [数据结构](#数据结构)
6. [使用示例](#使用示例)

## 类结构概览

```
CFEMeshGeneratorApp (MFC Application)
    └── CFEMeshGeneratorDlg (Main Dialog)
            ├── CWeldMeshGenerator (Mesh Generator)
            │       ├── Node3D (Node Structure)
            │       ├── Element3D (Element Structure)
            │       ├── GeometryParams (Geometry Parameters)
            │       ├── WeldParams (Weld Parameters)
            │       └── MeshDensity (Mesh Density)
            └── COpenGLView (3D Visualization)
```

## CWeldMeshGenerator 类

网格生成核心类，负责所有网格生成相关的算法实现。

### 类声明

```cpp
class CWeldMeshGenerator
{
public:
    CWeldMeshGenerator();
    ~CWeldMeshGenerator();
    
    // 主要方法
    bool LoadWeldTemplate(const CString& filename);
    bool GenerateMesh();
    bool ExportToINP(const CString& filename);
    
    // 参数设置
    void SetGeometryParams(const GeometryParams& params);
    void SetWeldParams(const WeldParams& params);
    void SetMeshDensity(const MeshDensity& density);
    
    // 数据访问
    const std::vector<Node3D>& GetNodes() const;
    const std::vector<Element3D>& GetElements() const;
    int GetNodeCount() const;
    int GetElementCount() const;
    
private:
    // 核心算法
    double PowerCurveDeformation(double h1, double h2, double x);
    void SweepWeldAlongPath();
    void GenerateTransitionMesh();
    void GeneratePlateMesh();
    void ClearMesh();
};
```

### 内部结构体

#### GeometryParams - 几何参数

```cpp
struct GeometryParams {
    double L1, L2;      // 长度 (mm)
    double W1, W2;      // 宽度 (mm)
    double t1, t2, t3;  // 厚度 (mm)
    double R, d;        // 半径、距离 (mm)
    
    // 默认构造函数
    GeometryParams();
};
```

**成员说明**：
- `L1`：主板长度，默认 200mm
- `L2`：副板长度，默认 150mm
- `W1`：主板宽度，默认 100mm
- `W2`：副板宽度，默认 80mm
- `t1`：主板厚度，默认 10mm
- `t2`：副板厚度，默认 8mm
- `t3`：过渡厚度，默认 5mm
- `R`：缺口半径，默认 20mm
- `d`：肋骨距离，默认 30mm

#### WeldParams - 焊缝参数

```cpp
struct WeldParams {
    double HW, LW;      // 焊缝高度、长度 (mm)
    double k;           // 幂曲线系数
    bool isDouble;      // 是否双边焊缝
    
    // 默认构造函数
    WeldParams();
};
```

**成员说明**：
- `HW`：焊缝高度，默认 6.0mm
- `LW`：焊缝长度，默认 6.0mm
- `k`：幂曲线系数，默认 2.0
- `isDouble`：是否双边焊缝，默认 true

#### MeshDensity - 网格密度

```cpp
struct MeshDensity {
    double weld;        // 焊缝区密度 (mm)
    double transition;  // 过渡区密度 (mm)
    double plate;       // 平板区密度 (mm)
    
    // 默认构造函数
    MeshDensity();
};
```

**成员说明**：
- `weld`：焊缝区网格密度，默认 2.0mm
- `transition`：过渡区网格密度，默认 3.0mm
- `plate`：平板区网格密度，默认 5.0mm

### 公共方法

#### LoadWeldTemplate

```cpp
bool LoadWeldTemplate(const CString& filename);
```

**功能**：加载焊缝截面模板文件（INP格式）

**参数**：
- `filename`：模板文件路径（如 "data\\yao.inp"）

**返回值**：
- `true`：加载成功
- `false`：加载失败

**使用示例**：
```cpp
CWeldMeshGenerator generator;
if (generator.LoadWeldTemplate(_T("data\\yao.inp"))) {
    // 模板加载成功
} else {
    // 处理错误
}
```

**注意事项**：
- 必须先加载模板才能生成网格
- 模板文件必须包含 `*NODE` 和 `*ELEMENT` 部分
- 节点坐标 Z 值应为 0（2D 截面）

#### GenerateMesh

```cpp
bool GenerateMesh();
```

**功能**：根据当前参数生成完整的三维网格

**返回值**：
- `true`：生成成功
- `false`：生成失败

**使用示例**：
```cpp
// 设置参数
CWeldMeshGenerator::GeometryParams geom;
geom.L1 = 200.0;
geom.t1 = 10.0;
generator.SetGeometryParams(geom);

// 生成网格
if (generator.GenerateMesh()) {
    int nodeCount = generator.GetNodeCount();
    int elemCount = generator.GetElementCount();
    // 使用生成的网格
}
```

**生成流程**：
1. 清空现有网格数据
2. 执行扫描拉伸算法生成焊缝区
3. 生成过渡区网格
4. 生成平板区网格

#### ExportToINP

```cpp
bool ExportToINP(const CString& filename);
```

**功能**：将生成的网格导出为 Abaqus INP 格式文件

**参数**：
- `filename`：输出文件路径（如 "output.inp"）

**返回值**：
- `true`：导出成功
- `false`：导出失败

**使用示例**：
```cpp
if (generator.ExportToINP(_T("my_mesh.inp"))) {
    // 文件导出成功
}
```

**输出格式**：
```
*HEADING
T-Joint Fillet Weld Structure
*NODE
1, 0.0, 0.0, 0.0
2, 1.0, 0.0, 0.0
...
*ELEMENT, TYPE=C3D8R
1, 1, 2, 3, 4, 5, 6, 7, 8
...
*END
```

#### SetGeometryParams

```cpp
void SetGeometryParams(const GeometryParams& params);
```

**功能**：设置几何参数

**参数**：
- `params`：GeometryParams 结构体

**使用示例**：
```cpp
CWeldMeshGenerator::GeometryParams geom;
geom.L1 = 250.0;
geom.L2 = 180.0;
geom.W1 = 120.0;
geom.t1 = 12.0;
generator.SetGeometryParams(geom);
```

#### SetWeldParams

```cpp
void SetWeldParams(const WeldParams& params);
```

**功能**：设置焊缝参数

**参数**：
- `params`：WeldParams 结构体

**使用示例**：
```cpp
CWeldMeshGenerator::WeldParams weld;
weld.HW = 8.0;
weld.LW = 8.0;
weld.k = 2.5;
weld.isDouble = true;
generator.SetWeldParams(weld);
```

#### SetMeshDensity

```cpp
void SetMeshDensity(const MeshDensity& density);
```

**功能**：设置网格密度

**参数**：
- `density`：MeshDensity 结构体

**使用示例**：
```cpp
CWeldMeshGenerator::MeshDensity density;
density.weld = 1.5;
density.transition = 2.5;
density.plate = 4.0;
generator.SetMeshDensity(density);
```

### 私有方法

#### PowerCurveDeformation

```cpp
double PowerCurveDeformation(double h1, double h2, double x);
```

**功能**：计算幂曲线变形

**公式**：`y = h2 * (1 - (x/h1)^k)`

**参数**：
- `h1`：焊缝长度
- `h2`：焊缝高度
- `x`：当前位置（0 到 h1）

**返回值**：变形量 y

#### SweepWeldAlongPath

```cpp
void SweepWeldAlongPath();
```

**功能**：沿路径扫描拉伸焊缝截面生成三维网格

**算法步骤**：
1. 计算扫描路径点数
2. 为每个截面复制模板节点
3. 应用幂曲线变形
4. 连接相邻截面生成六面体单元

## COpenGLView 类

OpenGL 3D 可视化类，负责渲染和交互。

### 类声明

```cpp
class COpenGLView : public CWnd
{
public:
    COpenGLView();
    virtual ~COpenGLView();
    
    // 主要方法
    BOOL CreateGLWindow(CWnd* pParent, const CRect& rect, UINT nID);
    void SetMeshGenerator(CWeldMeshGenerator* pGenerator);
    void RefreshView();
    
protected:
    // 消息处理
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnPaint();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    
private:
    // OpenGL 方法
    BOOL InitializeOpenGL();
    void CleanupOpenGL();
    void RenderScene();
    void DrawMesh();
    void DrawAxes();
    void SetupViewport();
    void SetupProjection();
};
```

### 公共方法

#### CreateGLWindow

```cpp
BOOL CreateGLWindow(CWnd* pParent, const CRect& rect, UINT nID);
```

**功能**：创建 OpenGL 窗口

**参数**：
- `pParent`：父窗口指针
- `rect`：窗口矩形区域
- `nID`：控件 ID

**返回值**：
- `TRUE`：创建成功
- `FALSE`：创建失败

**使用示例**：
```cpp
COpenGLView m_glView;
CRect rect(0, 0, 400, 400);
m_glView.CreateGLWindow(this, rect, IDC_OPENGL_VIEW);
```

#### SetMeshGenerator

```cpp
void SetMeshGenerator(CWeldMeshGenerator* pGenerator);
```

**功能**：设置要显示的网格生成器

**参数**：
- `pGenerator`：CWeldMeshGenerator 指针

**使用示例**：
```cpp
m_glView.SetMeshGenerator(&m_meshGenerator);
```

#### RefreshView

```cpp
void RefreshView();
```

**功能**：刷新 OpenGL 视图

**使用示例**：
```cpp
// 生成网格后刷新显示
if (m_meshGenerator.GenerateMesh()) {
    m_glView.RefreshView();
}
```

### 交互功能

#### 鼠标旋转

- **操作**：按住鼠标左键拖动
- **实现**：
```cpp
void COpenGLView::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_bDragging) {
        int dx = point.x - m_lastPoint.x;
        int dy = point.y - m_lastPoint.y;
        m_rotateY += dx * 0.5f;
        m_rotateX += dy * 0.5f;
        RefreshView();
    }
}
```

#### 鼠标缩放

- **操作**：滚动鼠标滚轮
- **实现**：
```cpp
BOOL COpenGLView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    m_zoom -= zDelta / 1200.0f;
    if (m_zoom < 0.1f) m_zoom = 0.1f;
    if (m_zoom > 5.0f) m_zoom = 5.0f;
    RefreshView();
    return TRUE;
}
```

## CFEMeshGeneratorDlg 类

主对话框类，管理用户界面和协调各组件。

### 类声明

```cpp
class CFEMeshGeneratorDlg : public CDialogEx
{
public:
    CFEMeshGeneratorDlg(CWnd* pParent = nullptr);
    
protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);
    
    // 消息处理
    afx_msg void OnBnClickedGenerateMesh();
    afx_msg void OnBnClickedExportInp();
    afx_msg void OnBnClickedLoadTemplate();
    afx_msg void OnBnClickedResetParams();
    afx_msg void OnBnClickedBrowseTemplate();
    
private:
    void UpdateStatusBar();
    void LoadDefaultParameters();
    void ResizeControls();
    void UpdateMeshGenerator();
    
    COpenGLView m_glView;
    CWeldMeshGenerator m_meshGenerator;
};
```

### 主要方法

#### OnBnClickedGenerateMesh

```cpp
void CFEMeshGeneratorDlg::OnBnClickedGenerateMesh()
{
    UpdateMeshGenerator();
    
    if (m_meshGenerator.GenerateMesh()) {
        m_glView.RefreshView();
        UpdateStatusBar();
        MessageBox(_T("网格生成成功！"), _T("提示"), MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBox(_T("网格生成失败！"), _T("错误"), MB_OK | MB_ICONERROR);
    }
}
```

## 数据结构

### Node3D - 节点结构

```cpp
struct Node3D
{
    int id;           // 节点 ID
    double x, y, z;   // 坐标
    
    Node3D();
    Node3D(int _id, double _x, double _y, double _z);
};
```

**使用示例**：
```cpp
Node3D node(1, 0.0, 0.0, 0.0);
node.x = 10.0;
```

### Element3D - 单元结构

```cpp
struct Element3D
{
    int id;                    // 单元 ID
    std::vector<int> nodeIds;  // 节点 ID 列表（C3D8R 为 8 个）
    
    Element3D();
    Element3D(int _id);
};
```

**使用示例**：
```cpp
Element3D elem(1);
elem.nodeIds.push_back(1);
elem.nodeIds.push_back(2);
// ... 添加 8 个节点
```

## 使用示例

### 完整使用流程

```cpp
// 1. 创建网格生成器
CWeldMeshGenerator generator;

// 2. 加载模板
if (!generator.LoadWeldTemplate(_T("data\\yao.inp"))) {
    // 处理错误
    return;
}

// 3. 设置参数
CWeldMeshGenerator::GeometryParams geom;
geom.L1 = 200.0;
geom.W1 = 100.0;
geom.t1 = 10.0;
generator.SetGeometryParams(geom);

CWeldMeshGenerator::WeldParams weld;
weld.HW = 6.0;
weld.LW = 6.0;
weld.k = 2.0;
generator.SetWeldParams(weld);

CWeldMeshGenerator::MeshDensity density;
density.weld = 2.0;
density.transition = 3.0;
density.plate = 5.0;
generator.SetMeshDensity(density);

// 4. 生成网格
if (!generator.GenerateMesh()) {
    // 处理错误
    return;
}

// 5. 获取结果
int nodeCount = generator.GetNodeCount();
int elemCount = generator.GetElementCount();

// 6. 导出文件
generator.ExportToINP(_T("output.inp"));
```

### 自定义网格生成

```cpp
// 继承并扩展 CWeldMeshGenerator
class CCustomMeshGenerator : public CWeldMeshGenerator
{
public:
    void CustomizeWeldShape() {
        // 自定义焊缝形状算法
    }
    
    void AddCustomRegion() {
        // 添加自定义区域
    }
};
```

## 错误处理

### 返回值检查

```cpp
// 推荐的错误处理模式
if (!generator.LoadWeldTemplate(_T("data\\yao.inp"))) {
    CString error = _T("模板加载失败");
    AfxMessageBox(error, MB_ICONERROR);
    return FALSE;
}

if (!generator.GenerateMesh()) {
    CString error = _T("网格生成失败");
    AfxMessageBox(error, MB_ICONERROR);
    return FALSE;
}

if (!generator.ExportToINP(_T("output.inp"))) {
    CString error = _T("文件导出失败");
    AfxMessageBox(error, MB_ICONERROR);
    return FALSE;
}
```

## 性能考虑

### 网格规模估算

网格规模受以下因素影响：

**节点数估算**：
```
nodes ≈ (LW/weld_density) * template_node_count + 
        (L1/plate_density) * (W1/plate_density) * (t1/plate_density)
```

**单元数估算**：
```
elements ≈ (LW/weld_density) * template_element_count + 
           (L1/plate_density) * (W1/plate_density) * (t1/plate_density)
```

### 优化建议

1. **参数范围**：
   - 节点数 < 10万：交互流畅
   - 节点数 10-50万：可接受
   - 节点数 > 50万：考虑增大密度值

2. **渲染优化**：
   - 使用显示列表缓存几何
   - 仅在参数改变时重新生成
   - 考虑 LOD（细节层次）显示

3. **内存管理**：
   - 使用 `std::vector::reserve()` 预分配
   - 及时释放临时数据
   - 避免不必要的深拷贝

## 扩展开发

### 添加新的单元类型

```cpp
// 在 WeldMeshGenerator.h 中添加
enum ElementType {
    C3D8R,     // 现有：8节点六面体
    C3D20R,    // 新增：20节点六面体
    C3D4       // 新增：4节点四面体
};

// 在生成算法中处理不同类型
void GenerateElement(ElementType type) {
    switch(type) {
        case C3D8R:
            GenerateHex8();
            break;
        case C3D20R:
            GenerateHex20();
            break;
        // ...
    }
}
```

### 添加新的变形算法

```cpp
// 添加到 WeldMeshGenerator 类
double SinusoidalDeformation(double h1, double h2, double x) {
    return h2 * sin(M_PI * x / (2 * h1));
}

double ExponentialDeformation(double h1, double h2, double x) {
    return h2 * (1 - exp(-x / h1));
}
```

---

**更新日期**：2026-02-10
**版本**：1.0.0
