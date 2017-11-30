#pragma once
#pragma warning(disable : 4996)
#include <vector>
#include <fstream>
//#include "pch.h"
#include "..\Common\StepTimer.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"

using namespace std;
using namespace DirectX;
using namespace Windows::Foundation;

struct ObjectVertices
{
	XMFLOAT3 Position;
	XMFLOAT2 UV;
	XMFLOAT3 Normals;
};

struct ObjectData
{
	vector<ObjectVertices> MeshVerts;
	vector<unsigned int> MeshIndecies;
};

static bool LoadOBJFile(const char* filepath, ObjectData Mesh)
{
	vector<unsigned int> TempVertIndies;
	vector<unsigned int> TempUVIndies;
	vector<unsigned int> TempNormalIndies;

	vector<XMFLOAT3> TempVerts;
	vector<XMFLOAT2> TempUVs;
	vector<XMFLOAT3> TempNormals;

	FILE* objFile = fopen(filepath, "r");
	if (objFile == NULL)
	{
		return false;
	}

	while (true)
	{
		char FileHeader[128];

		int fword = fscanf(objFile, "%s", FileHeader);

		if (fword == EOF)
		{
			break;
		}

		if (strcmp(FileHeader, "v") == 0)
		{
			XMFLOAT3 tempVertex;

			fscanf(objFile, "%f %f %f\n", &tempVertex.x, &tempVertex.y, &tempVertex.z);

			TempVerts.push_back(tempVertex);
		}

		else if (strcmp(FileHeader, "vt") == 0)
		{
			XMFLOAT2 tempUV;

			fscanf(objFile, "%f %f\n", &tempUV.x, &tempUV.y);

			TempUVs.push_back(tempUV);
		}

		else if (strcmp(FileHeader, "vn") == 0)
		{
			XMFLOAT3 tempNormal;

			fscanf(objFile, "%f %f %f\n", &tempNormal.x, &tempNormal.y, &tempNormal.z);

			TempNormals.push_back(tempNormal);
		}

		else if (strcmp(FileHeader, "f") == 0)
		{
			unsigned int vertIndex[3];
			unsigned int uvIndex[3];
			unsigned int normalIndex[3];

			int Indexes = fscanf(objFile, "%d%d%d %d%d%d %d%d%d\n", &vertIndex[0], &uvIndex[0],
				&normalIndex[0], &vertIndex[1], &uvIndex[1], &normalIndex[1],
				&vertIndex[2], &uvIndex[2], &normalIndex[2]);

			if (Indexes != 9)
			{
				return false;
			}

			TempVertIndies.push_back(vertIndex[0]);
			TempVertIndies.push_back(vertIndex[1]);
			TempVertIndies.push_back(vertIndex[2]);

			TempUVIndies.push_back(uvIndex[0]);
			TempUVIndies.push_back(uvIndex[1]);
			TempUVIndies.push_back(uvIndex[2]);

			TempNormalIndies.push_back(normalIndex[0]);
			TempNormalIndies.push_back(normalIndex[1]);
			TempNormalIndies.push_back(normalIndex[2]);
		}
	}

	for (unsigned int i = 0; i < TempVertIndies.size(); i++)
	{
		unsigned int vertexIn = TempVertIndies[i];
		unsigned int uvIndex = TempUVIndies[i];
		unsigned int normalIndex = TempNormalIndies[i];

		ObjectVertices temp;

		XMFLOAT3 vert = TempVerts[vertexIn - 1];
		XMFLOAT2 uv = TempUVs[uvIndex - 1];
		XMFLOAT3 normal = TempNormals[normalIndex - 1];

		temp.Position = vert;
		temp.UV = uv;
		temp.Normals = normal;

		Mesh.MeshVerts.push_back(temp);
		Mesh.MeshIndecies.push_back(i + 1);
	}

	return true;
}
