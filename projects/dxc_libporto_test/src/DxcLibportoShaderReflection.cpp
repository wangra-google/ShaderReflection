// ShaderReflection.cpp : Defines the entry point for the application.
//

#include "DxcLibportoShaderReflection.h"
#include "d3d12shader.h"
#include "dxcapi.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <wrl.h>
#include "CommonShader.h"


// this is from DirectXShaderCompiler\include\dxc\DxilContainer\DxilContainer.h
// which is not supposed to be a public header!!!
#define DXIL_FOURCC(ch0, ch1, ch2, ch3)                                        \
  ((uint32_t)(uint8_t)(ch0) | (uint32_t)(uint8_t)(ch1) << 8 |                  \
   (uint32_t)(uint8_t)(ch2) << 16 | (uint32_t)(uint8_t)(ch3) << 24)

using namespace std;
using Microsoft::WRL::ComPtr;

int main() {
	ComPtr<IDxcUtils> pUtils;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));

  ComPtr<IDxcLibrary> pLibrary;
  DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(pLibrary.GetAddressOf()));

  ComPtr<IDxcContainerReflection> pContainer;
  DxcCreateInstance(CLSID_DxcContainerReflection,
                    IID_PPV_ARGS(pContainer.GetAddressOf()));

  bool compile_shader = false;
  bool test_library = false;
  DxcBuffer shaderBuffer;
  std::vector<char> raw_buffer;
  ComPtr<IDxcBlob> pReflectionData;
  ComPtr<IDxcBlob> pShaderBlob;
  ComPtr<ID3D12ShaderReflection> pShaderReflection;

  if (compile_shader) {
    cout << "Getting d3d12 reflection data from code directly" << endl;
    ComPtr<IDxcCompiler3> pCompiler3;
    DxcCreateInstance(CLSID_DxcCompiler,
                      IID_PPV_ARGS(pCompiler3.GetAddressOf()));

    ComPtr<IDxcIncludeHandler> pIncludeHandler;
    pUtils->CreateDefaultIncludeHandler(
        pIncludeHandler.ReleaseAndGetAddressOf());

    std::vector<LPCWSTR> arguments;
    //-E for the entry point (eg. PSMain)
    arguments.push_back(L"-E");
    arguments.push_back(L"main");

    //-T for the target profile (eg. ps_6_2)
    arguments.push_back(L"-T");
    arguments.push_back(L"ps_6_0");

    // Strip reflection data and pdbs (see later)
    arguments.push_back(L"-Qstrip_debug");
    arguments.push_back(L"-Qstrip_reflect");

    arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);   //-WX
    arguments.push_back(DXC_ARG_DEBUG);                 //-Zi
    arguments.push_back(DXC_ARG_PACK_MATRIX_ROW_MAJOR); //-Zp

    ComPtr<IDxcBlobEncoding> pSource;
    uint32_t shader_size = (uint32_t)shader_source.size();
    pUtils->CreateBlob(shader_source.c_str(), shader_size, CP_UTF8,
                       pSource.GetAddressOf());

    shaderBuffer.Ptr = pSource->GetBufferPointer();
    shaderBuffer.Size = pSource->GetBufferSize();
    shaderBuffer.Encoding = 0;

    ComPtr<IDxcResult> pCompileResult;
    auto hr = pCompiler3->Compile(&shaderBuffer, arguments.data(),
                                  (uint32_t)arguments.size(), nullptr,
                                  IID_PPV_ARGS(pCompileResult.GetAddressOf()));

    if (test_library) {
      pCompileResult->GetResult(pShaderBlob.GetAddressOf());
      ComPtr<IDxcBlobEncoding> pBlobEncoding;
      pLibrary->CreateBlobWithEncodingFromPinned(
          pShaderBlob->GetBufferPointer(),
          (uint32_t)pShaderBlob->GetBufferSize(), CP_UTF8,
          pBlobEncoding.GetAddressOf());
      pContainer->Load(pBlobEncoding.Get());
      UINT32 shader_idx;
      pContainer->FindFirstPartKind(DXIL_FOURCC('D', 'X', 'I', 'L'),
                                    &shader_idx);
      pContainer->GetPartReflection(
          shader_idx, IID_PPV_ARGS(pShaderReflection.GetAddressOf()));
    } else {
      pCompileResult->GetOutput(DXC_OUT_REFLECTION,
                                IID_PPV_ARGS(pReflectionData.GetAddressOf()),
                                nullptr);
      shaderBuffer.Ptr = pReflectionData->GetBufferPointer();
      shaderBuffer.Size = pReflectionData->GetBufferSize();
      shaderBuffer.Encoding = 0;
    }

  } else {
    auto path = filesystem::current_path().string();
    // both reflection and dxil would work
	//path += "\\..\\..\\..\\shaders\\hjr.ref";
	//path += "\\..\\..\\..\\shaders\\HJ.dxil";
	path += "\\..\\..\\..\\shaders\\HJ_strip_reflect.dxil";
    cout << "Getting d3d12 reflection data from ref file: " << path << endl;

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    cout << "Create ifstream" << endl;
    std::streamsize size = file.tellg();
    cout << "Getting file size" << size << endl;
    file.seekg(0, std::ios::beg);
    cout << "file.seekg" << endl;
    raw_buffer.resize(size);
    if (file.read(raw_buffer.data(), size)) {
      cout << "Yes I found dlls!!!" << path << endl;
	  // raw buffer could contain dxil or just reflection data
      shaderBuffer.Ptr = raw_buffer.data();
      shaderBuffer.Size = size;
      shaderBuffer.Encoding = 0;
    } else {
      cout << "FAILED: No I cannot find dlls!!!" << endl;
      return 0;
    }
  }

  HRESULT hr = S_OK;

  if (!test_library) {
    hr = pUtils->CreateReflection(&shaderBuffer,
                             IID_PPV_ARGS(pShaderReflection.GetAddressOf()));
  }

  if (hr == S_OK) {
    D3D12_SHADER_DESC desc;
    pShaderReflection->GetDesc(&desc);

    auto const_buffer_count = desc.ConstantBuffers;
    for (uint32_t i = 0; i < const_buffer_count; ++i) {
      auto const_buffer = pShaderReflection->GetConstantBufferByIndex(i);
      D3D12_SHADER_BUFFER_DESC buffer_desc;
      const_buffer->GetDesc(&buffer_desc);
      cout << "Constant Buffer: " << buffer_desc.Name << endl;
      auto var_count = buffer_desc.Variables;
      for (uint32_t j = 0; j < var_count; ++j) {
        auto var = const_buffer->GetVariableByIndex(j);
        D3D12_SHADER_VARIABLE_DESC var_desc;
        ID3D12ShaderReflectionConstantBuffer *buffer = var->GetBuffer();
        D3D12_SHADER_BUFFER_DESC buffer_desc_1;
        buffer->GetDesc(&buffer_desc_1);
        var->GetDesc(&var_desc);
        cout << "Variable: " << var_desc.Name << endl;
        cout << endl << endl;
      }
    }

    auto bound_resource_count = desc.BoundResources;
    for (uint32_t i = 0; i < bound_resource_count; ++i) {
      D3D12_SHADER_INPUT_BIND_DESC bound_res_desc;
      pShaderReflection->GetResourceBindingDesc(i, &bound_res_desc);
      cout << "Resource: " << bound_res_desc.Name << endl;
      cout << "Type: " << bound_res_desc.Type << endl;
      cout << "BindPoint: " << bound_res_desc.BindPoint << endl;
      cout << "BindCount: " << bound_res_desc.BindCount << endl;
      cout << "uFlags: " << bound_res_desc.uFlags << endl;
      cout << "ReturnType: " << bound_res_desc.ReturnType << endl;
      cout << "Dimension: " << bound_res_desc.Dimension << endl;
      cout << "NumSamples: " << bound_res_desc.NumSamples << endl;
      cout << "Space: " << bound_res_desc.Space << endl;
      cout << "uID: " << bound_res_desc.uID << endl;
      cout << endl;
    }
  } else {
    cout << "FAILED: Cannot get reflection data!!!" << endl;
  }

  return 0;
}
