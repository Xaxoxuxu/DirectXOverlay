// include the basic windows header files and the Direct3D header files
#pragma once

#define NOMINMAX // right before windows.h so we get rid of the annoying min max macros
#include <windows.h>
#include <windowsx.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3DX10.h>
#include <functional>
#include <chrono>
#include <thread>
#include <DirectXMath.h>
#include <D3DCompiler.h>
#include "Effects.h"
#include "Utils.h"
#include "XorStr.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

using renderCallbackFn = std::function<void()>;
using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;

class DrawManager
{
private:
    IDXGISwapChain* m_pSwapChain{ nullptr };             // the pointer to the swap chain interface
    ID3D11Device* m_pDev{ nullptr };                     // the pointer to our Direct3D device interface
    ID3D11DeviceContext* m_pDevCon{ nullptr };           // the pointer to our Direct3D device context
    ID3D11RenderTargetView* m_pBackBuffer{ nullptr };    // the pointer to our back buffer
    ID3D11InputLayout* m_pLayout{ nullptr };             // the pointer to the input layout
    ID3D11VertexShader* m_pVS{ nullptr };                // the pointer to the vertex shader
    ID3D11PixelShader* m_pPS{ nullptr };                 // the pointer to the pixel shader
    ID3D11Buffer* m_pVBuffer{ nullptr };                 // the pointer to the vertex buffer

    renderCallbackFn m_callbackFn{ nullptr };
    HWND m_windowHandle{ nullptr };
    HWND m_targetWindowHwnd{ nullptr };
    uint32_t m_overlayWidth{ 0 };
    uint32_t m_overlayHeight{ 0 };

    void InitD3D();               // sets up and initializes Direct3D
    void RenderFrame() const;     // renders a single frame
    void CleanD3D() const;        // closes Direct3D and releases memory
    void InitPipeline();          // loads and prepares the shaders
    void Scale();
    void InitWindow();            // initializes window and message loop
    template<typename T = RECT>
    static T GetWindowProps(HWND hWnd);

    // the WindowProc function prototype
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    explicit DrawManager(const std::string& windowToOverlayName);

    // a struct to define a single vertex
    struct VERTEX { FLOAT x{}, y{}, z{}; D3DXCOLOR color; };
    inline static D3DXCOLOR WHITE{ 1.0f, 1.0f, 1.0f, 1.0f };

    void InitOverlay(const bool& terminate);
    void DrawTriangle(const XMFLOAT2& point1, const XMFLOAT2& point2, const XMFLOAT2& point3, const D3DXCOLOR& col = WHITE) const;
    void DrawLine(const XMFLOAT2& point1, const XMFLOAT2& point2, const D3DXCOLOR& col = WHITE) const;
    void DrawBorderBox(const XMFLOAT2& topLeft, const XMFLOAT2& topRight, const XMFLOAT2& botRight, const XMFLOAT2& botLeft, const D3DXCOLOR& col = WHITE) const;
    void DrawCircle(const XMFLOAT2& centerPoint, int radius, const D3DXCOLOR& col = WHITE) const;
    void SetCallback(renderCallbackFn callback);
    void DrawLines(const XMFLOAT2 points[], const int& pointsCount, const D3DXCOLOR& col = WHITE) const;
};