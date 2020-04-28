//--------------------------------------------------------------------------------------
// File: SDKMesh.cpp
//
// The SDK Mesh format (.sdkmesh) is not a recommended file format for games.
// It was designed to meet the specific needs of the SDK samples.  Any real-world
// applications should avoid this file format in favor of a destination format that
// meets the specific needs of the application.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "SDKMesh.h"

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif


//--------------------------------------------------------------------------------------
HRESULT SDKMesh::CreateFromFile(const char* szFileName, bool bCreateAdjacencyIndices)
{
	HRESULT hr = S_OK;

	// Open the file
	FILE* hFile = fopen(szFileName, "rb");
	if (hFile == NULL)
		return E_FAIL;

	// Get the file size
	SIZE_T fileSize;
	fseek(hFile, 0, SEEK_END);
	fileSize = ftell(hFile);
	fseek(hFile, 0, SEEK_SET);

	// Allocate memory
	m_pStaticMeshData = new BYTE[fileSize];
	if (!m_pStaticMeshData)
	{
		return E_FAIL;
	}

	// Read in the file
	if (!fread(m_pStaticMeshData, fileSize, 1, hFile))
		hr = E_FAIL;

	fclose(hFile);

	if (hr == S_OK)
	{
		hr = CreateFromMemory(m_pStaticMeshData,
			fileSize,
			bCreateAdjacencyIndices,
			false);

		if (hr == E_FAIL)
			delete[]m_pStaticMeshData;
	}

	return hr;
}

HRESULT SDKMesh::CreateFromMemory(BYTE* pData,
	UINT DataBytes,
	bool bCreateAdjacencyIndices,
	bool bCopyStatic)
{
	HRESULT hr = E_FAIL;

	// Set outstanding resources to zero
	m_NumOutstandingResources = 0;

	if (bCopyStatic)
	{
		SDKMESH_HEADER* pHeader = (SDKMESH_HEADER*)pData;

		SIZE_T StaticSize = (SIZE_T)(pHeader->HeaderSize + pHeader->NonBufferDataSize);
		m_pHeapData = new BYTE[StaticSize];
		if (!m_pHeapData)
			return hr;

		m_pStaticMeshData = m_pHeapData;

		memcpy(m_pStaticMeshData, pData, StaticSize);
	}
	else
	{
		m_pHeapData = pData;
		m_pStaticMeshData = pData;
	}

	// Pointer fixup
	m_pMeshHeader = (SDKMESH_HEADER*)m_pStaticMeshData;
	m_pVertexBufferArray = (SDKMESH_VERTEX_BUFFER_HEADER*)(m_pStaticMeshData + m_pMeshHeader->VertexStreamHeadersOffset);
	m_pIndexBufferArray = (SDKMESH_INDEX_BUFFER_HEADER*)(m_pStaticMeshData + m_pMeshHeader->IndexStreamHeadersOffset);
	m_pMeshArray = (SDKMESH_MESH*)(m_pStaticMeshData + m_pMeshHeader->MeshDataOffset);
	m_pSubsetArray = (SDKMESH_SUBSET*)(m_pStaticMeshData + m_pMeshHeader->SubsetDataOffset);
	m_pFrameArray = (SDKMESH_FRAME*)(m_pStaticMeshData + m_pMeshHeader->FrameDataOffset);
	m_pMaterialArray = (SDKMESH_MATERIAL*)(m_pStaticMeshData + m_pMeshHeader->MaterialDataOffset);

	// Setup subsets
	for (UINT i = 0; i < m_pMeshHeader->NumMeshes; i++)
	{
		m_pMeshArray[i].pSubsets = (UINT*)(m_pStaticMeshData + m_pMeshArray[i].SubsetOffset);
		m_pMeshArray[i].pFrameInfluences = (UINT*)(m_pStaticMeshData + m_pMeshArray[i].FrameInfluenceOffset);
	}

	// error condition
	if (m_pMeshHeader->Version != SDKMESH_FILE_VERSION)
	{
		hr = E_FAIL;
		goto Error;
	}

	// Setup buffer data pointer
	BYTE* pBufferData = pData + m_pMeshHeader->HeaderSize + m_pMeshHeader->NonBufferDataSize;

	// Get the start of the buffer data
	UINT64 BufferDataStart = m_pMeshHeader->HeaderSize + m_pMeshHeader->NonBufferDataSize;

	// Create VBs
	m_ppVertices = new BYTE*[m_pMeshHeader->NumVertexBuffers];
	for (UINT i = 0; i < m_pMeshHeader->NumVertexBuffers; i++)
	{
		BYTE* pVertices = NULL;
		pVertices = (BYTE*)(pBufferData + (m_pVertexBufferArray[i].DataOffset - BufferDataStart));

		m_ppVertices[i] = pVertices;
	}

	// Create IBs
	m_ppIndices = new BYTE*[m_pMeshHeader->NumIndexBuffers];
	for (UINT i = 0; i < m_pMeshHeader->NumIndexBuffers; i++)
	{
		BYTE* pIndices = NULL;
		pIndices = (BYTE*)(pBufferData + (m_pIndexBufferArray[i].DataOffset - BufferDataStart));

		m_ppIndices[i] = pIndices;
	}

	hr = S_OK;
Error:

	return hr;
}

#define MAX_D3D11_VERTEX_STREAMS D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT


//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
SDKMesh::SDKMesh() : m_NumOutstandingResources(0),
	m_pMeshHeader(NULL),
	m_pStaticMeshData(NULL),
	m_pHeapData(NULL),
	m_pAdjacencyIndexBufferArray(NULL),
	m_pAnimationData(NULL),
	m_pAnimationHeader(NULL),
	m_ppVertices(NULL),
	m_ppIndices(NULL),
	m_pBindPoseFrameMatrices(NULL),
	m_pTransformedFrameMatrices(NULL),
	m_pWorldPoseFrameMatrices(NULL)
{
}


//--------------------------------------------------------------------------------------
SDKMesh::~SDKMesh()
{
	Destroy();
}

//--------------------------------------------------------------------------------------
HRESULT SDKMesh::Create(const char* szFileName, bool bCreateAdjacencyIndices)
{
	return CreateFromFile(szFileName, bCreateAdjacencyIndices);
}

//--------------------------------------------------------------------------------------
HRESULT SDKMesh::Create(BYTE* pData, UINT DataBytes, bool bCreateAdjacencyIndices, bool bCopyStatic)
{
	return CreateFromMemory(pData, DataBytes, bCreateAdjacencyIndices, bCopyStatic);
}

//--------------------------------------------------------------------------------------
void SDKMesh::Destroy()
{
	SAFE_DELETE_ARRAY(m_pAdjacencyIndexBufferArray);

	SAFE_DELETE_ARRAY(m_pHeapData);
	m_pStaticMeshData = NULL;
	SAFE_DELETE_ARRAY(m_pAnimationData);
	SAFE_DELETE_ARRAY(m_pBindPoseFrameMatrices);
	SAFE_DELETE_ARRAY(m_pTransformedFrameMatrices);
	SAFE_DELETE_ARRAY(m_pWorldPoseFrameMatrices);

	SAFE_DELETE_ARRAY(m_ppVertices);
	SAFE_DELETE_ARRAY(m_ppIndices);

	m_pMeshHeader = NULL;
	m_pVertexBufferArray = NULL;
	m_pIndexBufferArray = NULL;
	m_pMeshArray = NULL;
	m_pSubsetArray = NULL;
	m_pFrameArray = NULL;
	m_pMaterialArray = NULL;

	m_pAnimationHeader = NULL;
	m_pAnimationFrameData = NULL;

}


//--------------------------------------------------------------------------------------
/*
D3D11_PRIMITIVE_TOPOLOGY SDKMesh::GetPrimitiveType11( SDKMESH_PRIMITIVE_TYPE PrimType )
{
	D3D11_PRIMITIVE_TOPOLOGY retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	switch( PrimType )
	{
		case PT_TRIANGLE_LIST:
			retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case PT_TRIANGLE_STRIP:
			retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			break;
		case PT_LINE_LIST:
			retType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			break;
		case PT_LINE_STRIP:
			retType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			break;
		case PT_POINT_LIST:
			retType = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		case PT_TRIANGLE_LIST_ADJ:
			retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
			break;
		case PT_TRIANGLE_STRIP_ADJ:
			retType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
			break;
		case PT_LINE_LIST_ADJ:
			retType = D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
			break;
		case PT_LINE_STRIP_ADJ:
			retType = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
			break;
	};

	return retType;
}

//--------------------------------------------------------------------------------------
DXGI_FORMAT SDKMesh::GetIBFormat11( UINT iMesh )
{
	switch( m_pIndexBufferArray[ m_pMeshArray[ iMesh ].IndexBuffer ].IndexType )
	{
		case IT_16BIT:
			return DXGI_FORMAT_R16_UINT;
		case IT_32BIT:
			return DXGI_FORMAT_R32_UINT;
	};
	return DXGI_FORMAT_R16_UINT;
}
*/

//--------------------------------------------------------------------------------------
UINT SDKMesh::GetNumMeshes()
{
	if (!m_pMeshHeader)
		return 0;
	return m_pMeshHeader->NumMeshes;
}

//--------------------------------------------------------------------------------------
UINT SDKMesh::GetNumMaterials()
{
	if (!m_pMeshHeader)
		return 0;
	return m_pMeshHeader->NumMaterials;
}

//--------------------------------------------------------------------------------------
UINT SDKMesh::GetNumVBs()
{
	if (!m_pMeshHeader)
		return 0;
	return m_pMeshHeader->NumVertexBuffers;
}

//--------------------------------------------------------------------------------------
UINT SDKMesh::GetNumIBs()
{
	if (!m_pMeshHeader)
		return 0;
	return m_pMeshHeader->NumIndexBuffers;
}

//--------------------------------------------------------------------------------------
BYTE* SDKMesh::GetRawVerticesAt(UINT iVB)
{
	return m_ppVertices[iVB];
}

//--------------------------------------------------------------------------------------
BYTE* SDKMesh::GetRawIndicesAt(UINT iIB)
{
	return m_ppIndices[iIB];
}

//--------------------------------------------------------------------------------------
SDKMESH_MATERIAL* SDKMesh::GetMaterial(UINT iMaterial)
{
	return &m_pMaterialArray[iMaterial];
}

//--------------------------------------------------------------------------------------
SDKMESH_MESH* SDKMesh::GetMesh(UINT iMesh)
{
	return &m_pMeshArray[iMesh];
}

//--------------------------------------------------------------------------------------
UINT SDKMesh::GetNumSubsets(UINT iMesh)
{
	return m_pMeshArray[iMesh].NumSubsets;
}

//--------------------------------------------------------------------------------------
SDKMESH_SUBSET* SDKMesh::GetSubset(UINT iMesh, UINT iSubset)
{
	return &m_pSubsetArray[m_pMeshArray[iMesh].pSubsets[iSubset]];
}

//--------------------------------------------------------------------------------------
UINT SDKMesh::GetVertexStride(UINT iMesh, UINT iVB)
{
	return (UINT)m_pVertexBufferArray[m_pMeshArray[iMesh].VertexBuffers[iVB]].StrideBytes;
}

//--------------------------------------------------------------------------------------
UINT64 SDKMesh::GetNumVertices(UINT iMesh, UINT iVB)
{
	return m_pVertexBufferArray[m_pMeshArray[iMesh].VertexBuffers[iVB]].NumVertices;
}

//--------------------------------------------------------------------------------------
UINT64 SDKMesh::GetNumIndices(UINT iMesh)
{
	return m_pIndexBufferArray[m_pMeshArray[iMesh].IndexBuffer].NumIndices;
}

SDKMESH_INDEX_TYPE SDKMesh::GetIndexType(UINT iMesh)
{
	return (SDKMESH_INDEX_TYPE)m_pIndexBufferArray[m_pMeshArray[iMesh].IndexBuffer].IndexType;
}

const D3DVERTEXELEMENT9* SDKMesh::VBElements(UINT iMesh, UINT iVB)
{
	return m_pVertexBufferArray[m_pMeshArray[iMesh].VertexBuffers[iVB]].Decl;
}

void SDKMesh::PrintVBElements(const D3DVERTEXELEMENT9* declaration)
{
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

	// Figure out the number of elements
	UINT numInputElements = 0;
	while (declaration[numInputElements].Stream != 0xFF)
	{
		const D3DVERTEXELEMENT9& element9 = declaration[numInputElements];
		std::cout << "  + " << nameMap[element9.Usage] << " - " << formatMap[element9.Type] << " - " << element9.Offset << std::endl;
		numInputElements++;
	}
}