//--------------------------------------------------------------------------------------
// File: SDKMesh.h
//
// Disclaimer:
//   The SDK Mesh format (.sdkmesh) is not a recommended file format for shipping titles.
//   It was designed to meet the specific needs of the SDK samples.  Any real-world
//   applications should avoid this file format in favor of a destination format that
//   meets the specific needs of the application.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#pragma once

#ifndef _SDKMESH_
#define _SDKMESH_

#include <vector>
#include <map>

#include <iostream>

//--------------------------------------------------------------------------------------
// Hard Defines for the various structures
//--------------------------------------------------------------------------------------

typedef float FLOAT;
typedef short WORD;
typedef unsigned int DWORD;
typedef unsigned char BYTE;
typedef bool BOOL;
typedef unsigned int UINT;
typedef unsigned __int64 UINT64;
typedef _Return_type_success_(return >= 0) long HRESULT;
typedef size_t SIZE_T;

#define S_OK 0
#define E_FAIL 1

#define MAX_PATH 260

#define SDKMESH_FILE_VERSION 101
#define MAX_VERTEX_ELEMENTS 32
#define MAX_VERTEX_STREAMS 16
#define MAX_FRAME_NAME 100
#define MAX_MESH_NAME 100
#define MAX_SUBSET_NAME 100
#define MAX_MATERIAL_NAME 100
#define MAX_TEXTURE_NAME MAX_PATH
#define MAX_MATERIAL_PATH MAX_PATH
#define INVALID_FRAME ((UINT)-1)
#define INVALID_MESH ((UINT)-1)
#define INVALID_MATERIAL ((UINT)-1)
#define INVALID_SUBSET ((UINT)-1)
#define INVALID_ANIMATION_DATA ((UINT)-1)
#define INVALID_SAMPLER_SLOT ((UINT)-1)
#define ERROR_RESOURCE_VALUE 1

struct D3DXVECTOR3
{
	FLOAT x;
	FLOAT y;
	FLOAT z;
};

struct D3DXVECTOR4
{
	FLOAT x;
	FLOAT y;
	FLOAT z;
	FLOAT w;
};

struct D3DXMATRIX
{
	float m[4][4];
};

typedef enum _D3DDECLUSAGE
{
	D3DDECLUSAGE_POSITION = 0,
	D3DDECLUSAGE_BLENDWEIGHT,   // 1
	D3DDECLUSAGE_BLENDINDICES,  // 2
	D3DDECLUSAGE_NORMAL,        // 3
	D3DDECLUSAGE_PSIZE,         // 4
	D3DDECLUSAGE_TEXCOORD,      // 5
	D3DDECLUSAGE_TANGENT,       // 6
	D3DDECLUSAGE_BINORMAL,      // 7
	D3DDECLUSAGE_TESSFACTOR,    // 8
	D3DDECLUSAGE_POSITIONT,     // 9
	D3DDECLUSAGE_COLOR,         // 10
	D3DDECLUSAGE_FOG,           // 11
	D3DDECLUSAGE_DEPTH,         // 12
	D3DDECLUSAGE_SAMPLE,        // 13
} D3DDECLUSAGE;

typedef enum _D3DDECLTYPE
{
	D3DDECLTYPE_FLOAT1 = 0,  // 1D float expanded to (value, 0., 0., 1.)
	D3DDECLTYPE_FLOAT2 = 1,  // 2D float expanded to (value, value, 0., 1.)
	D3DDECLTYPE_FLOAT3 = 2,  // 3D float expanded to (value, value, value, 1.)
	D3DDECLTYPE_FLOAT4 = 3,  // 4D float
	D3DDECLTYPE_D3DCOLOR = 4,  // 4D packed unsigned bytes mapped to 0. to 1. range
								// Input is in D3DCOLOR format (ARGB) expanded to (R, G, B, A)
								D3DDECLTYPE_UBYTE4 = 5,  // 4D unsigned byte
								D3DDECLTYPE_SHORT2 = 6,  // 2D signed short expanded to (value, value, 0., 1.)
								D3DDECLTYPE_SHORT4 = 7,  // 4D signed short
														 // The following types are valid only with vertex shaders >= 2.0

														 D3DDECLTYPE_UBYTE4N = 8,  // Each of 4 bytes is normalized by dividing to 255.0
														 D3DDECLTYPE_SHORT2N = 9,  // 2D signed short normalized (v[0]/32767.0,v[1]/32767.0,0,1)
														 D3DDECLTYPE_SHORT4N = 10,  // 4D signed short normalized (v[0]/32767.0,v[1]/32767.0,v[2]/32767.0,v[3]/32767.0)
														 D3DDECLTYPE_USHORT2N = 11,  // 2D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,0,1)
														 D3DDECLTYPE_USHORT4N = 12,  // 4D unsigned short normalized (v[0]/65535.0,v[1]/65535.0,v[2]/65535.0,v[3]/65535.0)
														 D3DDECLTYPE_UDEC3 = 13,  // 3D unsigned 10 10 10 format expanded to (value, value, value, 1)
														 D3DDECLTYPE_DEC3N = 14,  // 3D signed 10 10 10 format normalized and expanded to (v[0]/511.0, v[1]/511.0, v[2]/511.0, 1)
														 D3DDECLTYPE_FLOAT16_2 = 15,  // Two 16-bit floating point values, expanded to (value, value, 0, 1)
														 D3DDECLTYPE_FLOAT16_4 = 16,  // Four 16-bit floating point values
														 D3DDECLTYPE_UNUSED = 17,  // When the type field in a decl is unused.
} D3DDECLTYPE;

typedef struct D3DVERTEXELEMENT9 {
	WORD Stream;
	WORD Offset;
	BYTE Type;
	BYTE Method;
	BYTE Usage;
	BYTE UsageIndex;
} D3DVERTEXELEMENT9, *LPD3DVERTEXELEMENT9;

template<typename T> BOOL IsErrorResource(T data)
{
	if ((T)ERROR_RESOURCE_VALUE == data)
		return TRUE;
	return FALSE;
}

//--------------------------------------------------------------------------------------
// Enumerated Types.  These will have mirrors in both D3D9 and D3D11
//--------------------------------------------------------------------------------------
enum SDKMESH_PRIMITIVE_TYPE
{
	PT_TRIANGLE_LIST = 0,
	PT_TRIANGLE_STRIP,
	PT_LINE_LIST,
	PT_LINE_STRIP,
	PT_POINT_LIST,
	PT_TRIANGLE_LIST_ADJ,
	PT_TRIANGLE_STRIP_ADJ,
	PT_LINE_LIST_ADJ,
	PT_LINE_STRIP_ADJ,
	PT_QUAD_PATCH_LIST,
	PT_TRIANGLE_PATCH_LIST,
};

enum SDKMESH_INDEX_TYPE
{
	IT_16BIT = 0,
	IT_32BIT,
};

enum FRAME_TRANSFORM_TYPE
{
	FTT_RELATIVE = 0,
	FTT_ABSOLUTE,		//This is not currently used but is here to support absolute transformations in the future
};

//--------------------------------------------------------------------------------------
// Structures.  Unions with pointers are forced to 64bit.
//--------------------------------------------------------------------------------------
struct SDKMESH_HEADER
{
	//Basic Info and sizes
	UINT Version;
	BYTE IsBigEndian;
	UINT64 HeaderSize;
	UINT64 NonBufferDataSize;
	UINT64 BufferDataSize;

	//Stats
	UINT NumVertexBuffers;
	UINT NumIndexBuffers;
	UINT NumMeshes;
	UINT NumTotalSubsets;
	UINT NumFrames;
	UINT NumMaterials;

	//Offsets to Data
	UINT64 VertexStreamHeadersOffset;
	UINT64 IndexStreamHeadersOffset;
	UINT64 MeshDataOffset;
	UINT64 SubsetDataOffset;
	UINT64 FrameDataOffset;
	UINT64 MaterialDataOffset;
};

struct SDKMESH_VERTEX_BUFFER_HEADER
{
	UINT64 NumVertices;
	UINT64 SizeBytes;
	UINT64 StrideBytes;
	D3DVERTEXELEMENT9 Decl[MAX_VERTEX_ELEMENTS];
	union
	{
		UINT64 DataOffset;				//(This also forces the union to 64bits)
	};
};

struct SDKMESH_INDEX_BUFFER_HEADER
{
	UINT64 NumIndices;
	UINT64 SizeBytes;
	UINT IndexType;
	union
	{
		UINT64 DataOffset;				//(This also forces the union to 64bits)
	};
};

struct SDKMESH_MESH
{
	char Name[MAX_MESH_NAME];
	BYTE NumVertexBuffers;
	UINT VertexBuffers[MAX_VERTEX_STREAMS];
	UINT IndexBuffer;
	UINT NumSubsets;
	UINT NumFrameInfluences; //aka bones

	D3DXVECTOR3 BoundingBoxCenter;
	D3DXVECTOR3 BoundingBoxExtents;

	union
	{
		UINT64 SubsetOffset;	//Offset to list of subsets (This also forces the union to 64bits)
		UINT* pSubsets;	    //Pointer to list of subsets
	};
	union
	{
		UINT64 FrameInfluenceOffset;  //Offset to list of frame influences (This also forces the union to 64bits)
		UINT* pFrameInfluences;      //Pointer to list of frame influences
	};
};

struct SDKMESH_SUBSET
{
	char Name[MAX_SUBSET_NAME];
	UINT MaterialID;
	UINT PrimitiveType;
	UINT64 IndexStart;
	UINT64 IndexCount;
	UINT64 VertexStart;
	UINT64 VertexCount;
};

struct SDKMESH_FRAME
{
	char Name[MAX_FRAME_NAME];
	UINT Mesh;
	UINT ParentFrame;
	UINT ChildFrame;
	UINT SiblingFrame;
	D3DXMATRIX Matrix;
	UINT AnimationDataIndex;		//Used to index which set of keyframes transforms this frame
};

struct SDKMESH_MATERIAL
{
	char    Name[MAX_MATERIAL_NAME];

	// Use MaterialInstancePath
	char    MaterialInstancePath[MAX_MATERIAL_PATH];

	// Or fall back to d3d8-type materials
	char    DiffuseTexture[MAX_TEXTURE_NAME];
	char    NormalTexture[MAX_TEXTURE_NAME];
	char    SpecularTexture[MAX_TEXTURE_NAME];

	D3DXVECTOR4 Diffuse;
	D3DXVECTOR4 Ambient;
	D3DXVECTOR4 Specular;
	D3DXVECTOR4 Emissive;
	FLOAT Power;

	union
	{
		UINT64 Force64_1;			//Force the union to 64bits
	};
	union
	{
		UINT64 Force64_2;			//Force the union to 64bits
	};
	union
	{
		UINT64 Force64_3;			//Force the union to 64bits
	};

	union
	{
		UINT64 Force64_4;			//Force the union to 64bits
	};
	union
	{
		UINT64 Force64_5;		    //Force the union to 64bits
	};
	union
	{
		UINT64 Force64_6;			//Force the union to 64bits
	};

};

struct SDKANIMATION_FILE_HEADER
{
	UINT Version;
	BYTE IsBigEndian;
	UINT FrameTransformType;
	UINT NumFrames;
	UINT NumAnimationKeys;
	UINT AnimationFPS;
	UINT64 AnimationDataSize;
	UINT64 AnimationDataOffset;
};

struct SDKANIMATION_DATA
{
	D3DXVECTOR3 Translation;
	D3DXVECTOR4 Orientation;
	D3DXVECTOR3 Scaling;
};

struct SDKANIMATION_FRAME_DATA
{
	char FrameName[MAX_FRAME_NAME];
	union
	{
		UINT64 DataOffset;
		SDKANIMATION_DATA* pAnimationData;
	};
};

#ifndef _CONVERTER_APP_

//--------------------------------------------------------------------------------------
// CDXUTSDKMesh class.  This class reads the sdkmesh file format for use by the samples
//--------------------------------------------------------------------------------------
class SDKMesh
{
protected:
	//These are the pointers to the two chunks of data loaded in from the mesh file
	BYTE* m_pStaticMeshData;
	BYTE* m_pHeapData;
	BYTE* m_pAnimationData;
	BYTE** m_ppVertices;
	BYTE** m_ppIndices;

	WORD m_NumOutstandingResources;

	//General mesh info
	SDKMESH_HEADER* m_pMeshHeader;
	SDKMESH_VERTEX_BUFFER_HEADER* m_pVertexBufferArray;
	SDKMESH_INDEX_BUFFER_HEADER* m_pIndexBufferArray;
	SDKMESH_MESH* m_pMeshArray;
	SDKMESH_SUBSET* m_pSubsetArray;
	SDKMESH_FRAME* m_pFrameArray;
	SDKMESH_MATERIAL* m_pMaterialArray;

	// Adjacency information (not part of the m_pStaticMeshData, so it must be created and destroyed separately )
	SDKMESH_INDEX_BUFFER_HEADER* m_pAdjacencyIndexBufferArray;

	//Animation (TODO: Add ability to load/track multiple animation sets)
	SDKANIMATION_FILE_HEADER* m_pAnimationHeader;
	SDKANIMATION_FRAME_DATA* m_pAnimationFrameData;
	D3DXMATRIX* m_pBindPoseFrameMatrices;
	D3DXMATRIX* m_pTransformedFrameMatrices;
	D3DXMATRIX* m_pWorldPoseFrameMatrices;

protected:
	virtual HRESULT CreateFromFile(const char* szFileName, bool bCreateAdjacencyIndices);

	virtual HRESULT CreateFromMemory(BYTE* pData,
		UINT DataBytes,
		bool bCreateAdjacencyIndices,
		bool bCopyStatic);
public:
	SDKMesh();
	virtual ~SDKMesh();

	virtual HRESULT Create(const char* szFileName, bool bCreateAdjacencyIndices = false);
	virtual HRESULT Create(BYTE* pData, UINT DataBytes, bool bCreateAdjacencyIndices = false, bool bCopyStatic = false);
	virtual void Destroy();


	// Helpers (D3D11 specific)
	// static D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveType11(SDKMESH_PRIMITIVE_TYPE PrimType);
	// DXGI_FORMAT                     GetIBFormat11(UINT iMesh);
	SDKMESH_INDEX_TYPE              GetIndexType(UINT iMesh);

	//Helpers (general)
	UINT                            GetNumMeshes();
	UINT                            GetNumMaterials();
	UINT                            GetNumVBs();
	UINT                            GetNumIBs();

	BYTE* GetRawVerticesAt(UINT iVB);
	BYTE* GetRawIndicesAt(UINT iIB);
	SDKMESH_MATERIAL* GetMaterial(UINT iMaterial);
	SDKMESH_MESH* GetMesh(UINT iMesh);
	UINT                            GetNumSubsets(UINT iMesh);
	SDKMESH_SUBSET* GetSubset(UINT iMesh, UINT iSubset);
	UINT                            GetVertexStride(UINT iMesh, UINT iVB);
	UINT                            GetNumFrames();
	SDKMESH_FRAME*                  GetFrame(UINT iFrame);
	SDKMESH_FRAME*                  FindFrame(char* pszName);
	UINT64                          GetNumVertices(UINT iMesh, UINT iVB);
	UINT64                          GetNumIndices(UINT iMesh);
	const D3DVERTEXELEMENT9*        VBElements(UINT iMesh, UINT iV);
	void							PrintVBElements(const D3DVERTEXELEMENT9* declaration);
};


#endif

#endif

