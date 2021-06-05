#include "Game.h"

void CGame::OnInit()
{
    m_pD3Ddevice->SetRenderState(D3DRS_LIGHTING, TRUE);    // turn on the 3D lighting
    m_pD3Ddevice->SetRenderState(D3DRS_ZENABLE, TRUE);    // turn on the z-buffer
    m_pD3Ddevice->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50, 50, 50));    // ambient light
    m_pD3Ddevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
}
void CGame::OnRender()
{

}
void CGame::OnUpdate()
{

}
void CGame::OnRelease()
{

}

CGame::CGame()
{

}
CGame::~CGame()
{

}