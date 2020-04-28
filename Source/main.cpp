/*
!@
MIT License
Copyright (c) 2019 Skylicht Technology CO., LTD
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
This file is part of the "Skylicht Engine".
https://github.com/skylicht-lab/skylicht-engine
!#
*/

#include "SDKMesh.h"
#include "OBJWriter.h"

#include <iostream>

std::string getCmdOption(int argc, char* argv[], const std::string& option)
{
	std::string cmd;
	for (int i = 0; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (0 == arg.find(option))
		{
			if (i + 1 < argc)
				cmd = argv[i + 1];
			return cmd;
		}
	}
	return cmd;
}

int main(int argc, char** argv)
{
	std::string input = getCmdOption(argc, argv, "-i");
	std::string output = getCmdOption(argc, argv, "-o");

	if (input.empty() || output.empty())
	{
		std::cout << "Missing command: SDKMeshObjExporter.exe -i=INPUT.sdkmesh -o=OUTPUT.obj\n";
		return 1;
	}

	SDKMesh sdkMesh;
	HRESULT r = sdkMesh.Create(input.c_str());
	if (r == E_FAIL)
	{
		std::cout << "Open" << input.c_str() << L" Error\n";
		return -1;
	}

	OBJWriter writer(&sdkMesh, output.c_str());
	if (writer.CanWrite() == false)
	{
		std::cout << "Can not write: " << output.c_str() << "\n";
		return -1;
	}

	std::cout << "\n# Material infomations:\n";

	UINT numMaterials = sdkMesh.GetNumMaterials();
	for (UINT i = 0; i < numMaterials; ++i)
	{
		SDKMESH_MATERIAL* mat = sdkMesh.GetMaterial(i);
		std::cout << "Material: " << mat->Name << std::endl;
		std::cout << "- DiffuseMapName: " << mat->DiffuseTexture << std::endl;
		std::cout << "- NormalMapName: " << mat->NormalTexture << std::endl;

		writer.WriteMaterial(mat);
	}

	int errorCount = 0;

	std::cout << "\n# Mesh infomations:\n";
	UINT numMeshes = sdkMesh.GetNumMeshes();
	for (UINT meshIdx = 0; meshIdx < numMeshes; ++meshIdx)
	{
		std::cout << "\n Mesh ID: " << meshIdx << " - ";

		// Figure out the index type
		if (sdkMesh.GetIndexType(meshIdx) == IT_32BIT)
			std::cout << "32BIT";
		else
			std::cout << "16BIT";

		UINT numPrims = static_cast<UINT>(sdkMesh.GetNumIndices(meshIdx) / 3);
		UINT numVerts = static_cast<UINT>(sdkMesh.GetNumVertices(meshIdx, 0));

		std::cout << " - Prims: " << numPrims << ", Verts: " << numVerts << std::endl;

		std::cout << " - Vertex buffer elements\n";
		sdkMesh.PrintVBElements(sdkMesh.VBElements(meshIdx, 0));

		SDKMESH_MESH* mesh = sdkMesh.GetMesh(meshIdx);

		// write name
		writer.WriteObject(mesh->Name);

		UINT numSubsets = sdkMesh.GetNumSubsets(meshIdx);

		for (UINT i = 0; i < numSubsets; ++i)
		{
			SDKMESH_SUBSET* subset = sdkMesh.GetSubset(meshIdx, i);

			int materialID = subset->MaterialID;
			SDKMESH_MATERIAL* mat = sdkMesh.GetMaterial(materialID);

			int faceStart = static_cast<DWORD>(subset->IndexStart / 3);
			int faceCount = static_cast<DWORD>(subset->IndexCount / 3);

			int vertexStart = static_cast<DWORD>(subset->VertexStart);
			int vertexCount = static_cast<DWORD>(subset->VertexCount);

			const char *PrimitiveType[] = {
				"PT_TRIANGLE_LIST",
				"PT_TRIANGLE_STRIP",
				"PT_LINE_LIST",
				"PT_LINE_STRIP",
				"PT_POINT_LIST",
				"PT_TRIANGLE_LIST_ADJ",
				"PT_TRIANGLE_STRIP_ADJ",
				"PT_LINE_LIST_ADJ",
				"PT_LINE_STRIP_ADJ",
				"PT_QUAD_PATCH_LIST",
				"PT_TRIANGLE_PATCH_LIST",
			};

			std::cout << "- Subset: " << i << " " << mat->Name << " - " << PrimitiveType[subset->PrimitiveType] << std::endl;
			std::cout << "  + Indices start: " << subset->IndexStart << std::endl;
			std::cout << "  + Indices count: " << subset->IndexCount << std::endl;
			std::cout << "  + Face count: " << faceCount << std::endl;

			if (subset->PrimitiveType == 0)
			{
				if (writer.WriteSubset(meshIdx, mesh, subset, numSubsets > 1) == true)
					std::cout << "  -> Writed!\n";
				else
					std::cout << "  -> Write error!\n";
			}
			else
			{
				std::cout << "  -> Error: OBJ Exporter just support TRIANGLE_LIST!\n";
				errorCount++;
			}
		}
	}

	if (errorCount > 0)
		std::cout << "Error: " << errorCount;
	else
		std::cout << "Finished!\n";

	return 0;
}