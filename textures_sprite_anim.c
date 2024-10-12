#include "include/raylib.h"
#include "include/raymath.h"

#define SCREEN_WIDTH 620
#define SCREEN_HEIGHT 620
#define MIN_FRAME_SPEED 1
#define MAX_FRAME_SPEED 15

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "callie - sprite anim");

    float aclySpeed = 4.0f;

    Image acly_img = LoadImage("assets/acly.png");
    ImageResize(&acly_img, acly_img.width * 0.25, acly_img.height * 0.25); // It was too big...
    Texture2D acly = LoadTextureFromImage(acly_img);
    UnloadImage(acly_img);

    float acly_rec_width = (float)acly.width / 4; // The width of each individual sprite

    Rectangle frameRect = { 0.0f, 0.0f, acly_rec_width, (float)acly.height };
    int currentFrame = 0;

    // Centerize the la creatura
    float acly_x_center = (float)(SCREEN_WIDTH - acly_rec_width) / 2;
    float acly_y_center = (float)(SCREEN_HEIGHT - acly.height) / 2;
    Vector2 acly_pos = {acly_x_center, acly_y_center};

    SetTargetFPS(60);

    while(!WindowShouldClose())
    {
        Vector2 moveDir = { 0.0f, 0.0f };

        if (IsKeyDown(KEY_W))
        {
            moveDir.y = -1.0f;
            frameRect.x = 1 * acly_rec_width;
        }
        else if (IsKeyDown(KEY_S))
        {
             moveDir.y = 1.0f;
             frameRect.x = 0.0f;
        };

        if (moveDir.y == 0.0f) {
            if (IsKeyDown(KEY_A))
            {
                moveDir.x = -1.0f;
                frameRect.x = 3 * acly_rec_width;
            }
            else if (IsKeyDown(KEY_D))
            {
                moveDir.x = 1.0f;
                frameRect.x = 2 * acly_rec_width;
            };
        }

        acly_pos.x += moveDir.x * aclySpeed;
        acly_pos.y += moveDir.y * aclySpeed;

        BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Hello, World!", 16, 16, 16, WHITE);
            DrawTextureRec(acly, frameRect, acly_pos, WHITE);
        EndDrawing();
    }

    UnloadTexture(acly);
    CloseWindow();
    return 0;
}
