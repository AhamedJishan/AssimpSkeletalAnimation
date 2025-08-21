#include "Texture.h"

#include <glad/glad.h>
#include <stbi/stb_image.h>
#include <iostream>

Texture::Texture()
{
	m_Id = 0;
	m_Width = 0;
	m_Height = 0;
}

Texture::~Texture()
{
	Clear();
}

bool Texture::Load(const std::string& filepath)
{
	Clear();

	int nrChannels;

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filepath.c_str(), &m_Width, &m_Height, &nrChannels, 0);

	if (!data)
	{
		printf("Failed to load texture '%s'\n", filepath.c_str());
		return false;
	}

	GLenum format;
	if (nrChannels == 1) format = GL_RED;
	else if (nrChannels == 3) format = GL_RGB;
	else if (nrChannels == 4) format = GL_RGBA;
	else
	{
		printf("%d channeled images are not supported\n", nrChannels);
		return false;
	}

	GLenum internalFormat = (nrChannels == 4) ? GL_RGBA : (nrChannels == 3 ? GL_RGB : GL_RED);

	glGenTextures(1, &m_Id);
	glBindTexture(GL_TEXTURE_2D, m_Id);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	return true;
}

void Texture::SetActive(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, m_Id);
}

void Texture::Clear()
{
	if(m_Id != 0)
	{
		glDeleteTextures(1, &m_Id);
		m_Id = 0;
	}
}
