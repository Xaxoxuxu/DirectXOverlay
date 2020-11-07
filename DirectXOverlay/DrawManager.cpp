#include "DrawManager.h"

DrawManager::DrawManager(const std::string &windowToOverlayName)
{
    this->m_windowToOverlayName = windowToOverlayName;
}

void DrawManager::InitOverlay(const bool& terminate)
{
    InitWindow();

    MSG msg;
    while (!terminate)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }

        RenderFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    CleanD3D();
}

void DrawManager::InitWindow()
{
    const auto randomString = [](const std::size_t length)
    {
        constexpr static char charset[]
        {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
        };
        constexpr static std::size_t num_chars = sizeof charset;

        static std::random_device rd;
        std::mt19937 gen(rd());
        const std::uniform_int_distribution<std::size_t> engine(std::numeric_limits<std::size_t>::min(), num_chars - 1);

        std::string str(length, '\0');
        for (auto& c : str)
        {
            c = charset[engine(gen)];
        }

        return str;
    };

    const std::string className{ randomString(32) };
    const std::string windowName{ randomString(64) };

    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = className.c_str();

    RegisterClassEx(&wc);

    RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED,
        className.c_str(),
        windowName.c_str(),
        WS_POPUP,
        300,
        300,
        wr.right - wr.left,
        wr.bottom - wr.top,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr);

    SetLayeredWindowAttributes(hWnd, 0, 0, LWA_ALPHA);
    SetLayeredWindowAttributes(hWnd, 0, RGB(0, 0, 0), LWA_COLORKEY);
    ShowWindow(hWnd, SW_SHOW);

    InitD3D(hWnd);
}

LRESULT CALLBACK DrawManager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    } break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void DrawManager::InitD3D(HWND hWnd)
{
    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    ZeroMemory(&scd, sizeof(scd));

    scd.BufferCount = 1;                                   // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
    scd.BufferDesc.Width = SCREEN_WIDTH;                   // set the back buffer width
    scd.BufferDesc.Height = SCREEN_HEIGHT;                 // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // how swap chain is to be used
    scd.OutputWindow = hWnd;                               // the window to be used
    scd.SampleDesc.Count = 4;                              // how many multisamples
    scd.Windowed = TRUE;                                   // windowed/full-screen mode
    scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;    // allow full-screen switching

    // create a device, device context and swap chain using the information in the scd struct
    D3D11CreateDeviceAndSwapChain(nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        NULL,
        nullptr,
        NULL,
        D3D11_SDK_VERSION,
        &scd,
        &m_swapChain,
        &m_dev,
        nullptr,
        &m_devCon);

    // get the address of the back buffer
    ID3D11Texture2D* pBackBuffer;
    m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));

    // use the back buffer address to create the render target
    m_dev->CreateRenderTargetView(pBackBuffer, nullptr, &m_backBuffer);
    pBackBuffer->Release();

    // set the render target as the back buffer
    m_devCon->OMSetRenderTargets(1, &m_backBuffer, nullptr);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = SCREEN_WIDTH;
    viewport.Height = SCREEN_HEIGHT;

    m_devCon->RSSetViewports(1, &viewport);

    InitPipeline();
}

// this is the function used to render a single frame
void DrawManager::RenderFrame() const
{
    if (m_callbackFn)
    {
        m_callbackFn();

        // switch the back buffer and the front buffer
        m_swapChain->Present(0, 0);
    }
}

// this is the function that cleans up Direct3D and COM
void DrawManager::CleanD3D() const
{
    m_swapChain->SetFullscreenState(FALSE, nullptr);    // switch to windowed mode

    // close and release all existing COM objects
    m_pLayout->Release();
    m_pVS->Release();
    m_pPS->Release();
    m_pVBuffer->Release();
    m_swapChain->Release();
    m_backBuffer->Release();
    m_dev->Release();
    m_devCon->Release();
}

// TODO: x y z vector, we dont care about the color so fuck those vertices
void DrawManager::DrawTriangle(const VERTEX triangleVertices[3]) const
{
    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    m_devCon->Map(m_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, triangleVertices, sizeof(VERTEX) * 3);
    m_devCon->Unmap(m_pVBuffer, NULL);

    // select which primitive type we are using
    m_devCon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw the vertex buffer to the back buffer
    m_devCon->Draw(3, 0);
}

void DrawManager::SetCallback(renderCallbackFn callback)
{
    if (callback)
    {
        m_callbackFn = std::move(callback);
    }
}

// this function loads and prepares the shaders
void DrawManager::InitPipeline()
{
    // load and compile the two shaders
    ID3D10Blob* VS, * PS;
    D3DX11CompileFromFile("shaders.shader", nullptr, nullptr, "VShader", "vs_4_0", 0, 0, nullptr, &VS, nullptr, nullptr);
    D3DX11CompileFromFile("shaders.shader", nullptr, nullptr, "PShader", "ps_4_0", 0, 0, nullptr, &PS, nullptr, nullptr);

    // encapsulate both shaders into shader objects
    m_dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), nullptr, &m_pVS);
    m_dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), nullptr, &m_pPS);

    // set the shader objects
    m_devCon->VSSetShader(m_pVS, nullptr, 0);
    m_devCon->PSSetShader(m_pPS, nullptr, 0);

    // create the input layout object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    m_dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &m_pLayout);
    m_devCon->IASetInputLayout(m_pLayout);

    // create the vertex buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));

    bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
    bd.ByteWidth = sizeof(VERTEX) * 300;           // size is the VERTEX struct * 300?
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

    m_dev->CreateBuffer(&bd, nullptr, &m_pVBuffer);       // create the buffer

    // select which vertex buffer to display
    UINT stride = sizeof(VERTEX);
    UINT offset = 0;
    m_devCon->IASetVertexBuffers(0, 1, &m_pVBuffer, &stride, &offset);
}
