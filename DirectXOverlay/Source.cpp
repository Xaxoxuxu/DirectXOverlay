#include <thread>

#include "DrawManager.h"


void InitDraw(const bool &terminate)
{
    DrawManager draw{"Task Manager"};

    const DrawManager::VERTEX triangleVertices[3]
    {
        {0.0f, 0.5f, 0.0f, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
        {0.45f, -0.5f, 0.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
        {-0.45f, -0.5f, 0.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)}
    };
    const DrawManager::VERTEX triangleVertices2[3]
    {
        {0.0f, 1.0f, 0.0f, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f)},
        {0.45f, -0.95f, 0.0f, D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f)},
        {-0.45f, -0.95f, 0.0f, D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f)}
    };

    draw.SetCallback([&draw, &triangleVertices, &triangleVertices2]
        {
            draw.DrawTriangle(triangleVertices);
            draw.DrawTriangle(triangleVertices2);
        });

    draw.InitOverlay(terminate);
}

int main()
{
    bool terminate{ false };
    std::thread drawThread(InitDraw, std::cref(terminate));

    getchar();

    terminate = true;

    drawThread.join();

    return 0;
}