#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <map>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class SkinnedMesh
{
public:
	SkinnedMesh() {};
	~SkinnedMesh();

	bool LoadMesh(const std::string& filename);

	void Render();

	int GetNumBones() const { return m_BoneNameToIndexMap.size(); }

private:	
	bool InitFromScene(const aiScene* scene, const std::string& filename);
	void CountVerticesAndIndices(const aiScene* scene, unsigned int& numVertices, unsigned int& numIndices);
	void ReserveSpaces(unsigned int numVertices, unsigned int numIndices);
	void InitAllMeshes(const aiScene* scene);
	void InitSingleMesh(unsigned int meshIndex, const aiMesh* mesh);
	bool InitMaterials(const aiScene* scene, const std::string& filename);
	void PopulateBuffers();

	void LoadMeshBones(int meshIndex, const aiMesh* mesh);
	void LoadSingleBone(int meshIndex, const aiBone* bone);
	int GetBoneId(const aiBone* bone);

#define MAX_NUM_BONES_PER_VERTEX 4
#define INVALID_MATERIAL 0xFFFFFFFF

	enum BufferType
	{
		INDEX_BUFFER	= 0,
		POSITION_VB		= 1,
		NORMAL_VB		= 2,
		TEXCOORD_VB		= 3,
		BONE_VB			= 4,
		NUM_BUFFERS		= 5 
	};

	struct BasicMeshEntry
	{
		BasicMeshEntry()
		{
			NumIndices = 0;
			BaseVertex = 0;
			BaseIndex = 0;
			MaterialIndex = INVALID_MATERIAL;
		}

		unsigned int NumIndices;
		unsigned int BaseVertex;
		unsigned int BaseIndex;
		unsigned int MaterialIndex;
	};

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

private:
	GLuint m_VAO;
	GLuint m_Buffers[BufferType::NUM_BUFFERS] = { 0 };

	std::vector<BasicMeshEntry> m_Meshes;
	std::vector<class Texture*> m_Textures;

	std::vector<glm::vec3> m_Positions;
	std::vector<glm::vec3> m_Normals;
	std::vector<glm::vec2> m_TexCoords;
	std::vector<unsigned int> m_Indices;
	std::vector<VertexBoneData> m_Bones;

	std::map<std::string, unsigned int> m_BoneNameToIndexMap;
};