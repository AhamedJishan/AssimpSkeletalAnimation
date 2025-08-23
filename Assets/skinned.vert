#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in ivec4 aBoneIds;
layout (location = 4) in vec4 aBoneWeights;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;

const int MAX_BONES = 100;
uniform mat4 uBones[MAX_BONES];

out VS_OUT {
	vec2 TexCoords;
	vec3 Normal;
	flat ivec4 BoneIds;
	vec4 BoneWeights;
} vs_out;

void main()
{
	vs_out.TexCoords = aTexCoord;
	vs_out.Normal = transpose(inverse(mat3(uModel))) * aNormal;
	vs_out.BoneIds = aBoneIds;
	vs_out.BoneWeights = aBoneWeights;

	mat4 boneTransform = uBones[aBoneIds[0]] * aBoneWeights[0];
	boneTransform += uBones[aBoneIds[1]] * aBoneWeights[1];
	boneTransform += uBones[aBoneIds[2]] * aBoneWeights[2];
	boneTransform += uBones[aBoneIds[3]] * aBoneWeights[3];

	vec4 pos = vec4(aPos, 1);
	pos = boneTransform * pos;

	gl_Position = uProjection * uView * uModel * pos;
}