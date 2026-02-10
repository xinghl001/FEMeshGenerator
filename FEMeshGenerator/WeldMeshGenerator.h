#pragma once

#include <vector>
#include <string>

// 3D节点结构 / 3D Node Structure
struct Node3D
{
	int id;           // 节点ID / Node ID
	double x, y, z;   // 坐标 / Coordinates
	
	Node3D() : id(0), x(0), y(0), z(0) {}
	Node3D(int _id, double _x, double _y, double _z) 
		: id(_id), x(_x), y(_y), z(_z) {}
};

// 3D单元结构 / 3D Element Structure
struct Element3D
{
	int id;                    // 单元ID / Element ID
	std::vector<int> nodeIds;  // 节点ID列表 / Node ID list (8 for C3D8R)
	
	Element3D() : id(0) {}
	Element3D(int _id) : id(_id) {}
};

// 焊缝网格生成器类 / Weld Mesh Generator Class
class CWeldMeshGenerator
{
public:
	// 几何参数结构 / Geometry Parameters Structure
	struct GeometryParams {
		double L1, L2;      // 长度 / Length
		double W1, W2;      // 宽度 / Width
		double t1, t2, t3;  // 厚度 / Thickness
		double R, d;        // 半径、距离 / Radius, Distance
		
		GeometryParams() : L1(200), L2(150), W1(100), W2(80), 
			t1(10), t2(8), t3(5), R(20), d(30) {}
	};
	
	// 焊缝参数结构 / Weld Parameters Structure
	struct WeldParams {
		double HW, LW;      // 焊缝高度、长度 / Weld height, length
		double k;           // 幂曲线系数 / Power curve coefficient
		bool isDouble;      // 是否双边 / Is double-sided
		
		WeldParams() : HW(6.0), LW(6.0), k(2.0), isDouble(true) {}
	};
	
	// 网格密度结构 / Mesh Density Structure
	struct MeshDensity {
		double weld;        // 焊缝密度 / Weld density
		double transition;  // 过渡密度 / Transition density
		double plate;       // 平板密度 / Plate density
		
		MeshDensity() : weld(2.0), transition(3.0), plate(5.0) {}
	};

public:
	CWeldMeshGenerator();
	~CWeldMeshGenerator();

	// 主要方法 / Main Methods
	bool LoadWeldTemplate(const CString& filename);      // 加载模板 / Load template
	bool GenerateMesh();                                 // 生成网格 / Generate mesh
	bool ExportToINP(const CString& filename);          // 导出INP / Export INP

	// 参数设置 / Parameter Settings
	void SetGeometryParams(const GeometryParams& params) { m_geomParams = params; }
	void SetWeldParams(const WeldParams& params) { m_weldParams = params; }
	void SetMeshDensity(const MeshDensity& density) { m_meshDensity = density; }

	// 获取数据 / Get Data
	const std::vector<Node3D>& GetNodes() const { return m_nodes; }
	const std::vector<Element3D>& GetElements() const { return m_elements; }
	int GetNodeCount() const { return (int)m_nodes.size(); }
	int GetElementCount() const { return (int)m_elements.size(); }

	// 获取参数 / Get Parameters
	const GeometryParams& GetGeometryParams() const { return m_geomParams; }
	const WeldParams& GetWeldParams() const { return m_weldParams; }
	const MeshDensity& GetMeshDensity() const { return m_meshDensity; }

private:
	// 核心算法 / Core Algorithms
	double PowerCurveDeformation(double h1, double h2, double x);  // 幂曲线 / Power curve
	void SweepWeldAlongPath();                                     // 扫描拉伸 / Sweep extrusion
	void GenerateTransitionMesh();                                 // 过渡网格 / Transition mesh
	void GeneratePlateMesh();                                      // 平板网格 / Plate mesh
	
	// 辅助方法 / Helper Methods
	bool LoadTemplateNodes(const CString& filename);
	bool LoadTemplateElements(const CString& filename);
	void ClearMesh();
	int GetNextNodeId() const { return m_nextNodeId; }
	int GetNextElementId() const { return m_nextElementId; }

private:
	// 参数 / Parameters
	GeometryParams m_geomParams;
	WeldParams m_weldParams;
	MeshDensity m_meshDensity;

	// 数据 / Data
	std::vector<Node3D> m_nodes;
	std::vector<Element3D> m_elements;
	std::vector<Node3D> m_templateNodes;      // 模板节点 / Template nodes
	std::vector<Element3D> m_templateElements; // 模板单元 / Template elements

	// ID管理 / ID Management
	int m_nextNodeId;
	int m_nextElementId;

	// 状态 / Status
	bool m_templateLoaded;
	CString m_lastError;
};
