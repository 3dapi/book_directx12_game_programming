//***************************************************************************************
// MainApp.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "MainApp.h"

#if 0
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR cmdLine, int showCmd)
{
#else
int main(int, char**)
{
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);
#endif
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        auto appMain = IG2AppFrameWin::instance();
        auto hr = appMain->init(hInstance);
        if(FAILED(hr))
            return 0;

        return appMain->Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

