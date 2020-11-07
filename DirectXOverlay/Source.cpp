#include <thread>
#include <conio.h>
#include "DrawManager.h"

XMFLOAT2 pos1{ 100.0f, 100.0f }, pos2{ 200.0f, 200.0f };

void InitDraw(const bool& terminate)
{
    DrawManager draw{ "Task Manager" };

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

    draw.SetCallback([&]
        {
            //draw.DrawTriangle(triangleVertices);
            //draw.DrawTriangle(triangleVertices2);
            draw.DrawLine(pos1, pos2);
        });

    draw.InitOverlay(terminate);
}

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
int main()
{
    bool terminate{ false };
    std::thread drawThread(InitDraw, std::cref(terminate));

    int c = 0;
    while (true)
    {
        c = 0;

        switch ((c = _getch())) {
        case KEY_UP:
            pos1.y += 10.0f;
            pos2.y += 10.0f;
            break;
        case KEY_DOWN:
            pos1.y -= 10.0f;
            pos2.y -= 10.0f;
            break;
        case KEY_LEFT:
            pos1.x -= 10.0f;
            pos2.x -= 10.0f;
            break;
        case KEY_RIGHT:
            pos1.x += 10.0f;
            pos2.x += 10.0f;
            break;
        default:

            break;
        }

        printf("pos1[%f;%f] - pos2[%f;%f]\n", pos1.x, pos1.y, pos2.x, pos2.y);
    }

    terminate = true;

    drawThread.join();

    return 0;
}