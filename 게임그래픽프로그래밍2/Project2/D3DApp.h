#pragma once
#include <d3d9.h>

class CD3DApp
{
protected:
	LPDIRECT3D9 m_pD3D;
	LPDIRECT3DDEVICE9 m_pD3DDevice;
	HWND m_hWnd;

protected:
	virtual void OnD3DInit() = 0;
	virtual void OnRender() = 0;
	virtual void OnCleanUp() = 0;
	virtual void OnUpdate() = 0;

public:
	HRESULT InitD3D(HWND);
	void Render();
	void Update();
	void CleanUp();
	
	CD3DApp();
	~CD3DApp();
};



