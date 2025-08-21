#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <assert.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Mesh.h"

#define MAX_NUM_BONES_PER_VERTEX 4
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

struct VertexBoneData
{
	unsigned int BoneIds[MAX_NUM_BONES_PER_VERTEX] = { 0 };
	float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };

	void AddBoneData(int boneId, float weight)
	{
		for (unsigned int i = 0; i < MAX_NUM_BONES_PER_VERTEX; i++)
		{
			if (Weights[i] == 0.0f)
			{
				BoneIds[i] = boneId;
				Weights[i] = weight;
				printf("Bone %d weight %f index %i\n", boneId, weight, i);
				return;
			}
		}
		assert(0);
	}
};

// Packs the vertex to Bone data of all meshes together
std::vector<VertexBoneData> vertexToBones;
// Vector of Base indexes for each meshes in vertexToBones
std::vector<int> meshBaseVertex;
std::map<std::string, unsigned int> boneNameToIndexMap;

int GetBoneId(const aiBone* bone)
{
	int boneId = 0;
	std::string boneName(bone->mName.C_Str());

	if (boneNameToIndexMap.find(boneName) == boneNameToIndexMap.end())
	{
		boneId = boneNameToIndexMap.size();
		boneNameToIndexMap[boneName] = boneId;
	}
	else
		boneId = boneNameToIndexMap[boneName];

	return boneId;
}

void ParseSingleBone(int meshIndex, const aiBone* bone)
{
	printf("Bone '%s': num vertices affected by the bone = %d, ", bone->mName.C_Str(), bone->mNumWeights);

	int boneId = GetBoneId(bone);
	printf("bone id = %d\n", boneId);

	for (int i = 0; i < bone->mNumWeights; i++)
	{
		const aiVertexWeight& vw = bone->mWeights[i];

		unsigned int globalVertexId = meshBaseVertex[meshIndex] + vw.mVertexId;
		printf("\tvertex id %d ", globalVertexId);

		vertexToBones[globalVertexId].AddBoneData(boneId, vw.mWeight);
	}
	printf("\n");
}

void ParseMeshBones(int meshIndex, const aiMesh* mesh)
{
	for (int i = 0; i < mesh->mNumBones; i++)
		ParseSingleBone(meshIndex, mesh->mBones[i]);
}

void ParseMeshes(const aiScene* scene)
{
	printf("******************************************************************************\n");
	printf("Parsing %d meshes\n\n", scene->mNumMeshes);

	int totalVertices = 0;
	int totalIndices= 0;
	int totalBones= 0;

	meshBaseVertex.resize(scene->mNumMeshes);

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		int numVertices = mesh->mNumVertices;
		int numIndices = mesh->mNumFaces * 3;
		int numBones = mesh->mNumBones;
		meshBaseVertex[i] = totalVertices;
		printf("Mesh %d. '%s': vertices - %d, indices - %d, bones - %d\n\n", i, mesh->mName.C_Str(), numVertices, numIndices, numBones);
		totalVertices += numVertices;
		totalIndices += numIndices;
		totalBones += numBones;
		printf("\n");

		vertexToBones.resize(totalVertices);

		if (mesh->HasBones())
			ParseMeshBones(i, mesh);
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

	GLFWwindow* window;

	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World", glfwGetPrimaryMonitor(), NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD!\n" << glfwGetVersionString();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader shader("Assets/basic.vert", "Assets/basic.frag");
	Mesh* mesh = new Mesh();
	mesh->LoadMesh(filename);

	glm::mat4 projection = glm::perspective(glm::radians(80.0f), SCREEN_WIDTH / (float)(SCREEN_HEIGHT), 0.1f, 1000.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 0, 15), glm::vec3(0), glm::vec3(0, 1.0f, 0));
	glm::mat4 model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0, -10, 0));
	model = glm::scale(model, glm::vec3(0.1f));

	while (!glfwWindowShouldClose(window))
	{
		if(glfwGetKey(window, GLFW_KEY_ESCAPE))
			break;

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.Use();
		shader.SetMat4("uProjection", projection);
		shader.SetMat4("uView", view);
		shader.SetMat4("uModel", model);

		model = glm::rotate(model, glm::radians(0.025f), glm::vec3(0, 1, 0));

		mesh->Render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;

	return 0;
}