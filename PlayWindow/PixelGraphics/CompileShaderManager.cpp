/** 구조체 관련 */
#include <vector>
#include "CompileShaderManager.h"
#include "ShaderResources.h"
#include "PixelGraphicsLib.h"

CompileShaderManager::CompileShaderManager()
{

}

CompileShaderManager::~CompileShaderManager()
{

}


void CompileShaderManager::Initialize()
{
	std::string finalSource = "";
	headerPtr = DefaultLoadShader(L"MAIN_HEADER_RESOURCE", headerSize);
	if (headerPtr) {
		finalSource.append("\n/* HEADER START */\n");
		finalSource.append((const char*)headerPtr, headerSize);
		finalSource.append("\n/* HEADER END */\n\n");
	}
	mShaderResources_List.insert({"Debug",DefaultCompileShader("DEBUG")});
	mShaderResources_List.insert({"Static",DefaultCompileShader("STATIC")});
	mShaderSampler_List.insert({ "Basic",	CreateSampler() });
}

void CompileShaderManager::Release()
{
	delete this;
}

ShaderResources* CompileShaderManager::CompileShader(std::string ShaderName)
{
	ShaderResources* mShaderResources = new ShaderResources();

	ID3D10Blob* VertexShaderBuffer = CompileShader(SHADER_TYPE::VS, ShaderName);
	ID3D10Blob* PixelShaderBuffer  = CompileShader(SHADER_TYPE::PS, ShaderName);

	CreateVertexShader(VertexShaderBuffer, mShaderResources);
	CreatePixelShader(PixelShaderBuffer, mShaderResources);

	//모두 사용한 데이터는 제거
	VertexShaderBuffer->Release();
	PixelShaderBuffer->Release();

	return mShaderResources;
}


ShaderResources* CompileShaderManager::DefaultCompileShader(std::string ShaderName)
{
	ShaderResources* mShaderResources = new ShaderResources();

	ID3D10Blob* VertexShaderBuffer = DefaultCompileShader(SHADER_TYPE::VS, ShaderName, mShaderResources);
	ID3D10Blob* PixelShaderBuffer  = DefaultCompileShader(SHADER_TYPE::PS, ShaderName, mShaderResources);

	CreateVertexShader(VertexShaderBuffer, mShaderResources);
	CreatePixelShader(PixelShaderBuffer, mShaderResources);

	//모두 사용한 데이터는 제거
	VertexShaderBuffer->Release();
	PixelShaderBuffer->Release();

	return mShaderResources;
}


void* CompileShaderManager::DefaultLoadShader(const wchar_t* resourceName, DWORD& outSize)
{
	HMODULE hModule = GetModuleHandle(L"PixelGraphics.dll");
	HRSRC hRes = FindResource(hModule, resourceName, L"SHADER");
	HGLOBAL hData = LoadResource(hModule, hRes);
	outSize = SizeofResource(hModule, hRes);
	return LockResource(hData);
}

ID3D10Blob* CompileShaderManager::DefaultCompileShader(SHADER_TYPE mType, std::string& ShaderName, ShaderResources* SR)
{
	// 1. 헤더 리소스 로드 (headerPtr, headerSize가 클래스 멤버가 아니라면 여기서 로드해야 함)
	// 예: void* headerPtr = DefaultLoadShader(L"MAIN_HEADER_RESOURCE", headerSize);
	if (!headerPtr) return nullptr;

	// 2. 쉐이더 본문 리소스 로드
	std::string TotalPath = ShaderName;
	TotalPath += (mType == SHADER_TYPE::VS) ? "_VS_SHADER" : "_PS_SHADER";
	std::wstring w_path(TotalPath.begin(), TotalPath.end());

	DWORD size = 0;
	void* buffer = DefaultLoadShader(w_path.c_str(), size);
	if (!buffer) return nullptr;

	// 3. [핵심] 문자열 합치기
	// 리소스 데이터는 끝에 \0이 없을 수 있으므로 크기를 명시해서 string을 생성해야 안전합니다.
	std::string fullSource;
	fullSource.assign((char*)headerPtr, headerSize);
	fullSource += "\n\n";
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
		(mType == SHADER_TYPE::VS) ? "vs_5_0" : "ps_5_0",
		D3DCOMPILE_DEBUG, 0,
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

ID3D10Blob* CompileShaderManager::CompileShader(SHADER_TYPE mType, std::string& ShaderName)
{
	ID3D10Blob* ShaderBuffer = nullptr;

	std::string		TotalPath	= "";
	std::string		Version		= "";

	TotalPath += ShaderFilePath;
	TotalPath += ShaderName;

	/** 쉐이더 타입을 설정 */
	switch (mType)
	{
	case SHADER_TYPE::VS:
		TotalPath  += "_vs.hlsl";
		Version		= "vs_5_0";
		break;
	case SHADER_TYPE::PS:
		TotalPath  += "_ps.hlsl";
		Version		= "ps_5_0";
		break;
	}

	std::wstring _path = std::wstring(TotalPath.begin(), TotalPath.end());
	const wchar_t* w_path = _path.c_str();

	//플래그 설정
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif 
	//버택스 쉐이더를 로드 한다
	D3DCompileFromFile
	(
		w_path,								//쉐이더 코드가 들어있는 파일이름
		NULL,								//쉐이더 매크로를 정의 사용하지않으면 NULL
		D3D_COMPILE_STANDARD_FILE_INCLUDE,	//기본포함 핸들러에 대한 포인터 (기본값 NULL)
		"main",								//쉐이더 실행이 시작되는 쉐이더 진입점 함수의 이름
		Version.c_str(),					//컴파일할 쉐이더 대상 버전
		dwShaderFlags,						//컴파일 쉐이더 플레그 1
		0,									//컴파일 쉐이더 플레그 2
		&ShaderBuffer,						//반환값
		NULL								//에러메세지 반환값
	);

	return ShaderBuffer;
}

HRESULT CompileShaderManager::CreateVertexShader(ID3D10Blob* mShaderBuffer, ShaderResources* SR)
{
	//필요한 리소스들
	HRESULT hr;
	ID3D11VertexShader* mVertexShader = nullptr;

	//링커를 생성
	ID3D11ClassLinkage* g_pPsClassLinkage = nullptr;
	GetDevice()->CreateClassLinkage(&g_pPsClassLinkage);

	hr = GetDevice()->CreateVertexShader
	(
		mShaderBuffer->GetBufferPointer(),
		mShaderBuffer->GetBufferSize(),
		g_pPsClassLinkage,
		&(SR->mVertexShader)
	);
	if (FAILED(hr)) { return hr; }

	//리플렉션 객체를 생성
	ID3D11ShaderReflection* mReFlector = nullptr;
	hr = D3DReflect
	(
		mShaderBuffer->GetBufferPointer(),
		mShaderBuffer->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)&mReFlector
	);
	if (FAILED(hr)) { return hr; }

	//인풋 레이아웃 생성
	SR->mLayout = CreateInputLayout(mReFlector, mShaderBuffer);

	//상수 버퍼 생성
	CreateContantBuffer(mReFlector, SR);

	//리플렉션 객체 삭제
	mReFlector->Release();
	return hr;
}

HRESULT CompileShaderManager::CreatePixelShader(ID3D10Blob* mShaderBuffer, ShaderResources* SR)
{
	HRESULT hr;

	//링커를 생성
	ID3D11ClassLinkage* g_pPsClassLinkage = nullptr;
	GetDevice()->CreateClassLinkage(&g_pPsClassLinkage);

	// 버퍼에서 픽셀 쉐이더를 생성합니다.
	ID3D11PixelShader* mPixelShader = nullptr;
	hr = GetDevice()->CreatePixelShader
	(
		mShaderBuffer->GetBufferPointer(),
		mShaderBuffer->GetBufferSize(),
		g_pPsClassLinkage,
		&(SR->mPixelShader)
	);

	return hr;
}

ID3D11InputLayout* CompileShaderManager::CreateInputLayout(ID3D11ShaderReflection* mReflection, ID3D10Blob* mShaderBuffer)
{
	//쉐디어 정보 가져오기
	D3D11_SHADER_DESC desc;
	mReflection->GetDesc(&desc);

	//쉐이더 레이아웃 개수를 가져옴
	UINT inputLayCount = desc.InputParameters;

	//레이아웃 개수만큼 배열사이즈를 잡아줌
	std::vector<D3D11_INPUT_ELEMENT_DESC> DescList;
	DescList.resize((int)inputLayCount);

	//처음 들어가는곳은 0번쨰부터 들어가기때문에
	int TypeSize = 0;

	for (int i = 0; i < (int)inputLayCount; i++)
	{
		//한개의 레이아웃 구조체를 생성함
		D3D11_SIGNATURE_PARAMETER_DESC desc;
		mReflection->GetInputParameterDesc(i, &desc);

		unsigned char numberMask = desc.Mask;
		int number = (int)desc.ComponentType;

		DXGI_FORMAT FormatNumver = FindFormat(number, numberMask);

		//한개의 레이아웃 구조체값 넣기
		DescList[i].SemanticName = desc.SemanticName;
		DescList[i].SemanticIndex = 0;
		DescList[i].Format = FormatNumver;
		DescList[i].InputSlot = 0;
		DescList[i].AlignedByteOffset = TypeSize;
		DescList[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		DescList[i].InstanceDataStepRate = 0;

		//한개가 완성이 되었고 자료형의 끝을 알려줘야 다음놈이 그다음으로 들어오기때문에
		switch (numberMask)
		{
		case 0b00000001:	//float
			TypeSize += 4;
			break;
		case 0b00000011:	//float2
			TypeSize += 8;
			break;
		case 0b00000111:	//float3
			TypeSize += 12;
			break;
		case 0b00001111:	//float4
			TypeSize += 16;
			break;
		}
	}


	//레이아웃 구조체의 값은 다넣었고 생성해준다
	ID3D11InputLayout* m_layout = nullptr;
	HRESULT result;

	result = GetDevice()->CreateInputLayout
	(
		&DescList[0],
		inputLayCount,
		mShaderBuffer->GetBufferPointer(),
		mShaderBuffer->GetBufferSize(),
		&m_layout
	);

	return m_layout;
}

ID3D11SamplerState* CompileShaderManager::CreateSampler()
{
	ID3D11SamplerState* Sampler = nullptr;
	//기본 샘플러
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	GetDevice()->CreateSamplerState(&samplerDesc, &Sampler);
	return Sampler;
}

DXGI_FORMAT CompileShaderManager::FindFormat(int Number, byte Mask)
{
	DXGI_FORMAT FormatType = DXGI_FORMAT();

	if (Number == D3D_REGISTER_COMPONENT_FLOAT32)
	{
		//number + 1;
		switch (Mask)
		{
		case 0b00000001:
			FormatType = DXGI_FORMAT_R32_FLOAT;
			break;
		case 0b00000011:
			FormatType = DXGI_FORMAT_R32G32_FLOAT;
			break;
		case 0b00000111:
			FormatType = DXGI_FORMAT_R32G32B32_FLOAT;
			break;
		case 0b00001111:
			FormatType = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		default:
			break;
		}
	}
	else if (Number == D3D_REGISTER_COMPONENT_UINT32)
	{
		switch (Mask)
		{
		case 0b00000001:
			FormatType = DXGI_FORMAT_R32_UINT;
			break;
		case 0b00000011:
			FormatType = DXGI_FORMAT_R32G32_UINT;
			break;
		case 0b00000111:
			FormatType = DXGI_FORMAT_R32G32B32_UINT;
			break;
		case 0b00001111:
			FormatType = DXGI_FORMAT_R32G32B32A32_UINT;
			break;
		default:
			break;
		}
	}
	else if (Number == D3D_REGISTER_COMPONENT_SINT32)
	{
		switch (Mask)
		{
		case 0b00000001:
			FormatType = DXGI_FORMAT_R32_SINT;
			break;
		case 0b00000011:
			FormatType = DXGI_FORMAT_R32G32_SINT;
			break;
		case 0b00000111:
			FormatType = DXGI_FORMAT_R32G32B32_SINT;
			break;
		case 0b00001111:
			FormatType = DXGI_FORMAT_R32G32B32A32_SINT;
			break;
		default:
			break;
		}
	}
	return FormatType;
}

ID3D11Buffer* CompileShaderManager::CreateContantBuffer(ID3D11ShaderReflection* mReflection, ShaderResources* SR)
{
	D3D11_SHADER_DESC desc;
	mReflection->GetDesc(&desc);

	//쉐이더의 cbuffer의 수
	UINT CBufferCount = desc.ConstantBuffers;
	//c버퍼를 연다!!
	//버퍼의 개수만큼 cbuffer를 가져옴
	for (int i = 0; i < (int)CBufferCount; i++)
	{
		ID3D11ShaderReflectionConstantBuffer* Cbuffer = mReflection->GetConstantBufferByIndex(i);
		D3D11_SHADER_BUFFER_DESC buffer_Desc;
		Cbuffer->GetDesc(&buffer_Desc);
		std::string name = buffer_Desc.Name;

		//만약 이미버퍼에 들어가있는 상수버퍼라면
		if (mContextBuffer_List.find(name) != mContextBuffer_List.end())
		{
			continue;
		}

		//총 전체의 사이즈
		UINT size = buffer_Desc.Size;
		UINT plags = buffer_Desc.uFlags;
		//변수의 개수
		UINT var = buffer_Desc.Variables;


		ID3D11Buffer* m_GlobalBuffer;
		D3D11_BUFFER_DESC matrixBufferDesc;
		matrixBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		matrixBufferDesc.ByteWidth = size;
		matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags = 0;
		matrixBufferDesc.MiscFlags = 0;
		matrixBufferDesc.StructureByteStride = 0;

		//버퍼를 만들어줌
		GetDevice()->CreateBuffer(&matrixBufferDesc, NULL, &m_GlobalBuffer);
		mContextBuffer_List.insert({ name,m_GlobalBuffer });

	}
	return nullptr;
}

ID3D11InputLayout* CompileShaderManager::CreateInputLayout(ID3D11ShaderReflection* mReflection, void* mShaderBuffer, SIZE_T size)
{
	//쉐디어 정보 가져오기
	D3D11_SHADER_DESC desc;
	mReflection->GetDesc(&desc);

	//쉐이더 레이아웃 개수를 가져옴
	UINT inputLayCount = desc.InputParameters;

	//레이아웃 개수만큼 배열사이즈를 잡아줌
	std::vector<D3D11_INPUT_ELEMENT_DESC> DescList;
	DescList.resize((int)inputLayCount);

	//처음 들어가는곳은 0번쨰부터 들어가기때문에
	int TypeSize = 0;

	for (int i = 0; i < (int)inputLayCount; i++)
	{
		//한개의 레이아웃 구조체를 생성함
		D3D11_SIGNATURE_PARAMETER_DESC desc;
		mReflection->GetInputParameterDesc(i, &desc);

		unsigned char numberMask = desc.Mask;
		int number = (int)desc.ComponentType;

		DXGI_FORMAT FormatNumver = FindFormat(number, numberMask);

		//한개의 레이아웃 구조체값 넣기
		DescList[i].SemanticName = desc.SemanticName;
		DescList[i].SemanticIndex = 0;
		DescList[i].Format = FormatNumver;
		DescList[i].InputSlot = 0;
		DescList[i].AlignedByteOffset = TypeSize;
		DescList[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		DescList[i].InstanceDataStepRate = 0;

		//한개가 완성이 되었고 자료형의 끝을 알려줘야 다음놈이 그다음으로 들어오기때문에
		switch (numberMask)
		{
		case 0b00000001:	//float
			TypeSize += 4;
			break;
		case 0b00000011:	//float2
			TypeSize += 8;
			break;
		case 0b00000111:	//float3
			TypeSize += 12;
			break;
		case 0b00001111:	//float4
			TypeSize += 16;
			break;
		}
	}


	//레이아웃 구조체의 값은 다넣었고 생성해준다
	ID3D11InputLayout* m_layout = nullptr;
	HRESULT result;

	result = GetDevice()->CreateInputLayout
	(
		&DescList[0],
		inputLayCount,
		mShaderBuffer,
		size,
		&m_layout
	);

	return m_layout;
}

