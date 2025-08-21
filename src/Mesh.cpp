#include "Mesh.h"

#include <iostream>
#include "Texture.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#define POSITION_LOCATION 0
#define NORMAL_LOCATION 1
#define TEXCOORD_LOCATION 2

Mesh::~Mesh()
{
	// TODO: Clear
}

bool Mesh::LoadMesh(const std::string& filename)
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

	bool ret = false;
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

	if (scene)
		ret = InitFromScene(scene, filename);
	else
		printf("Error loading '%s': %s", filename.c_str(), importer.GetErrorString());

	glBindVertexArray(0);

	return ret;
}

void Mesh::Render()
{
	glBindVertexArray(m_VAO);

	for (unsigned int i = 0; i < m_Meshes.size(); i++)
	{
		unsigned int materialIndex = m_Meshes[i].MaterialIndex;
		m_Textures[materialIndex]->SetActive();

		glDrawElementsBaseVertex (  GL_TRIANGLES,
									m_Meshes[i].NumIndices,
									GL_UNSIGNED_INT,
									(void*)(sizeof(unsigned int) * m_Meshes[i].BaseIndex),
									m_Meshes[i].BaseVertex );
	}

	glBindVertexArray(0);
}

bool Mesh::InitFromScene(const aiScene* scene, const std::string& filename)
{
	m_Meshes.resize(scene->mNumMeshes);
	m_Textures.resize(scene->mNumMaterials);

	unsigned int numVertices = 0;
	unsigned int numIndices = 0;

	CountVerticesAndIndices(scene, numVertices, numIndices);

	ReserveSpaces(numVertices, numIndices);

	InitAllMeshes(scene);

	if (!InitMaterials(scene, filename))
		return false;

	PopulateBuffers();
}

void Mesh::CountVerticesAndIndices(const aiScene* scene, unsigned int& numVertices, unsigned int& numIndices)
{
	for (unsigned int i = 0; i < m_Meshes.size(); i++)
	{
		m_Meshes[i].MaterialIndex = scene->mMeshes[i]->mMaterialIndex;
		m_Meshes[i].NumIndices = scene->mMeshes[i]->mNumFaces * 3;
		m_Meshes[i].BaseIndex = numIndices;
		m_Meshes[i].BaseVertex = numVertices;

		numIndices += m_Meshes[i].NumIndices;
		numVertices += scene->mMeshes[i]->mNumVertices;
	}
}

void Mesh::ReserveSpaces(unsigned int numVertices, unsigned int numIndices)
{
	m_Positions.reserve(numVertices);
	m_Normals.reserve(numVertices);
	m_TexCoords.reserve(numVertices);
	m_Indices.reserve(numIndices);
}

void Mesh::InitAllMeshes(const aiScene* scene)
{
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		InitSingleMesh(mesh);
	}
}

void Mesh::InitSingleMesh(const aiMesh* mesh)
{
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		const aiVector3D& pos = mesh->mVertices[i];
		const aiVector3D& normal = mesh->mNormals[i];
		const aiVector3D& texCoords = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][i] : aiVector3D(0, 0, 0);

		m_Positions.push_back(glm::vec3(pos.x, pos.y, pos.z));
		m_Normals.push_back(glm::vec3(normal.x, normal.y, normal.z));
		m_TexCoords.push_back(glm::vec2(texCoords.x, texCoords.y));
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		m_Indices.push_back(face.mIndices[0]);
		m_Indices.push_back(face.mIndices[1]);
		m_Indices.push_back(face.mIndices[2]);
	}
}

bool Mesh::InitMaterials(const aiScene* scene, const std::string& filename)
{
	std::string dir = filename.substr(0, filename.find_last_of('/') + 1);

	bool ret = true;

	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		const aiMaterial* mat = scene->mMaterials[i];
		if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			aiString path;
			mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
			std::string texturePath = dir + path.C_Str();
			Texture* texture = new Texture();
			if (!texture->Load(texturePath))
			{
				delete texture;
				texture = nullptr;
				ret = false;
			}
			else
				m_Textures[i] = texture;

			printf("Loaded Texture '%s'\n", texturePath.c_str());
		}
	}

	return ret;
}

void Mesh::PopulateBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BufferType::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), &m_Positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BufferType::NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), &m_Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BufferType::TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), &m_TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TEXCOORD_LOCATION);
	glVertexAttribPointer(TEXCOORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[BufferType::INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
}
