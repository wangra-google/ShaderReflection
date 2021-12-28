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

int main()
{
	ComPtr<IDxcUtils> pUtils;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));

	ComPtr<IDxcContainerReflection> pReflection;
	DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(pReflection.GetAddressOf()));

	ComPtr<IDxcCompiler3> pCompiler3;
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(pCompiler3.GetAddressOf()));

	ComPtr<IDxcIncludeHandler> pIncludeHandler;
	pUtils->CreateDefaultIncludeHandler(pIncludeHandler.ReleaseAndGetAddressOf());
	auto path = filesystem::current_path().string();
	path += "\\..\\shaders\\hjr.ref";
	cout << "Getting d3d12 reflection data from "<< path << endl;

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	cout << "Create ifstream" << endl;
	std::streamsize size = file.tellg();
	cout << "Getting file size" << size << endl;
	file.seekg(0, std::ios::beg);
	cout << "file.seekg" << endl;

	std::vector<char> buffer(size);
	if (file.read(buffer.data(), size))
	{
		cout << "Yes I found dlls!!!" << path << endl;
		DxcBuffer reflectionBuffer;
		reflectionBuffer.Ptr = buffer.data();
		reflectionBuffer.Size = size;
		reflectionBuffer.Encoding = 0;
		ComPtr<ID3D12ShaderReflection> pShaderReflection;
		pUtils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(pShaderReflection.GetAddressOf()));
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
		cout << "No I cannot find dlls!!!" << endl;
	}

	/*ComPtr<IDxcResult> pResult;
	auto hr = pCompiler3->Compile(&source, argument.data(), argument.size(), pIncludeHandler.Get(), IID_PPV_ARGS(&pResult));*/
	return 0;
}
