// include the basic windows header files and the Direct3D header files
#pragma once

#define NOMINMAX 
#include <windows.h>
#include <windowsx.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3DX10.h>
#include <functional>
#include <random>
#include <chrono>
#include <thread>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

// define the screen resolution
constexpr int SCREEN_WIDTH{ 800 };
constexpr int SCREEN_HEIGHT{ 600 };

using renderCallbackFn = std::function<void()>;

class DrawManager
{
private:
    IDXGISwapChain* m_swapChain = nullptr;             // the pointer to the swap chain interface
    ID3D11Device* m_dev = nullptr;                     // the pointer to our Direct3D device interface
    ID3D11DeviceContext* m_devCon = nullptr;           // the pointer to our Direct3D device context
    ID3D11RenderTargetView* m_backBuffer = nullptr;    // the pointer to our back buffer
    ID3D11InputLayout* m_pLayout = nullptr;            // the pointer to the input layout
    ID3D11VertexShader* m_pVS = nullptr;               // the pointer to the vertex shader
    ID3D11PixelShader* m_pPS = nullptr;                // the pointer to the pixel shader
    ID3D11Buffer* m_pVBuffer = nullptr;                // the pointer to the vertex buffer

    renderCallbackFn m_callbackFn;
    std::string m_windowToOverlayName;

    void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
    void RenderFrame() const;     // renders a single frame
    void CleanD3D() const;        // closes Direct3D and releases memory
    void InitPipeline();    // loads and prepares the shaders
    void InitWindow();  // initializes window and message loop

    // the WindowProc function prototype
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    DrawManager(const std::string& windowToOverlayName);

    // a struct to define a single vertex
    struct VERTEX { FLOAT x{}, y{}, z{}; D3DXCOLOR color; };

    void InitOverlay(const bool &terminate);
    void DrawTriangle(const VERTEX triangleVertices[3]) const;
    void SetCallback(renderCallbackFn callback);
};