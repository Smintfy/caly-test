#include "include/raylib.h"
#include <stdbool.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_FPS 60
#define FRAMES_SPEED 6
#define PLAYER_SPEED 4

typedef enum
{
    DOWN,
    UP,
    RIGHT,
    LEFT
} PlayerSequence;

typedef struct Player
{
	Vector2 position;
	Texture2D texture;

	// Animation related
	Rectangle frameRect;
	int currentFrame;
	int currentSequence;
	int framesCounter;

	// helper
	float frame_width;
	float frame_height;
} Player;

Player InitPlayer()
{
	Player player;

	player.texture = LoadTexture("assets/acly.png");

	player.frame_width = (float)player.texture.width / 3;      // The width of each individual sprite
	player.frame_height = (float)player.texture.height / 4;    // The height of each individual sprite

	float player_x_center = (float)(SCREEN_WIDTH - player.frame_width) / 2;
	float player_y_center = (float)(SCREEN_HEIGHT - player.frame_height) / 2;
	player.position = (Vector2){player_x_center, player_y_center};

	player.frameRect = (Rectangle){0.0f, 0.0f, player.frame_width, player.frame_height};
	player.framesCounter = 0;
	player.currentSequence = 0;
	player.currentFrame = 0;

	return player;
}

void UnloadPlayer(Player *player)
{
    UnloadTexture(player->texture);
}

void UpdatePlayer(Player *player)
{
    Vector2 playerDirection = {0.0f, 0.0f};
    bool isMoving = false;

    player->frameRect.x = 0.0f;
    player->framesCounter++;

    // Limit of the player animation frame update
    if (player->framesCounter >= (MAX_FPS / FRAMES_SPEED))
    {
        player->framesCounter = 0;
        player->currentFrame++;

        if (player->currentFrame > 2) player->currentFrame = 0;
    }

    if (IsKeyDown(KEY_W))
    {
        playerDirection.y = -1.0f;
        player->currentSequence = UP;
        isMoving = true;
    }

    if (IsKeyDown(KEY_S))
    {
        playerDirection.y = 1.0f;
        player->currentSequence = DOWN;
        isMoving = true;
    };

    // Prevent diagonal movement
    if (playerDirection.y == 0.0f)
    {
        if (IsKeyDown(KEY_A))
        {
            playerDirection.x = -1.0f;
            player->currentSequence = LEFT;
            isMoving = true;
        }
        else if (IsKeyDown(KEY_D))
        {
            playerDirection.x = 1.0f;
            player->currentSequence = RIGHT;
            isMoving = true;
        };
    }

    player->position.x += playerDirection.x * PLAYER_SPEED;
    player->position.y += playerDirection.y * PLAYER_SPEED;

    player->frameRect.y = player->currentSequence * player->frame_height;
    player->frameRect.x = (isMoving ? player->currentFrame : 0) * player->frame_width;
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "callie - test");

    Player player = InitPlayer();
    Texture2D floor = LoadTexture("assets/Floor.png");

    Camera2D camera = { 0 };
    camera.target = (Vector2){
        player.position.x + (float)player.frame_width / 2,
        player.position.y + (float)player.frame_height / 2
    };
    camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f}; // Center of the screen
    camera.zoom = 1.0f; // Default zoom level

    SetTargetFPS(60);

    while(!WindowShouldClose())
    {
        UpdatePlayer(&player);

        camera.target = (Vector2){
            player.position.x + (float)player.frame_width / 2,
            player.position.y + (float)player.frame_height / 2
        };

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);
            DrawTextureRec(
                floor,
                (Rectangle){0.0f, 0.0f, floor.width, floor.height},
                (Vector2){(float)(SCREEN_WIDTH - floor.width) / 2, (float)(SCREEN_HEIGHT - floor.height) / 2},
                WHITE
            );
            DrawTextureRec(player.texture, player.frameRect, player.position, WHITE);
        EndMode2D();

        DrawText(TextFormat("FPS: %d", GetFPS()), 16, 16, 16, WHITE);

        EndDrawing();
    }

    UnloadTexture(floor);
    UnloadPlayer(&player);
    CloseWindow();

    return 0;
}
