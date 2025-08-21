#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uView;
uniform mat4 uModel;

out vec2 TexCoords;
out vec3 Normal;

void main()
{
	TexCoords = aTexCoord;
	Normal = transpose(inverse(mat3(uModel))) * aNormal;

	vec4 pos = vec4(aPos, 1);

	gl_Position = uProjection * uView * uModel * pos;
}