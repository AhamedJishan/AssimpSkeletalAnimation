#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


void ParseSingleBone(int index, const aiBone* bone)
{
	printf("\tBone %d: %s num vertices affected by the bone: %d\n", index, bone->mName.C_Str(), bone->mNumWeights);

	for (int i = 0; i < bone->mNumWeights; i++)
	{
		const aiVertexWeight& vw = bone->mWeights[i];
		printf("\t\t%d: vertex id %d, weight %.2f\n", i, vw.mVertexId, vw.mWeight);
	}
	printf("\n");
}

void ParseMeshBones(const aiMesh* mesh)
{
	for (int i = 0; i < mesh->mNumBones; i++)
		ParseSingleBone(i, mesh->mBones[i]);
}

void ParseMeshes(const aiScene* scene)
{
	printf("******************************************************************************\n");
	printf("Parsing %d meshes\n\n", scene->mNumMeshes);

	int totalVertices = 0;
	int totalIndices= 0;
	int totalBones= 0;

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		int numVertices = mesh->mNumVertices;
		int numIndices = mesh->mNumFaces * 3;
		int numBones = mesh->mNumBones;
		printf("Mesh %d. '%s': vertices - %d, indices - %d, bones - %d\n\n", i, mesh->mName.C_Str(), numVertices, numIndices, numBones);
		totalVertices += numVertices;
		totalIndices += numIndices;
		totalBones += numBones;
		printf("\n");

		if (mesh->HasBones())
			ParseMeshBones(mesh);
	}

	printf("\n Total vertices - %d, total indices - %d, total bones - %d", totalVertices, totalIndices, totalBones);
}

void ParseScene(const aiScene* scene)
{
	ParseMeshes(scene);
}

int main(int argc, char* argv[])
{
	const char* filename = "Assets/archer/Hip Hop Dancing.dae";

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_GenNormals | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	if (!scene)
	{
		printf("Error loading file '%s': '%s'", filename, importer.GetErrorString());
		return 1;
	}

	ParseScene(scene);
	std::cin.get();

	return 0;
}