#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Mesh
{
public:
	Mesh() {};
	~Mesh();

	bool LoadMesh(const std::string& filename);

	void Render();

private:
	bool InitFromScene(const aiScene* scene, const std::string& filename);
	void CountVerticesAndIndices(const aiScene* scene, unsigned int& numVertices, unsigned int& numIndices);
	void ReserveSpaces(unsigned int numVertices, unsigned int numIndices);
	void InitAllMeshes(const aiScene* scene);
	void InitSingleMesh(const aiMesh* mesh);
	bool InitMaterials(const aiScene* scene, const std::string& filename);
	void PopulateBuffers();

#define INVALID_MATERIAL 0xFFFFFFFF

	enum BufferType
	{
		INDEX		= 0,
		POSITION	= 1,
		NORMAL		= 2,
		TEXCOORD	= 3,
		NUM_BUFFERS = 4 
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

private:
	GLuint m_VAO;
	GLuint m_Buffers[BufferType::NUM_BUFFERS] = { 0 };

	std::vector<BasicMeshEntry> m_Meshes;
	std::vector<class Texture*> m_Textures;

	std::vector<glm::vec3> m_Positions;
	std::vector<glm::vec3> m_Normals;
	std::vector<glm::vec2> m_TexCoords;
	std::vector<unsigned int> m_Indices;
};