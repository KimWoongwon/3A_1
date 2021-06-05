#pragma once

#pragma comment(lib,"d3dx9")
#pragma comment(lib,"d3d9")
#pragma comment(lib,"winmm.lib")

#include <Windows.h>
#include <mmsystem.h>
#include <d3dx9.h>
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

#include<list>

struct CustomVertex
{
    float x, y, z;      // The position
    D3DCOLOR color;      // The color   
};

struct MyIndex
{
    WORD a, b, c;
};



namespace cha
{
    class IFunc
    {
    public:
        virtual HRESULT Init() = 0;
        virtual void Release() = 0;
    };

    class IObjFunc
    {
    public:
        virtual void Update() = 0;
        virtual void Render() = 0;
    };

    class DirectXOjb;

    class DirectX : public IFunc
    {
    public:
        std::list< IObjFunc*> m_obj_list;

        HWND hWnd;
        LPDIRECT3D9 m_d3d;
        LPDIRECT3DDEVICE9 m_device;
    public:
        DirectX(HWND hWnd)
            : hWnd(hWnd), m_d3d(nullptr), m_device(nullptr)
        {
            // init // throw
        }
        ~DirectX()
        {
            Release();
        }

        HRESULT Init() override
        {
            if (NULL == (m_d3d = Direct3DCreate9(D3D_SDK_VERSION)))
            {
                return E_FAIL;
            }

            D3DPRESENT_PARAMETERS d3dpp;
            ZeroMemory(&d3dpp, sizeof(d3dpp));
            d3dpp.Windowed = TRUE;
            d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
            d3dpp.EnableAutoDepthStencil = TRUE;
            d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

            if (FAILED(m_d3d->CreateDevice(D3DADAPTER_DEFAULT,
                D3DDEVTYPE_HAL,
                hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_device)))
            {
                return E_FAIL;
            }

            m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
            m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
            m_device->SetRenderState(D3DRS_ZENABLE, TRUE);

            return S_OK;
        }

        void UpdateOjbs()
        {
            for (const auto& item : m_obj_list)
            {
                item->Update();
            }
        }

        void Release() override
        {
            if (m_d3d != nullptr)
                m_d3d->Release();
            if (m_device != nullptr)
                m_device->Release();
        }

        void SetUpMatrix()
        {
            D3DXMATRIXA16 matWorld;
            D3DXMatrixIdentity(&matWorld);
            D3DXMatrixRotationX(&matWorld, timeGetTime() / 1000.0f);
            m_device->SetTransform(D3DTS_WORLD, &matWorld);

            D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);
            D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
            D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
            D3DXMATRIXA16 matView;
            D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);
            m_device->SetTransform(D3DTS_VIEW, &matView);

            D3DXMATRIXA16 matProj;
            D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
            m_device->SetTransform(D3DTS_PROJECTION, &matProj);
        }

        void Regist(IObjFunc* obj)
        {
            m_obj_list.push_back(obj);
        }

        void Render()
        {
            m_device->Clear(0, NULL,
                D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                D3DCOLOR_XRGB(0, 0, 255), 1.0f, 0);

            SetUpMatrix();

            for (const auto& item : m_obj_list)
            {
                item->Render();
            }

            m_device->Present(NULL, NULL, NULL, NULL);
        }
    };


    DirectX* g_directX;

    class DirectXOjb : public IFunc, public IObjFunc
    {
    private:
        LPDIRECT3DVERTEXBUFFER9 m_pVB;
        LPDIRECT3DINDEXBUFFER9 m_pIB;

        CustomVertex* m_vertexes;
        UINT         m_vertex_count;
        MyIndex* m_indexes;
        UINT         m_index_count;

    public:
        DirectXOjb()
            : m_pVB(nullptr), m_pIB(nullptr),
            m_vertexes(nullptr), m_vertex_count(0),
            m_indexes(nullptr), m_index_count(0) {}

        DirectXOjb(CustomVertex* _vertexes, UINT _vertex_count,
            MyIndex* _indexes, UINT _index_count)
            : m_pVB(nullptr), m_pIB(nullptr),
            m_vertexes(_vertexes), m_vertex_count(_vertex_count),
            m_indexes(_indexes), m_index_count(_index_count)
        { }

        HRESULT Init() override
        {
            // create vertex buffer
            if (FAILED(g_directX->m_device->CreateVertexBuffer(m_vertex_count * sizeof(CustomVertex), 0,
                D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pVB, NULL)))
                return E_FAIL;

            // create index buffer
            if (FAILED(g_directX->m_device->CreateIndexBuffer(m_vertex_count * sizeof(CustomVertex), 0,
                D3DFMT_INDEX16, D3DPOOL_DEFAULT, &m_pIB, NULL)))
                return E_FAIL;

            // copy vertexes
            CustomVertex* pVertices;
            if (FAILED(m_pVB->Lock(0, 0, (void**)&pVertices, 0)))
                return E_FAIL;
            memcpy(pVertices, m_vertexes, sizeof(CustomVertex) * m_vertex_count);
            m_pVB->Unlock();

            // copy indexes
            MyIndex* pIndexes;
            if (FAILED(m_pIB->Lock(0, 0, (void**)&pIndexes, 0)))
                return E_FAIL;

            memcpy(pIndexes, m_indexes, sizeof(MyIndex) * m_index_count);

            m_pIB->Unlock();
        }

        void Release() override
        {
            if (m_pIB != NULL)
                m_pIB->Release();
            if (m_pVB != nullptr)
                m_pVB->Release();
        }

        void Update() override
        {
            // set transform
        }

        void Render() override
        {
            if (SUCCEEDED(g_directX->m_device->BeginScene()))
            {
                g_directX->m_device->SetStreamSource(0, m_pVB, 0, sizeof(CustomVertex));
                g_directX->m_device->SetFVF(D3DFVF_CUSTOMVERTEX);

                g_directX->m_device->SetIndices(m_pIB);
                g_directX->m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_vertex_count, 0, m_index_count);

                g_directX->m_device->EndScene();
            }
        }
    };
}