#pragma once

#include "SDKMesh.h"

class OBJWriter
{
protected:
	SDKMesh *m_sdkMesh;
	
	FILE *m_file;
	FILE *m_mat;

	int m_group;
	int m_numVertex;
public:
	OBJWriter(SDKMesh *mesh, const char *output);

	virtual ~OBJWriter();

	bool CanWrite();

	void WriteObject(const char *name);

	bool WriteSubset(UINT meshID, SDKMESH_MESH* mesh, SDKMESH_SUBSET *subset, bool writeGroup);

	bool WriteMaterial(SDKMESH_MATERIAL *material);
};