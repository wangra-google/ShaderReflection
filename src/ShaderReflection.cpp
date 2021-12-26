// ShaderReflection.cpp : Defines the entry point for the application.
//

#include <wrl.h>
#include "dxcapi.h"
#include "ShaderReflection.h"

using namespace std;
using Microsoft::WRL::ComPtr;

int main()
{
	ComPtr<IDxcUtils> pUtils;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(pUtils.GetAddressOf()));

	/*IDxcUtils* pUtils;
	DxcCreateInstance(CLSID_DxcUtils, __uuidof(IDxcUtils), reinterpret_cast<void**>(&pUtils));*/

	/*IDxcLibrary* dxcLib;
	if (DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary),
		reinterpret_cast<void**>(&dxcLib)) != S_OK)
	{
		return E_FAIL;
	}*/

	cout << "Hello ShaderReflection!" << endl;
	return 0;
}
