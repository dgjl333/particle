#pragma once
#include <string>
#include <d3d12.h>


class Shader
{
public:
	Shader(const std::string& filePath);
	~Shader();

	ID3DBlob* GetVS() { return m_vsBlob; }
	ID3DBlob* GetPS() { return m_psBlob; }

private:
	ID3DBlob* m_vsBlob;
	ID3DBlob* m_psBlob;
};