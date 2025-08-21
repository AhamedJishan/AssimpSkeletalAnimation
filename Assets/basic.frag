#version 330 core

out vec4 FragColor;

uniform sampler2D texture_diffuse1;

in vec2 TexCoords;
in vec3 Normal;

void main()
{
	vec4 baseColor = texture(texture_diffuse1, TexCoords);

	FragColor = baseColor;
}