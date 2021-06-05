#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#define SAFE_RELEASE(p) if(p != NULL) (p)->Release()

// Base Class of Game. Setup DX
class CD3DApp
{
protected:
	// dx ��ü, device, ������ �ڵ�
	LPDIRECT3D9 m_pD3D;
	LPDIRECT3DDEVICE9 m_pD3Ddevice;
	HWND m_hWnd;
protected:
	//[Virtual Section]
	// init, render, update, release ==> �������̵� �� �� �ְ�
	virtual void OnInit() = 0;
	virtual void OnRender() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRelease() = 0;
public:
	// InitD3D, Render, Update, Cleanup
	// ������, �Ҹ���
	HRESULT InitD3D(HWND);
	void Render();
	void Update();
	void Cleanup();

	CD3DApp();
	~CD3DApp();
	
};

