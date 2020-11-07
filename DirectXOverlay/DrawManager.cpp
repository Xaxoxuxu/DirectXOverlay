#include "DrawManager.h"

DrawManager::DrawManager(const std::string& windowToOverlayName)
{
    const HWND tempHandle{ FindWindow(nullptr, windowToOverlayName.c_str()) };
    if (!tempHandle)
    {
        throw std::exception("Cannot find target window");
    }
    this->m_targetWindowHwnd = tempHandle;
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

template<typename T>
T DrawManager::GetWindowProps(const HWND hWnd)
{
    if (!hWnd)
    {
        throw std::exception("Invalid window handle");
    }

    RECT client{}, window{};
    GetClientRect(hWnd, &client);
    GetWindowRect(hWnd, &window);

    POINT diff{};
    ClientToScreen(hWnd, &diff);

    return
    {
        window.left + (diff.x - window.left),
        window.top + (diff.y - window.top),
        client.right,
        client.bottom
    };
}

void DrawManager::Scale()
{
    static auto fix = [](long& in, uint32_t& out)
    {
        if (in == 0)
        {
            in++;
            out--;
        }
        else
        {
            in--;
            out++;
        }
    };

    RECT props{ GetWindowProps<RECT>(m_targetWindowHwnd) };

    m_overlayWidth = static_cast<uint32_t>(props.right);
    m_overlayHeight = static_cast<uint32_t>(props.bottom);

    fix(props.left, m_overlayWidth);
    fix(props.top, m_overlayHeight);

    MoveWindow(
        m_windowHandle,
        props.left,
        props.top,
        static_cast<int32_t>(m_overlayWidth),
        static_cast<int32_t>(m_overlayHeight),
        1
    );
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
        constexpr static std::size_t num_chars{ sizeof charset };

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

    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = className.c_str();

    RegisterClassEx(&wc);

    m_windowHandle = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        className.c_str(),
        windowName.c_str(),
        WS_POPUP,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        800,
        600,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr);

    Scale();

    SetLayeredWindowAttributes(m_windowHandle, 0, 255, LWA_ALPHA);
    // SetLayeredWindowAttributes(m_windowHandle, 0, RGB(0, 0, 0), LWA_COLORKEY);

    ShowWindow(m_windowHandle, SW_SHOW);

    InitD3D();
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

void DrawManager::InitD3D()
{
    // create a struct to hold information about the swap chain
    DXGI_SWAP_CHAIN_DESC scd;

    ZeroMemory(&scd, sizeof(scd));

    scd.BufferCount = 1;                                   // one back buffer
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;    // use 32-bit color
    scd.BufferDesc.Width = m_overlayWidth;                 // set the back buffer width
    scd.BufferDesc.Height = m_overlayHeight;               // set the back buffer height
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;     // how swap chain is to be used
    scd.OutputWindow = m_windowHandle;                     // the window to be used
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
    viewport.Width = static_cast<FLOAT>(m_overlayWidth);
    viewport.Height = static_cast<FLOAT>(m_overlayHeight);

    m_devCon->RSSetViewports(1, &viewport);

    InitPipeline();
}

// this is the function used to render a single frame
void DrawManager::RenderFrame()
{
    if (m_callbackFn)
    {
        m_devCon->ClearRenderTargetView(m_backBuffer, D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f));

        Scale();

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

    // select which primitive T we are using
    m_devCon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // draw the vertex buffer to the back buffer
    m_devCon->Draw(3, 0);
}

void DrawManager::DrawLine(const XMFLOAT2 pos1, const XMFLOAT2 pos2) const
{
    UINT viewportNumber = 1;
    D3D11_VIEWPORT vp;
    this->m_devCon->RSGetViewports(&viewportNumber, &vp);

    const float xx0 = 2.0f * (pos1.x - 0.5f) / vp.Width - 1.0f;
    const float yy0 = 1.0f - 2.0f * (pos1.y - 0.5f) / vp.Height;
    const float xx1 = 2.0f * (pos2.x - 0.5f) / vp.Width - 1.0f;
    const float yy1 = 1.0f - 2.0f * (pos2.y - 0.5f) / vp.Height;

    VERTEX vertices[2]
    {
        {xx0, yy0, 0.0f, D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f)},
        {xx1, yy1, 0.0f, D3DXCOLOR(0.0f, 1.0f, 1.0f, 1.0f)}
    };

    // copy the vertices into the buffer
    D3D11_MAPPED_SUBRESOURCE ms;
    m_devCon->Map(m_pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, vertices, sizeof(vertices));
    m_devCon->Unmap(m_pVBuffer, NULL);

    // select which primitive T we are using
    m_devCon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);

    // draw the vertex buffer to the back buffer
    m_devCon->Draw(2, 0);
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
