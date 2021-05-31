#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <string>

#define S_DIAMETER 2
#define D_DIAMETER 100
#define SR_DIAMETER 0.05f
#define OR_DIAMETER 0.03f

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define KEY_DOWN(vk_code)((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code)((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;

enum Index { Sun = 0, Mercury, Venus, Earth, Moon, Mars, Jupiter, Saturn, Uranus, Neptune };
const float Scale[10] = { 5.0f, 0.38f, 0.95f, 1.00f, 0.25f, 0.53f, 4.21f, 3.45f, 1.51f, 1.38f };
const float Distance[10] = { 0.0f, 0.4f, 0.7f, 1.00f, 0.0256f, 1.2f, 1.6f, 2.1f, 2.8f, 3.7f };

const float Orbit_arr[10] = { 0.0f, 1.58f, 1.17f, 1.00f, 0.03f, 0.80f, 0.43f, 0.32f, 0.22f, 0.18f };
const float Spin_arr[10] = { 1.996f, 0.003f, 0.0018f, 0.4651f, 0.004626f, 0.2411f, 12.6f, 9.87f, 2.59f, 2.68f };

const char* texture_arr[10] = {"sunmap.jpg", "mercurymap.jpg", "venusmap.jpg", "earthmap.jpg", "moonmap.jpg", "marsmap.jpg", "jupitermap.jpg", "saturnmap.jpg", "uranusmap.jpg" , "neptunemap.jpg" };

LPD3DXMESH PlanetMesh[10];
D3DXMATRIX PlanetMatrix[10];
LPDIRECT3DTEXTURE9 PlanetTexture[10];

#define FVF_VERTEX  D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1

typedef struct TEXTURE_VERTEX
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 norm;
	float tu, tv;
} TVER, *LPTVER;

void initD3D(HWND hWnd);
void render_frame(void);
void SetupMatrices(void);
void SetupTexture(Index index);
void cleanD3D(void);
void init_graphics(void);
void init_light(void);

void Sun_Matrix_Process(void);
void Mercury_Matrix_Process(void);
void Venus_Matrix_Process(void);
void Earth_Matrix_Process(void);
void Moon_Matrix_Process(void);
void Mars_Matrix_Process(void);
void Jupiter_Matrix_Process(void);
void Saturn_Matrix_Process(void);
void Uranus_Matrix_Process(void);
void Neptune_Matrix_Process(void);

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc =
	{
		sizeof(WNDCLASSEX), CS_CLASSDC, (WNDPROC)WindowProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		"Final_Task_16032020", NULL
	};

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL, "Final_Task_16032020", "Final_Task_16032020", WS_OVERLAPPEDWINDOW,
		100, 100, 100 + SCREEN_WIDTH, 100 + SCREEN_HEIGHT, NULL, NULL, wc.hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);

	initD3D(hWnd);

	MSG msg;
	while (TRUE)
	{
		DWORD starting_point = GetTickCount();
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		render_frame();

		if (KEY_DOWN(VK_ESCAPE))
			PostMessage(hWnd, WM_DESTROY, 0, 0);

		while ((GetTickCount() - starting_point) < 25);
	}

	cleanD3D();

	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		
		}break;
		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);

}

void initD3D(HWND hWnd)
{
	if (NULL == (d3d = Direct3DCreate9(D3D_SDK_VERSION)))
		return;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferWidth = SCREEN_WIDTH;
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
		hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev)))
		return;

	init_graphics();
	init_light();

	d3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
	d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);
	//d3ddev->SetRenderState(D3DRS_AMBIENT, 0x00202020);
	
	return;
}

void render_frame(void)
{
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	if (SUCCEEDED(d3ddev->BeginScene()))
	{
		SetupMatrices();

		Sun_Matrix_Process();
		PlanetMesh[Index::Sun]->DrawSubset(0);
		
		
		D3DMATERIAL9 material;
		ZeroMemory(&material, sizeof(D3DMATERIAL9));
		material.Diffuse.r = material.Ambient.r = 0.3f;
		material.Diffuse.g = material.Ambient.g = 0.3f;
		material.Diffuse.b = material.Ambient.b = 0.3f;
		material.Diffuse.a = material.Ambient.a = 1.0f;

		d3ddev->SetMaterial(&material);

		
		
		Mercury_Matrix_Process();
		PlanetMesh[Index::Mercury]->DrawSubset(0);

		Venus_Matrix_Process();
		PlanetMesh[Index::Venus]->DrawSubset(0);

		Earth_Matrix_Process();
		PlanetMesh[Index::Earth]->DrawSubset(0);

		Moon_Matrix_Process();
		PlanetMesh[Index::Moon]->DrawSubset(0);

		Mars_Matrix_Process();
		PlanetMesh[Index::Mars]->DrawSubset(0);

		Jupiter_Matrix_Process();
		PlanetMesh[Index::Mars]->DrawSubset(0);

		Saturn_Matrix_Process();
		PlanetMesh[Index::Saturn]->DrawSubset(0);

		Uranus_Matrix_Process();
		PlanetMesh[Index::Uranus]->DrawSubset(0);

		Neptune_Matrix_Process();
		PlanetMesh[Index::Neptune]->DrawSubset(0);

	
		d3ddev->EndScene();
	}
	
	d3ddev->Present(NULL, NULL, NULL, NULL);

	return;
}

void SetupMatrices(void)
{
	D3DXVECTOR3 vEyePt(0.0f, 300.0f, -300.0f);
	D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
	d3ddev->SetTransform(D3DTS_VIEW, &matView);

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DXToRadian(45), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 1.0f, 10000.0f);
	d3ddev->SetTransform(D3DTS_PROJECTION, &matProj);

}

void SetupTexture(Index index)
{
	d3ddev->SetTexture(0, PlanetTexture[index]);
	d3ddev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	d3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	d3ddev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
}

void cleanD3D(void)
{
	for (int i = 0; i < 10; i++)
	{
		PlanetMesh[i]->Release();
	}

	d3ddev->Release();
	d3d->Release();

	return;
}

void init_graphics(void)
{
	for (int i = 0; i < 10; i++)
	{
		D3DXCreateSphere(d3ddev, 1.0f, 50, 50, &PlanetMesh[i], NULL);

		if (FAILED(D3DXCreateTextureFromFile(d3ddev, texture_arr[i], &PlanetTexture[i])))
		{
			char temp[30] = "..\\";
			strcat(temp, texture_arr[i]);
			// If texture is not in current folder, try parent folder
			if (FAILED(D3DXCreateTextureFromFile(d3ddev, temp, &PlanetTexture[i])))
			{
				MessageBox(NULL, "Could not find Image.bmp", "Textures.exe", MB_OK);
				return;
			}
		}

		LPD3DXMESH texMesh;
		if (FAILED(PlanetMesh[i]->CloneMeshFVF(D3DXMESH_SYSTEMMEM, FVF_VERTEX, d3ddev, &texMesh)))
			return;

		PlanetMesh[i]->Release();

		LPTVER vts;
		if (SUCCEEDED(texMesh->LockVertexBuffer(0, (void**)&vts)))
		{
			int num = texMesh->GetNumVertices();
			for (int i = 0; i < num; i++)
			{
				vts->tu = asinf(vts->norm.x) / D3DX_PI + 0.5f;
				vts->tv = asinf(vts->norm.y) / D3DX_PI + 0.5f;
				vts++;
			}
			texMesh->UnlockVertexBuffer();

			PlanetMesh[i] = texMesh;
		}
	}

	return;
}

void init_light(void)
{
	D3DLIGHT9 light, light2, light3, light4;
	D3DMATERIAL9 material;

	ZeroMemory(&light, sizeof(light));
	light.Type = D3DLIGHT_POINT;
	light.Diffuse.r = 0.3f;
	light.Diffuse.g = 0.3f;
	light.Diffuse.b = 0.3f;
	light.Attenuation0 = 0.0001f;
	light.Range = 5000.0f;
	light.Position.x = 0.0;
	light.Position.y = 0.0;
	light.Position.z = 0.0;
	
	d3ddev->SetLight(0, &light);
	d3ddev->LightEnable(0, TRUE);

	ZeroMemory(&material, sizeof(D3DMATERIAL9));
	material.Diffuse.r = material.Ambient.r = 0.3f;
	material.Diffuse.g = material.Ambient.g = 0.3f;
	material.Diffuse.b = material.Ambient.b = 0.3f;
	material.Diffuse.a = material.Ambient.a = 1.0f;

	d3ddev->SetMaterial(&material);

	return;
}

#pragma region Planet_Process_Function

void Sun_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Sun] * SR_DIAMETER;
	D3DXMATRIX mScale;
	D3DXMATRIX mSpinRotation;
	D3DMATERIAL9 material;

	float _Scale = Scale[Index::Sun] * S_DIAMETER;

	ZeroMemory(&material, sizeof(D3DMATERIAL9));
	material.Emissive.r = 1.0f;
	material.Emissive.g = 1.0f;
	material.Emissive.b = 1.0f;
	material.Emissive.a = 1.0f;
	material.Power = 100.0f;

	d3ddev->SetMaterial(&material);

	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);

	PlanetMatrix[Index::Sun] = mScale * mSpinRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Sun]);
	SetupTexture(Index::Sun);
}

void Mercury_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Mercury] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Mercury] * OR_DIAMETER;

	float _Scale = Scale[Index::Mercury] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Mercury] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Mercury] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Mercury]);
	SetupTexture(Index::Mercury);
}
void Venus_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Venus] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Venus] * OR_DIAMETER;

	float _Scale = Scale[Index::Venus] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Venus] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Venus] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Venus]);
	SetupTexture(Index::Venus);
}

void Earth_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Earth] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Earth] * OR_DIAMETER;

	float _Scale = Scale[Index::Earth] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Earth] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Earth] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Earth]);
	SetupTexture(Index::Earth);
}
void Moon_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Moon] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Moon] * OR_DIAMETER;

	float _Scale = Scale[Index::Moon] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Moon] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Moon] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation * PlanetMatrix[Index::Earth];
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Moon]);
	SetupTexture(Index::Moon);
}
void Mars_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Mars] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Mars] * OR_DIAMETER;

	float _Scale = Scale[Index::Mars] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Mars] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Mars] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Mars]);
	SetupTexture(Index::Mars);
}
void Jupiter_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Jupiter] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Jupiter] * OR_DIAMETER;

	float _Scale = Scale[Index::Jupiter] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Jupiter] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Jupiter] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Jupiter]);
	SetupTexture(Index::Jupiter);
}
void Saturn_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Saturn] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Saturn] * OR_DIAMETER;

	float _Scale = Scale[Index::Saturn] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Saturn] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Saturn] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Saturn]);
	SetupTexture(Index::Saturn);
}
void Uranus_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Uranus] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Uranus] * OR_DIAMETER;

	float _Scale = Scale[Index::Uranus] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Uranus] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Uranus] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Uranus]);
	SetupTexture(Index::Uranus);
}
void Neptune_Matrix_Process(void)
{
	static float Spin_Angle = 0.0f; Spin_Angle += Spin_arr[Index::Neptune] * SR_DIAMETER;
	static float Orbit_index = 0.0f; Orbit_index += Orbit_arr[Index::Neptune] * OR_DIAMETER;

	float _Scale = Scale[Index::Neptune] * S_DIAMETER;
	D3DXMATRIX mScale, mSpinRotation, mTranslationToOrbit, mOrbitRotation;


	D3DXMatrixScaling(&mScale, _Scale, _Scale, _Scale);
	D3DXMatrixRotationY(&mSpinRotation, Spin_Angle);
	D3DXMatrixTranslation(&mTranslationToOrbit, 0.0f, 0.0f, Distance[Index::Neptune] * D_DIAMETER);
	D3DXMatrixRotationY(&mOrbitRotation, Orbit_index);

	PlanetMatrix[Index::Neptune] = mScale * mSpinRotation * mTranslationToOrbit * mOrbitRotation;
	d3ddev->SetTransform(D3DTS_WORLD, &PlanetMatrix[Index::Neptune]);
	SetupTexture(Index::Neptune);
}
#pragma endregion

