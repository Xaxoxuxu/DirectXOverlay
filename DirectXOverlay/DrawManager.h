// include the basic windows header files and the Direct3D header files
#pragma once
#include <windows.h>
#include <windowsx.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3DX10.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")
#pragma comment (lib, "d3dx10.lib")

// define the screen resolution
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

class DrawManager
{
private:
    // global declarations
    IDXGISwapChain* swapchain;             // the pointer to the swap chain interface
    ID3D11Device* dev;                     // the pointer to our Direct3D device interface
    ID3D11DeviceContext* devcon;           // the pointer to our Direct3D device context
    ID3D11RenderTargetView* backbuffer;    // the pointer to our back buffer
    ID3D11InputLayout* pLayout;            // the pointer to the input layout
    ID3D11VertexShader* pVS;               // the pointer to the vertex shader
    ID3D11PixelShader* pPS;                // the pointer to the pixel shader
    ID3D11Buffer* pVBuffer;                // the pointer to the vertex buffer

    // a struct to define a single vertex
    struct VERTEX { FLOAT X, Y, Z; D3DXCOLOR Color; };

public:
    // function prototypes
    void InitD3D(HWND hWnd);    // sets up and initializes Direct3D
    void RenderFrame() const;     // renders a single frame
    void CleanD3D() const;        // closes Direct3D and releases memory
    void InitGraphics() const;    // creates the shape to render
    void InitPipeline();    // loads and prepares the shaders
    void InitOverlay();

    // the WindowProc function prototype
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};