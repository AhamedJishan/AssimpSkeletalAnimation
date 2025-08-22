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
	//vec4 baseColor = texture(texture_diffuse1, fs_in.TexCoords);
	vec4 baseColor = vec4(0, 0, 1, 1);

	for(int i = 0; i < 4; i++)
	{
		if(fs_in.BoneIds[i] == uActiveBoneId)
		{
			if (fs_in.BoneWeights[i] >= 0.7)
			{
				baseColor = vec4(1, 0, 0, 1) * fs_in.BoneWeights[i];
			}
			else if (fs_in.BoneWeights[i] >= 0.4)
			{
				baseColor = vec4(0, 1, 0, 1) * fs_in.BoneWeights[i];
			}
			else if (fs_in.BoneWeights[i] >= 0.05)
			{
				baseColor = vec4(1, 1, 0, 1) * fs_in.BoneWeights[i];
			}
			break;
		}
	}


	FragColor = baseColor;
}