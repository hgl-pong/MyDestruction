#include "Application.h"
#include <iostream>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE prevInstance,
                   _In_ LPSTR cmdLine, _In_ int showCmd)
{
    // 这些参数不使用
	//UNREFERENCED_PARAMETER(prevInstance);
	//UNREFERENCED_PARAMETER(cmdLine);
	//UNREFERENCED_PARAMETER(showCmd);
    // 允许在Debug版本进行运行时内存分配和泄漏检测
//#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	AllocConsole();  //create console
	SetConsoleTitle(L"Destruction Editor"); //set console title   
	FILE* tempFile = nullptr;
	freopen_s(&tempFile, "conin$", "r+t", stdin); //reopen the stdin, we can use std::cout.
	freopen_s(&tempFile, "conout$", "w+t", stdout);
//#endif

    Application theApp(hInstance);
    if( !theApp.Init() )
        return 0;
    
    return theApp.Run();
}




