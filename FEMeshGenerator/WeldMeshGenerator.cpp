#include "stdafx.h"
#include "WeldMeshGenerator.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

CWeldMeshGenerator::CWeldMeshGenerator()
	: m_nextNodeId(1)
	, m_nextElementId(1)
	, m_templateLoaded(false)
{
}

CWeldMeshGenerator::~CWeldMeshGenerator()
{
}

// 加载焊缝模板文件 / Load weld template file
bool CWeldMeshGenerator::LoadWeldTemplate(const CString& filename)
{
	m_templateNodes.clear();
	m_templateElements.clear();
	m_templateLoaded = false;

	// 转换路径 / Convert path
	CStringA filenameA(filename);
	std::ifstream file(filenameA.GetString());
	
	if (!file.is_open())
	{
		m_lastError = _T("无法打开模板文件");
		return false;
	}

	std::string line;
	bool readingNodes = false;
	bool readingElements = false;

	while (std::getline(file, line))
	{
		// 跳过空行和注释 / Skip empty lines and comments
		if (line.empty() || line[0] == '*')
		{
			if (line.find("*NODE") != std::string::npos)
			{
				readingNodes = true;
				readingElements = false;
				continue;
			}
			else if (line.find("*ELEMENT") != std::string::npos)
			{
				readingNodes = false;
				readingElements = true;
				continue;
			}
			else if (line.find("*****") != std::string::npos)
			{
				// 结束标记 / End marker
				break;
			}
			continue;
		}

		if (readingNodes)
		{
			// 解析节点行 / Parse node line
			std::istringstream iss(line);
			int id;
			double x, y, z;
			char comma;

			iss >> id >> comma >> x >> comma >> y >> comma >> z;
			
			if (!iss.fail())
			{
				m_templateNodes.push_back(Node3D(id, x, y, z));
			}
		}
		else if (readingElements)
		{
			// 解析单元行 / Parse element line
			std::istringstream iss(line);
			int id;
			char comma;

			iss >> id >> comma;
			
			if (!iss.fail())
			{
				Element3D elem(id);
				int nodeId;
				while (iss >> nodeId)
				{
					elem.nodeIds.push_back(nodeId);
					iss >> comma; // 读取逗号 / Read comma
				}
				m_templateElements.push_back(elem);
			}
		}
	}

	file.close();

	if (m_templateNodes.empty())
	{
		m_lastError = _T("模板文件中没有节点数据");
		return false;
	}

	m_templateLoaded = true;
	return true;
}

// 生成完整网格 / Generate complete mesh
bool CWeldMeshGenerator::GenerateMesh()
{
	if (!m_templateLoaded)
	{
		m_lastError = _T("未加载模板文件");
		return false;
	}

	ClearMesh();

	// 生成焊缝区域 / Generate weld region
	SweepWeldAlongPath();

	// 生成过渡区域 / Generate transition region
	GenerateTransitionMesh();

	// 生成平板区域 / Generate plate region
	GeneratePlateMesh();

	return true;
}

// 幂曲线变形公式 / Power curve deformation formula
// y = h2 * (1 - (x/h1)^k)
double CWeldMeshGenerator::PowerCurveDeformation(double h1, double h2, double x)
{
	if (x < 0 || x > h1 || h1 <= 0)
		return 0;

	double ratio = x / h1;
	double y = h2 * (1.0 - pow(ratio, m_weldParams.k));
	return y;
}

// 扫描拉伸焊缝 / Sweep weld along path
void CWeldMeshGenerator::SweepWeldAlongPath()
{
	if (m_templateNodes.empty())
		return;

	// 计算扫描路径点数 / Calculate sweep path points
	int numSections = (int)(m_weldParams.LW / m_meshDensity.weld) + 1;
	double stepZ = m_weldParams.LW / (numSections - 1);

	// 存储每层的节点ID映射 / Store node ID mapping for each layer
	std::vector<std::vector<int>> layerNodeIds(numSections);

	// 为每个截面生成节点 / Generate nodes for each section
	for (int i = 0; i < numSections; i++)
	{
		double z = i * stepZ;
		
		for (size_t j = 0; j < m_templateNodes.size(); j++)
		{
			const Node3D& templateNode = m_templateNodes[j];
			
			// 应用幂曲线变形 / Apply power curve deformation
			double deformY = PowerCurveDeformation(m_weldParams.LW, m_weldParams.HW, z);
			
			Node3D newNode;
			newNode.id = m_nextNodeId++;
			newNode.x = templateNode.x;
			newNode.y = templateNode.y + deformY;
			newNode.z = z;
			
			m_nodes.push_back(newNode);
			layerNodeIds[i].push_back(newNode.id);
		}
	}

	// 在相邻截面间生成C3D8R单元 / Generate C3D8R elements between adjacent sections
	for (int i = 0; i < numSections - 1; i++)
	{
		for (size_t j = 0; j < m_templateElements.size(); j++)
		{
			const Element3D& templateElem = m_templateElements[j];
			
			if (templateElem.nodeIds.size() == 4)  // 2D四边形转3D六面体 / 2D quad to 3D hexahedron
			{
				Element3D newElem(m_nextElementId++);
				
				// 底面4个节点 / Bottom 4 nodes
				for (int k = 0; k < 4; k++)
				{
					int templateNodeId = templateElem.nodeIds[k];
					// 在模板节点列表中查找索引 / Find index in template node list
					size_t idx = 0;
					for (idx = 0; idx < m_templateNodes.size(); idx++)
					{
						if (m_templateNodes[idx].id == templateNodeId)
							break;
					}
					if (idx < layerNodeIds[i].size())
						newElem.nodeIds.push_back(layerNodeIds[i][idx]);
				}
				
				// 顶面4个节点 / Top 4 nodes
				for (int k = 0; k < 4; k++)
				{
					int templateNodeId = templateElem.nodeIds[k];
					size_t idx = 0;
					for (idx = 0; idx < m_templateNodes.size(); idx++)
					{
						if (m_templateNodes[idx].id == templateNodeId)
							break;
					}
					if (idx < layerNodeIds[i + 1].size())
						newElem.nodeIds.push_back(layerNodeIds[i + 1][idx]);
				}
				
				if (newElem.nodeIds.size() == 8)
					m_elements.push_back(newElem);
			}
		}
	}
}

// 生成过渡区域网格 / Generate transition region mesh
void CWeldMeshGenerator::GenerateTransitionMesh()
{
	// 在焊缝和平板之间生成过渡网格 / Generate transition mesh between weld and plate
	// 这里实现简化版本 / Simplified version here
	
	double transitionLength = m_geomParams.t3;
	int numLayers = (int)(transitionLength / m_meshDensity.transition) + 1;
	
	// 实际实现需要根据几何形状生成复杂的过渡单元
	// Actual implementation needs to generate complex transition elements based on geometry
}

// 生成平板区域网格 / Generate plate region mesh
void CWeldMeshGenerator::GeneratePlateMesh()
{
	// 生成T型结构的上下平板网格 / Generate mesh for top and bottom plates of T-joint
	// 这里实现简化版本 / Simplified version here
	
	double plateLength = m_geomParams.L1;
	double plateWidth = m_geomParams.W1;
	double plateThickness = m_geomParams.t1;
	
	int numX = (int)(plateLength / m_meshDensity.plate) + 1;
	int numY = (int)(plateWidth / m_meshDensity.plate) + 1;
	int numZ = (int)(plateThickness / m_meshDensity.plate) + 1;
	
	// 实际实现需要生成完整的结构化网格
	// Actual implementation needs to generate complete structured mesh
}

// 导出为INP文件 / Export to INP file
bool CWeldMeshGenerator::ExportToINP(const CString& filename)
{
	if (m_nodes.empty())
	{
		m_lastError = _T("没有可导出的网格数据");
		return false;
	}

	CStringA filenameA(filename);
	std::ofstream file(filenameA.GetString());
	
	if (!file.is_open())
	{
		m_lastError = _T("无法创建输出文件");
		return false;
	}

	// 写入头部 / Write header
	file << "*HEADING\n";
	file << "T-Joint Fillet Weld Structure - Generated by FEMeshGenerator\n";
	file << "*PREPRINT, ECHO=NO, MODEL=NO, HISTORY=NO\n";
	file << "**\n";

	// 写入节点 / Write nodes
	file << "*NODE\n";
	for (const auto& node : m_nodes)
	{
		file << node.id << ", " 
			 << node.x << ", " 
			 << node.y << ", " 
			 << node.z << "\n";
	}

	// 写入单元 / Write elements
	file << "**\n";
	file << "*ELEMENT, TYPE=C3D8R, ELSET=ALL_ELEMENTS\n";
	for (const auto& elem : m_elements)
	{
		file << elem.id;
		for (int nodeId : elem.nodeIds)
		{
			file << ", " << nodeId;
		}
		file << "\n";
	}

	// 写入单元集 / Write element sets
	file << "**\n";
	file << "*ELSET, ELSET=WELD\n";
	file << "**\n";
	file << "*ELSET, ELSET=TRANSITION\n";
	file << "**\n";
	file << "*ELSET, ELSET=PLATE\n";
	file << "**\n";

	// 写入材料属性 / Write material properties
	file << "*MATERIAL, NAME=STEEL\n";
	file << "*ELASTIC\n";
	file << "210000., 0.3\n";
	file << "**\n";
	file << "*END\n";

	file.close();
	return true;
}

// 清空网格 / Clear mesh
void CWeldMeshGenerator::ClearMesh()
{
	m_nodes.clear();
	m_elements.clear();
	m_nextNodeId = 1;
	m_nextElementId = 1;
}
