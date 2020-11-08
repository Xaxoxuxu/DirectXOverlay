#include <thread>
#include <conio.h>
#include "../DirectXOverlay/DrawManager.h"

XMFLOAT2 pos1{ 100.0f, 100.0f }, pos2{ 200.0f, 200.0f };

void InitDraw(const bool& terminate)
{
    DrawManager draw{ "League of Legends (TM) Client" };

    XMFLOAT2 posT1{ 300.0f, 300.0f };
    XMFLOAT2 posT3{ posT1.x - 200.0f, 500.0f };
    XMFLOAT2 posT2{ posT1.x + 200.0f, 500.0f };

    XMFLOAT2 posB1{ 500.0f, 500.0f };
    XMFLOAT2 posB2{ 500.0f + 100.0f, 500.0f };
    XMFLOAT2 posB3{ 500.0f + 100.0f, 500.0f + 100.0f };
    XMFLOAT2 posB4{ 500.0f, 500.0f + 100.0f };

    draw.SetCallback([&]
        {
            draw.DrawTriangle(posT1, posT2, posT3);
            draw.DrawLine(pos1, pos2);
            draw.DrawBorderBox(posB1, posB2, posB3, posB4);
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

        printf("pos1[%f:%f] - pos2[%f:%f]\n", pos1.x, pos1.y, pos2.x, pos2.y);
    }

    terminate = true;

    drawThread.join();

    return 0;
}