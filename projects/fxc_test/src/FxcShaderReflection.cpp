// ShaderReflection.cpp : Defines the entry point for the application.
//

#include <wrl.h>
#include "d3dcompiler.h"
#include "FxcShaderReflection.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include "CommonShader.h"

using namespace std;
using Microsoft::WRL::ComPtr;

int main()
{
	ComPtr<ID3DBlob> pBlob;
	ComPtr<ID3DBlob> pErrorBlob;
	uint32_t shader_size = (uint32_t)shader_source.size();
	auto hr = D3DCompile2(shader_source.c_str(), shader_size, nullptr, nullptr, nullptr, "main", "ps_5_1", 0, 0, 0, nullptr, 0, pBlob.GetAddressOf(), pErrorBlob.GetAddressOf());

	ComPtr<ID3D12ShaderReflection> pShaderReflection;
	if (hr != S_OK)
	{
		cout << "Compiling Shader failed!" << endl;
		return 0;
	}
	else
	{
		hr = D3DReflect(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), IID_PPV_ARGS(pShaderReflection.GetAddressOf()));
	}

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
			cout << "Constant Buffer: " << buffer_desc.Name << " with size: " << buffer_desc.Size << " and Flag: " << buffer_desc.uFlags << endl;
			auto var_count = buffer_desc.Variables;
			for (uint32_t j = 0; j < var_count; ++j)
			{
				auto var = const_buffer->GetVariableByIndex(j);
				D3D12_SHADER_VARIABLE_DESC var_desc;
				var->GetDesc(&var_desc);
				cout << "Variable: " << var_desc.Name << endl;
				cout << "Size: " << var_desc.Size << endl;
				cout << "DefaultValue: " << var_desc.DefaultValue << endl;
				cout << endl;
			}
		}

		cout << "Bindings: " << endl;
		auto binding_count = desc.BoundResources;
		for (uint32_t i = 0; i < binding_count; ++i)
		{
			D3D12_SHADER_INPUT_BIND_DESC desc;
			auto binding = pShaderReflection->GetResourceBindingDesc(i, &desc);
			cout << desc.Name << endl;
		}
	}
	else
	{
		cout << "FAILED: Cannot get reflection data!!!" << endl;
	}
	
	
	return 0;
}
