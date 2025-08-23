#version 330 core

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform int uActiveBoneId;

in VS_OUT {
	vec2 TexCoords;
	vec3 Normal;
	flat ivec4 BoneIds;
	vec4 BoneWeights;
} fs_in;

void main()
{
	vec4 baseColor = texture(texture_diffuse1, fs_in.TexCoords);
	FragColor = baseColor;
}