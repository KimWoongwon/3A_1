#include "CD3DApp.h"

CD3DApp::CD3DApp()
{
    
}

CD3DApp::~CD3DApp()
{

}

HRESULT CD3DApp::InitD3D(HWND hWnd)
{
    m_hWnd = hWnd;
    // Create the D3D object.
    if (NULL == (m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
        return E_FAIL;

    // Set up the structure used to create the D3DDevice. Since we are now
    // using more complex geometry, we will create a device with a zbuffer.
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // Create the D3DDevice
    if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pD3Ddevice)))
    {
        return E_FAIL;
    }

    OnInit();
    // OnInit();
    //// Turn off culling
    //m_pD3Ddevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    //// Turn off D3D lighting
    //m_pD3Ddevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    //// Turn on the zbuffer
    //m_pD3Ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);

    return S_OK;
}
void CD3DApp::Render()
{
    if (m_pD3Ddevice == NULL)
        return;

    m_pD3Ddevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

    if (SUCCEEDED(m_pD3Ddevice->BeginScene()))
    {
        OnRender();
        m_pD3Ddevice->EndScene();
    }

    m_pD3Ddevice->Present(NULL, NULL, NULL, NULL);
}
void CD3DApp::Update()
{
    OnUpdate();
}
void CD3DApp::Cleanup()
{
    OnRelease();
    SAFE_RELEASE(m_pD3Ddevice);
    SAFE_RELEASE(m_pD3D);
}
