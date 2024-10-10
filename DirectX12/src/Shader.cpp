#include "Shader.h"
#include "Utils.h"
#include "Debug.h"
#include <d3dcompiler.h>


Shader::Shader(const std::string& filePath)
{
#ifdef _DEBUG
	UINT flag = D3DCOMPILE_DEBUG;
#else
	UINT flag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif
	ID3DBlob* errorBlobVS;
	HRESULT result = D3DCompileFromFile(Utils::GetWStringFromString(ProjectPath + filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vert", "vs_5_0", flag, 0, &m_vsBlob, &errorBlobVS);

	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			print("Vertex Shader: File not found");
			exit(1);
		}
		else if (result == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
		{
			print("Vertex Shader: Path not found");
			exit(1);
		}
		else if (errorBlobVS)
		{
			print("Vertex Shader", (char*)errorBlobVS->GetBufferPointer());
			errorBlobVS->Release();
			exit(1);
		}
	}

	ID3DBlob* errorBlobPS;
	result = D3DCompileFromFile(Utils::GetWStringFromString(ProjectPath + filePath).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "frag", "ps_5_0", flag, 0, &m_psBlob, &errorBlobPS);

	if (FAILED(result))
	{
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
		{
			print("Pixel Shader: File not found");
			exit(1);
		}
		else if (result == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
		{
			print("Pixel Shader: Path not found");
			exit(1);
		}
		else if (errorBlobPS)
		{
			print("Pixel Shader", (char*)errorBlobPS->GetBufferPointer());
			errorBlobPS->Release();
			exit(1);
		}
	}
}

Shader::~Shader()
{
	m_psBlob->Release();
	m_vsBlob->Release();
}
