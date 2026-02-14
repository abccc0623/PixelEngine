#include "DefaultShaderLoader.h"
#include <vector>
#include "PixelResources.h"
#include "PixelGraphicsLib.h"

DefaultShaderLoader::DefaultShaderLoader()
{

}

DefaultShaderLoader::~DefaultShaderLoader()
{

}

void DefaultShaderLoader::Initialize()
{
	
}

void DefaultShaderLoader::Release()
{

}


ID3D10Blob* DefaultShaderLoader::LoadCompileShader(int type, std::string name)
{
	// 2. 쉐이더 본문 리소스 로드
	std::string TotalPath = name;
	TotalPath += (type == PIXEL_VS) ? "_VS" : "_PS";
	std::wstring w_path(TotalPath.begin(), TotalPath.end());

	DWORD size = 0;
	void* buffer = DefaultLoadShader(w_path.c_str(), size);
	if (!buffer) return nullptr;

	// 3. [핵심] 문자열 합치기
	// 리소스 데이터는 끝에 \0이 없을 수 있으므로 크기를 명시해서 string을 생성해야 안전합니다.
	std::string fullSource;
	fullSource += "\n";
	fullSource.append((char*)buffer, size);

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;


	// 4. [수정] D3DCompile에 합쳐진 fullSource를 전달합니다.
	HRESULT hr = D3DCompile(
		fullSource.c_str(),     // [변경] buffer 대신 합쳐진 문자열 전달
		fullSource.length(),    // [변경] size 대신 합쳐진 문자열의 길이 전달
		nullptr,
		nullptr,
		nullptr,
		"main",
		(type == PIXEL_VS) ? "vs_5_0" : "ps_5_0",
		D3DCOMPILE_ENABLE_STRICTNESS, 0,
		&shaderBlob, &errorBlob
	);

	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}
		return nullptr;
	}
	return shaderBlob;
}


void* DefaultShaderLoader::DefaultLoadShader(const wchar_t* resourceName, DWORD& outSize)
{
	HMODULE hModule = GetModuleHandle(L"PixelGraphics.dll");
	HRSRC hRes = FindResource(hModule, resourceName, L"HLSL");
	HGLOBAL hData = LoadResource(hModule, hRes);
	outSize = SizeofResource(hModule, hRes);
	return LockResource(hData);
}