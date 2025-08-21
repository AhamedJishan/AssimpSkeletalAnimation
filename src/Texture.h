#pragma once

#include <string>

class Texture
{
public:
	Texture();
	~Texture();

	bool Load(const std::string& filepath);
	void SetActive(int slot = 0);

private:
	void Clear();

private:
	unsigned int m_Id;
	int m_Width;
	int m_Height;
};