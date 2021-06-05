#include"d3dapp.h"


LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
        cha::g_directX->Release();
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

INT WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, INT)
{
    UNREFERENCED_PARAMETER(hInst);

    // Register the window class
    WNDCLASSEX wc =
    {
       sizeof(WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
       GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
       L"D3D Tutorial", NULL
    };
    RegisterClassEx(&wc);

    // Create the application's window
    HWND hWnd = CreateWindow(L"D3D Tutorial", L"D3D Tutorial 05: Textures",
        WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
        NULL, NULL, wc.hInstance, NULL);

    CustomVertex vertices[] =
    {
       {-1,1,1,0xffff0000},   // v0
       {1,1,1,0xffff0000},      // v1
       {1,1,-1,0xffff0000},   // v2
       {-1,1,-1,0xffff0000},   // v3

       {-1,-1,1,0xffff0000},   // v4
       {1,-1,1,0xffff0000},   // v5
       {1,-1,-1,0xffff0000},   // v6
       {-1,-1,-1,0xffff0000},   // v7
    };

    MyIndex indexes[] =
    {
       {0,1,2},{0,2,3},// 위
       {4,5,6},{4,7,6},// 아래
       {0,3,7},{0,7,4},// 왼
       {1,5,6},{1,6,2},// 오
       {3,2,6},{3,6,7},// 앞
       {0,4,5},{0,5,1},// 뒤
    };

    cha::g_directX = new cha::DirectX(hWnd);
    cha::DirectXOjb* cube = new cha::DirectXOjb(vertices, 8, indexes, 12);
    cha::g_directX->Regist(cube);

    // Initialize Direct3D
    if (SUCCEEDED(cha::g_directX->Init()))
    {
        // Create the scene geometry
        if (SUCCEEDED(cube->Init()))
        {
            // Show the window
            ShowWindow(hWnd, SW_SHOWDEFAULT);
            UpdateWindow(hWnd);

            // Enter the message loop
            MSG msg;
            ZeroMemory(&msg, sizeof(msg));
            while (msg.message != WM_QUIT)
            {
                if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
                {
                    cha::g_directX->UpdateOjbs();
                    cha::g_directX->Render();
                }
            }
        }
    }

    delete cube;
    delete cha::g_directX;

    UnregisterClass(L"D3D Tutorial", wc.hInstance);
    return 0;
}