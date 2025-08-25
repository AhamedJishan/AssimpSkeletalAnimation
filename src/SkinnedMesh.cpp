#include "SkinnedMesh.h"

#include <iostream>
#include "Texture.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

#define POSITION_LOCATION		0
#define NORMAL_LOCATION			1
#define TEXCOORD_LOCATION		2
#define BONE_ID_LOCATION		3
#define BONE_WEIGHT_LOCATION	4

glm::mat4 AiMatToGLM(const aiMatrix4x4& m)
{
	glm::mat4 r;
	r[0][0] = m.a1; r[1][0] = m.a2; r[2][0] = m.a3; r[3][0] = m.a4;
	r[0][1] = m.b1; r[1][1] = m.b2; r[2][1] = m.b3; r[3][1] = m.b4;
	r[0][2] = m.c1; r[1][2] = m.c2; r[2][2] = m.c3; r[3][2] = m.c4;
	r[0][3] = m.d1; r[1][3] = m.d2; r[2][3] = m.d3; r[3][3] = m.d4;
	return r;
}

glm::mat4 AiMatToGLM(const aiMatrix3x3& m)
{
	glm::mat4 r(1.0f);
	r[0][0] = m.a1; r[1][0] = m.a2; r[2][0] = m.a3;
	r[0][1] = m.b1; r[1][1] = m.b2; r[2][1] = m.b3;
	r[0][2] = m.c1; r[1][2] = m.c2; r[2][2] = m.c3;

	return r;
}

SkinnedMesh::~SkinnedMesh()
{
	// TODO: Clear
}

bool SkinnedMesh::LoadMesh(const std::string& filename)
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	glGenBuffers(ARRAY_SIZE_IN_ELEMENTS(m_Buffers), m_Buffers);

	bool ret = false;

	m_Scene = m_Importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

	if (m_Scene)
		ret = InitFromScene(m_Scene, filename);
	else
		printf("Error loading '%s': %s", filename.c_str(), m_Importer.GetErrorString());

	glBindVertexArray(0);

	return ret;
}

void SkinnedMesh::Render()
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

bool SkinnedMesh::InitFromScene(const aiScene* scene, const std::string& filename)
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

void SkinnedMesh::CountVerticesAndIndices(const aiScene* scene, unsigned int& numVertices, unsigned int& numIndices)
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

void SkinnedMesh::ReserveSpaces(unsigned int numVertices, unsigned int numIndices)
{
	m_Positions.reserve(numVertices);
	m_Normals.reserve(numVertices);
	m_TexCoords.reserve(numVertices);
	m_Indices.reserve(numIndices);
	m_Bones.resize(numVertices);
}

void SkinnedMesh::InitAllMeshes(const aiScene* scene)
{
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		InitSingleMesh(i, mesh);
	}
}

void SkinnedMesh::InitSingleMesh(unsigned int meshIndex, const aiMesh* mesh)
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

	LoadMeshBones(meshIndex, mesh);

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		m_Indices.push_back(face.mIndices[0]);
		m_Indices.push_back(face.mIndices[1]);
		m_Indices.push_back(face.mIndices[2]);
	}
}

void SkinnedMesh::LoadMeshBones(int meshIndex, const aiMesh* mesh)
{
	for (int i = 0; i < mesh->mNumBones; i++)
		LoadSingleBone(meshIndex, mesh->mBones[i]);
}

void SkinnedMesh::LoadSingleBone(int meshIndex, const aiBone* bone)
{
	int boneId = GetBoneId(bone);

	if (boneId == m_BoneInfos.size())
	{
		BoneInfo bi(AiMatToGLM(bone->mOffsetMatrix));
		m_BoneInfos.push_back(bi);
	}
	
	for (int i = 0; i < bone->mNumWeights; i++)
	{
		const aiVertexWeight& vw = bone->mWeights[i];
		unsigned int globalVertexId = m_Meshes[meshIndex].BaseVertex + vw.mVertexId;
		m_Bones[globalVertexId].AddBoneData(boneId, vw.mWeight);
	}
}

int SkinnedMesh::GetBoneId(const aiBone* bone)
{
	int boneId = 0;
	std::string boneName(bone->mName.C_Str());

	if (m_BoneNameToIndexMap.find(boneName) == m_BoneNameToIndexMap.end())
	{
		boneId = m_BoneNameToIndexMap.size();
		m_BoneNameToIndexMap[boneName] = boneId;
	}
	else
		boneId = m_BoneNameToIndexMap[boneName];

	return boneId;
}

void SkinnedMesh::GetBoneTransforms(double timeInSeconds, std::vector<glm::mat4>& transforms)
{
	transforms.resize(m_BoneInfos.size());

	double ticksPerSecond = m_Scene->mAnimations[0]->mTicksPerSecond != 0 ? m_Scene->mAnimations[0]->mTicksPerSecond : 25.0;
	double timeInTicks = timeInSeconds * ticksPerSecond;
	double animTimeInTicks = fmod(timeInTicks, m_Scene->mAnimations[0]->mDuration);

	// this transform to cancel out any transformations on rootnode
	glm::mat4 m_GlobalInverseTransform = glm::inverse(AiMatToGLM(m_Scene->mRootNode->mTransformation));
	ReadNodeHierarchy(animTimeInTicks, m_Scene->mRootNode, m_GlobalInverseTransform);

	for (unsigned int i = 0; i < m_BoneInfos.size(); i++)
		transforms[i] = m_BoneInfos[i].FinalTransformation;
}

void SkinnedMesh::ReadNodeHierarchy(double animationTimeInTicks, const aiNode* node, const glm::mat4& parentTransform)
{
	std::string nodeName = node->mName.C_Str();
	const aiAnimation* animation = m_Scene->mAnimations[0];

	glm::mat4 nodeTransform = AiMatToGLM(node->mTransformation);

	const aiNodeAnim* nodeAnim = FindNodeAnim(animation, nodeName);

	if (nodeAnim)
	{
		aiVector3D scaling;
		CalcInterpolatedScaling(scaling, animationTimeInTicks, nodeAnim);
		glm::mat4 scalingMat = glm::scale(glm::mat4(1), glm::vec3(scaling.x, scaling.y, scaling.z));

		aiQuaternion rotation;
		CalcInterpolatedRotation(rotation, animationTimeInTicks, nodeAnim);
		glm::mat4 rotationMat = AiMatToGLM(rotation.GetMatrix());

		aiVector3D position;
		CalcInterpolatedPosition(position, animationTimeInTicks, nodeAnim);
		glm::mat4 positionMat = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, position.z));

		nodeTransform = positionMat * rotationMat * scalingMat;
	}

	glm::mat4 globalTransformation = parentTransform * nodeTransform;

	if (m_BoneNameToIndexMap.find(nodeName) != m_BoneNameToIndexMap.end())
	{
		unsigned int boneIndex = m_BoneNameToIndexMap[nodeName];
		m_BoneInfos[boneIndex].FinalTransformation = globalTransformation * m_BoneInfos[boneIndex].OffsetMatrix;
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
		ReadNodeHierarchy(animationTimeInTicks, node->mChildren[i], globalTransformation);
}

const aiNodeAnim* SkinnedMesh::FindNodeAnim(const aiAnimation* animation, const std::string& nodeName)
{
	for (unsigned int i = 0; i < animation->mNumChannels; i++)
	{
		const aiNodeAnim* animNode = animation->mChannels[i];

		if (nodeName == animNode->mNodeName.C_Str())
			return animNode;
	}

	return nullptr;
}

void SkinnedMesh::CalcInterpolatedScaling(aiVector3D& outVector, double animationTimeInTicks, const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumScalingKeys == 1)
	{
		outVector = nodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned int scalingIndex = FindScaling(animationTimeInTicks, nodeAnim);
	unsigned int nextScalingIndex = scalingIndex + 1;

	float t1 = nodeAnim->mScalingKeys[scalingIndex].mTime;
	float t2 = nodeAnim->mScalingKeys[nextScalingIndex].mTime;
	float deltaTime = t2 - t1;
	float factor = ((float)animationTimeInTicks - t1) / deltaTime;
	const aiVector3D& start = nodeAnim->mScalingKeys[scalingIndex].mValue;
	const aiVector3D& end = nodeAnim->mScalingKeys[nextScalingIndex].mValue;
	aiVector3D delta = end - start;
	outVector = start + (float)factor * delta;
}

unsigned int SkinnedMesh::FindScaling(double animationTimeInTicks, const aiNodeAnim* nodeAnim)
{
	for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++)
	{
		double t = nodeAnim->mScalingKeys[i].mTime;
		if (animationTimeInTicks < t)
			return i;
	}
	return 0;
}

void SkinnedMesh::CalcInterpolatedRotation(aiQuaternion& outQuaternion, double animationTimeInTicks, const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumRotationKeys == 1)
	{
		outQuaternion = nodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned int rotationIndex = FindRotation(animationTimeInTicks, nodeAnim);
	unsigned int nextRotationIndex = rotationIndex + 1;

	float t1 = nodeAnim->mRotationKeys[rotationIndex].mTime;
	float t2 = nodeAnim->mRotationKeys[nextRotationIndex].mTime;
	float deltaTime = t2 - t1;
	float factor = ((float)animationTimeInTicks - t1) / deltaTime;
	const aiQuaternion& start = nodeAnim->mRotationKeys[rotationIndex].mValue;
	const aiQuaternion& end = nodeAnim->mRotationKeys[nextRotationIndex].mValue;
	aiQuaternion::Interpolate(outQuaternion, start, end, factor);
	//outQuaternion = start;
	outQuaternion.Normalize();
}

unsigned int SkinnedMesh::FindRotation(double animationTimeInTicks, const aiNodeAnim* nodeAnim)
{
	for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++)
	{
		double t = nodeAnim->mRotationKeys[i].mTime;
		if (animationTimeInTicks < t)
			return i;
	}
	return 0;
}

void SkinnedMesh::CalcInterpolatedPosition(aiVector3D& outVector, double animationTimeInTicks, const aiNodeAnim* nodeAnim)
{
	if (nodeAnim->mNumPositionKeys == 1)
	{
		outVector = nodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned int positionIndex = FindPosition(animationTimeInTicks, nodeAnim);
	unsigned int nextPositionIndex = positionIndex + 1;

	float t1 = nodeAnim->mPositionKeys[positionIndex].mTime;
	float t2 = nodeAnim->mPositionKeys[nextPositionIndex].mTime;
	float deltaTime = t2 - t1;
	float factor = ((float)animationTimeInTicks - t1) / deltaTime;
	const aiVector3D& start = nodeAnim->mPositionKeys[positionIndex].mValue;
	const aiVector3D& end = nodeAnim->mPositionKeys[nextPositionIndex].mValue;
	aiVector3D delta = end - start;
	outVector = start + (float)factor * delta;
}

unsigned int SkinnedMesh::FindPosition(double animationTimeInTicks, const aiNodeAnim* nodeAnim)
{
	for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++)
	{
		double t = nodeAnim->mPositionKeys[i].mTime;
		if (animationTimeInTicks < t)
			return i;
	}
	return 0;
}

bool SkinnedMesh::InitMaterials(const aiScene* scene, const std::string& filename)
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

void SkinnedMesh::PopulateBuffers()
{
	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BufferType::POSITION_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), &m_Positions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(POSITION_LOCATION);
	glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BufferType::NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), &m_Normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(NORMAL_LOCATION);
	glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BufferType::TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), &m_TexCoords[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(TEXCOORD_LOCATION);
	glVertexAttribPointer(TEXCOORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BufferType::BONE_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_Bones[0]) * m_Bones.size(), &m_Bones[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(BONE_ID_LOCATION);
	glVertexAttribIPointer(BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT, sizeof(VertexBoneData), 0);
	glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
	glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData),
		(const void*)(MAX_NUM_BONES_PER_VERTEX * sizeof(unsigned int)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[BufferType::INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);
}
