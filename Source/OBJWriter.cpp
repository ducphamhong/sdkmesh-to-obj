#include "OBJWriter.h"
#include "CStringImp.h"

#include <string>

using namespace Skylicht;

OBJWriter::OBJWriter(SDKMesh *mesh, const char *output)
	:m_sdkMesh(mesh)
{
	m_file = fopen(output, "wt");

	char material[MAX_PATH];
	strcpy(material, output);
	CStringImp::replaceExt(material, ".mtl");

	fprintf(m_file, "# exported by SDKMesh Expoter\n");
	fprintf(m_file, "mtllib %s\n", material);

	m_mat = fopen(material, "wt");
	fprintf(m_mat, "# exported by SDKMesh Expoter\n");

	m_group = 0;
	m_numVertex = 1;
}

bool OBJWriter::CanWrite()
{
	if (m_file == NULL)
		return false;

	return true;
}

OBJWriter::~OBJWriter()
{
	if (m_file != NULL)
	{
		fclose(m_file);
		m_file = NULL;
	}

	if (m_mat != NULL)
	{
		fclose(m_mat);
		m_mat = NULL;
	}
}

bool OBJWriter::WriteMaterial(SDKMESH_MATERIAL *material)
{
	fprintf(m_mat, "newmtl mat %s\n", material->Name);
	fprintf(m_mat, "Kd %f %f %f %f\n", 
		material->Diffuse.x,
		material->Diffuse.y,
		material->Diffuse.z,
		material->Diffuse.w);

	fprintf(m_mat, "Ka %f %f %f %f\n",
		material->Ambient.x,
		material->Ambient.y,
		material->Ambient.z,
		material->Ambient.w);

	fprintf(m_mat, "Ks %f %f %f %f\n",
		material->Specular.x,
		material->Specular.y,
		material->Specular.z,
		material->Specular.w);

	fprintf(m_mat, "Ke %f %f %f %f\n",
		material->Emissive.x,
		material->Emissive.y,
		material->Emissive.z,
		material->Emissive.w);

	fprintf(m_mat, "illum %f\n", material->Power);

	if (strlen(material->DiffuseTexture))
		fprintf(m_mat, "map_Kd %s\n", material->DiffuseTexture);

	if (strlen(material->NormalTexture))
		fprintf(m_mat, "map_bump %s\n", material->NormalTexture);

	if (strlen(material->SpecularTexture))
		fprintf(m_mat, "map_Ks %s\n", material->SpecularTexture);

	return true;
}

void OBJWriter::WriteObject(const char *name)
{
	fprintf(m_file, "o %s\n", name);
}

bool OBJWriter::WriteSubset(UINT meshID, SDKMESH_MESH* mesh, SDKMESH_SUBSET *subset, bool writeGroup)
{	
	BYTE* vertexBufferData = m_sdkMesh->GetRawVerticesAt(mesh->VertexBuffers[0]);
	BYTE* indexBufferData = m_sdkMesh->GetRawIndicesAt(mesh->IndexBuffer);
	UINT vertexStride = m_sdkMesh->GetVertexStride(meshID, 0);

	std::map<BYTE, const char *> nameMap;
	nameMap[D3DDECLUSAGE_POSITION] = "POSITION";
	nameMap[D3DDECLUSAGE_BLENDWEIGHT] = "BLENDWEIGHT";
	nameMap[D3DDECLUSAGE_BLENDINDICES] = "BLENDINDICES";
	nameMap[D3DDECLUSAGE_NORMAL] = "NORMAL";
	nameMap[D3DDECLUSAGE_TEXCOORD] = "TEXCOORD";
	nameMap[D3DDECLUSAGE_TANGENT] = "TANGENT";
	nameMap[D3DDECLUSAGE_BINORMAL] = "BINORMAL";
	nameMap[D3DDECLUSAGE_COLOR] = "COLOR";

	std::map<BYTE, const char*> formatMap;
	formatMap[D3DDECLTYPE_FLOAT1] = "DXGI_FORMAT_R32_FLOAT";
	formatMap[D3DDECLTYPE_FLOAT2] = "DXGI_FORMAT_R32G32_FLOAT";
	formatMap[D3DDECLTYPE_FLOAT3] = "DXGI_FORMAT_R32G32B32_FLOAT";
	formatMap[D3DDECLTYPE_FLOAT4] = "DXGI_FORMAT_R32G32B32A32_FLOAT";
	formatMap[D3DDECLTYPE_D3DCOLOR] = "DXGI_FORMAT_R8G8B8A8_UNORM";
	formatMap[D3DDECLTYPE_UBYTE4] = "DXGI_FORMAT_R8G8B8A8_UINT";
	formatMap[D3DDECLTYPE_SHORT2] = "DXGI_FORMAT_R16G16_SINT";
	formatMap[D3DDECLTYPE_SHORT4] = "DXGI_FORMAT_R16G16B16A16_SINT";
	formatMap[D3DDECLTYPE_UBYTE4N] = "DXGI_FORMAT_R8G8B8A8_UNORM";
	formatMap[D3DDECLTYPE_SHORT2N] = "DXGI_FORMAT_R16G16_SNORM";
	formatMap[D3DDECLTYPE_SHORT4N] = "DXGI_FORMAT_R16G16B16A16_SNORM";
	formatMap[D3DDECLTYPE_USHORT2N] = "DXGI_FORMAT_R16G16_UNORM";
	formatMap[D3DDECLTYPE_USHORT4N] = "DXGI_FORMAT_R16G16B16A16_UNORM";
	formatMap[D3DDECLTYPE_UDEC3] = "DXGI_FORMAT_R10G10B10A2_UINT";
	formatMap[D3DDECLTYPE_DEC3N] = "DXGI_FORMAT_R10G10B10A2_UNORM";
	formatMap[D3DDECLTYPE_FLOAT16_2] = "DXGI_FORMAT_R16G16_FLOAT";
	formatMap[D3DDECLTYPE_FLOAT16_4] = "DXGI_FORMAT_R16G16B16A16_FLOAT";

	bool success = true;

	std::vector<unsigned long> vertices;
	std::map<unsigned long, unsigned long> map;

	if (m_sdkMesh->GetIndexType(meshID) == IT_32BIT)
	{
		// 32bit
		DWORD *indices = (DWORD*)indexBufferData;
		for (UINT64 i = subset->IndexStart; i < subset->IndexStart + subset->IndexCount; i += 3)
		{
			int i0 = indices[i];
			int i1 = indices[i + 1];
			int i2 = indices[i + 2];

			if (map.find(i0) == map.end())
			{				
				map[i0] = vertices.size();
				vertices.push_back(i0);
			}

			if (map.find(i1) == map.end())
			{				
				map[i1] = vertices.size();
				vertices.push_back(i1);
			}

			if (map.find(i2) == map.end())
			{				
				map[i2] = vertices.size();
				vertices.push_back(i2);
			}
		}
	}
	else
	{
		// 16bit
		WORD *indices = (WORD*)indexBufferData;
		for (UINT64 i = subset->IndexStart; i < subset->IndexStart + subset->IndexCount; i += 3)
		{
			int i0 = indices[i];
			int i1 = indices[i + 1];
			int i2 = indices[i + 2];

			if (map.find(i0) == map.end())
			{				
				map[i0] = vertices.size();
				vertices.push_back(i0);
			}

			if (map.find(i1) == map.end())
			{				
				map[i1] = vertices.size();
				vertices.push_back(i1);
			}

			if (map.find(i2) == map.end())
			{				
				map[i2] = vertices.size();
				vertices.push_back(i2);
			}
		}
	}

	if (writeGroup)
		fprintf(m_file, "g grp %d \n", m_group++);

	const D3DVERTEXELEMENT9* declaration = m_sdkMesh->VBElements(meshID, 0);
	UINT numInputElements = 0;
	while (declaration[numInputElements].Stream != 0xFF)
	{
		const D3DVERTEXELEMENT9& element9 = declaration[numInputElements];

		std::string name = nameMap[element9.Usage];
		std::string format = formatMap[element9.Type];

		int offset = element9.Offset;

		if (name == "POSITION")
		{
			if (format == "DXGI_FORMAT_R32G32B32_FLOAT")
			{				
				for (unsigned int i = 0; i < vertices.size(); i++)
				{
					BYTE *vtxData = vertexBufferData + vertices[i] * vertexStride + offset;
					float *f = (float*)vtxData;
					fprintf(m_file, "v %f %f %f\n", f[0], f[1], f[2]);
				}
			}
			else
				std::cout << "  -> Error: " << name << "Can not support format: " << format << std::endl;
		}
		else if (name == "NORMAL")
		{
			if (format == "DXGI_FORMAT_R32G32B32_FLOAT")
			{
				for (unsigned int i = 0; i < vertices.size(); i++)
				{
					BYTE *vtxData = vertexBufferData + vertices[i] * vertexStride + offset;
					float *f = (float*)vtxData;
					fprintf(m_file, "vn %f %f %f\n", f[0], f[1], f[2]);
				}
			}
			else
				std::cout << "  -> Error: " << name << "Can not support format: " << format << std::endl;
		}
		else if (name == "TEXCOORD")
		{
			if (format == "DXGI_FORMAT_R32G32_FLOAT")
			{
				for (unsigned int i = 0; i < vertices.size(); i++)
				{
					BYTE *vtxData = vertexBufferData + vertices[i] * vertexStride + offset;
					float *f = (float*)vtxData;
					fprintf(m_file, "vt %f %f\n", f[0], 1.0f - f[1]);
				}
			}
			else
				std::cout << "  -> Error: " << name << "Can not support format: " << format << std::endl;
		}
		else
		{
			std::cout << "  -> Warning: Missing: " << name << std::endl;
		}

		numInputElements++;
	}

	SDKMESH_MATERIAL* mat = m_sdkMesh->GetMaterial(subset->MaterialID);

	fprintf(m_file, "usemtl mat %s\n", mat->Name);
	fprintf(m_file, "s off\n");
	
	if (m_sdkMesh->GetIndexType(meshID) == IT_32BIT)
	{
		// 32bit
		DWORD *indices = (DWORD*)indexBufferData;
		for (UINT64 i = subset->IndexStart; i < subset->IndexStart + subset->IndexCount; i += 3)
		{
			int i0 = indices[i];
			int i1 = indices[i + 1];
			int i2 = indices[i + 2];

			int m0 = map[i0] + m_numVertex;
			int m1 = map[i1] + m_numVertex;
			int m2 = map[i2] + m_numVertex;

			fprintf(m_file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", 
				m0, m0, m0,
				m1, m1, m1,
				m2, m2, m2);
		}
	}
	else
	{
		// 16bit
		WORD *indices = (WORD*)indexBufferData;
		for (UINT64 i = subset->IndexStart; i < subset->IndexStart + subset->IndexCount; i += 3)
		{
			int i0 = indices[i];
			int i1 = indices[i + 1];
			int i2 = indices[i + 2];

			int m0 = map[i0] + m_numVertex;
			int m1 = map[i1] + m_numVertex;
			int m2 = map[i2] + m_numVertex;

			fprintf(m_file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
				m0, m0, m0,
				m1, m1, m1,
				m2, m2, m2);
		}
	}

	m_numVertex += vertices.size();

	return success;
}