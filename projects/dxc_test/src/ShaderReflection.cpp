// ShaderReflection.cpp : Defines the entry point for the application.
//

#include <wrl.h>
#include "dxcapi.h"
#include "d3d12shader.h"
#include "ShaderReflection.h"
#include <fstream>
#include <filesystem>
#include <vector>

using namespace std;
using Microsoft::WRL::ComPtr;

static string shader_source = "float3              iMouse;\n"
"float4              iResolution;\n"
"float               iTime;\n"
"float               iFrame;\n"
"RWTexture2D<float4> outImage : register(u1);\n"
"\n"
"[numthreads(8, 8, 1)]\n"
"void csmain(uint3 tid : SV_DispatchThreadID)\n"
"{\n"
"    float4 outColor = (float4)0;\n"
"    float2 fragCoord = float2(tid.x, iResolution.y - tid.y) + float2(0.5, 0.5);\n"
"    outColor = float4(fragCoord, iTime, iFrame) * iMouse.xyzz;       \n"
"    outImage[tid.xy] = outColor;\n"
"}"
;

int main()
{
	ComPtr<IDxcUtils> pUtils;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));

	ComPtr<IDxcContainerReflection> pReflection;
	DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(pReflection.GetAddressOf()));

	bool compile_shader = true;
	DxcBuffer sourceBuffer;
	std::vector<char> raw_buffer;

	if (compile_shader)
	{
		cout << "Getting d3d12 reflection data from code directly" << endl;
		ComPtr<IDxcCompiler3> pCompiler3;
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler3.GetAddressOf()));

		ComPtr<IDxcIncludeHandler> pIncludeHandler;
		pUtils->CreateDefaultIncludeHandler(pIncludeHandler.ReleaseAndGetAddressOf());

		std::vector<LPCWSTR> arguments;
		//-E for the entry point (eg. PSMain)
		arguments.push_back(L"-E");
		arguments.push_back(L"csmain");

		//-T for the target profile (eg. ps_6_2)
		arguments.push_back(L"-T");
		arguments.push_back(L"cs_6_0");

		//Strip reflection data and pdbs (see later)
		arguments.push_back(L"-Qstrip_debug");
		arguments.push_back(L"-Qstrip_reflect");

		arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS); //-WX
		arguments.push_back(DXC_ARG_DEBUG); //-Zi
		arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp

		ComPtr<IDxcBlobEncoding> pSource;
		uint32_t shader_size = (uint32_t)shader_source.size();
		pUtils->CreateBlob(shader_source.c_str(), shader_size, CP_UTF8, pSource.GetAddressOf());

		sourceBuffer.Ptr = pSource->GetBufferPointer();
		sourceBuffer.Size = pSource->GetBufferSize();
		sourceBuffer.Encoding = 0;

		ComPtr<IDxcResult> pCompileResult;
		auto hr = pCompiler3->Compile(&sourceBuffer, arguments.data(), (uint32_t)arguments.size(), nullptr, IID_PPV_ARGS(pCompileResult.GetAddressOf()));

		ComPtr<IDxcBlob> pReflectionData;
		pCompileResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(pReflectionData.GetAddressOf()), nullptr);
		sourceBuffer.Ptr = pReflectionData->GetBufferPointer();
		sourceBuffer.Size = pReflectionData->GetBufferSize();
		sourceBuffer.Encoding = 0;
	}
	else
	{
		auto path = filesystem::current_path().string();
		path += "\\..\\..\\..\\shaders\\hjr.ref";
		cout << "Getting d3d12 reflection data from ref file: " << path << endl;

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		cout << "Create ifstream" << endl;
		std::streamsize size = file.tellg();
		cout << "Getting file size" << size << endl;
		file.seekg(0, std::ios::beg);
		cout << "file.seekg" << endl;
		raw_buffer.resize(size);
		if (file.read(raw_buffer.data(), size))
		{
			cout << "Yes I found dlls!!!" << path << endl;
			sourceBuffer.Ptr = raw_buffer.data();
			sourceBuffer.Size = size;
			sourceBuffer.Encoding = 0;
		}
		else
		{
			cout << "FAILED: No I cannot find dlls!!!" << endl;
			return 0;
		}
	}

	ComPtr<ID3D12ShaderReflection> pShaderReflection;
	HRESULT hr = pUtils->CreateReflection(&sourceBuffer, IID_PPV_ARGS(pShaderReflection.GetAddressOf()));
	if (hr == S_OK)
	{
		D3D12_SHADER_DESC desc;
		pShaderReflection->GetDesc(&desc);

		auto const_buffer_count = desc.ConstantBuffers;
		for (uint32_t i = 0; i < const_buffer_count; ++i)
		{
			auto const_buffer = pShaderReflection->GetConstantBufferByIndex(i);
			D3D12_SHADER_BUFFER_DESC buffer_desc;
			const_buffer->GetDesc(&buffer_desc);
			cout << "Constant Buffer: " << buffer_desc.Name << endl;
			auto var_count = buffer_desc.Variables;
			for (uint32_t j = 0; j < var_count; ++j)
			{
				auto var = const_buffer->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC var_desc;
				var->GetDesc(&var_desc);
				cout << "Variable: " << var_desc.Name << endl;
			}
		}
	}
	else
	{
		cout << "FAILED: Cannot get reflection data!!!" << endl;
	}
	
	
	return 0;
}
