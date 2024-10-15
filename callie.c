#include "include/raylib.h"
#include <stdbool.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_FPS 60
#define FRAMES_SPEED 6

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
	Rectangle bound;
	Rectangle collision;
	int speed;

	// Animation related
	Rectangle frameRect;
	int currentFrame;
	int currentSequence;
	int framesCounter;

	// helper
	float frame_width;
	float frame_height;
} Player;

Player InitPlayer(Rectangle *rect)
{
	Player player;

	player.texture = LoadTexture("assets/Acly.png");

	player.frame_width = (float)player.texture.width / 3;      // The width of each individual sprite
	player.frame_height = (float)player.texture.height / 4;    // The height of each individual sprite

	float player_x_center = rect->x + (rect->width - player.frame_width) / 2;
	float player_y_center = rect->y + (rect->height - player.frame_height) / 2;
	player.position = (Vector2){player_x_center, player_y_center};
	player.speed = 4;

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

void UpdatePlayer(Player *player, Rectangle *rect)
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

    Vector2 newPosition = {
        player->position.x + playerDirection.x * player->speed,
        player->position.y + playerDirection.y * player->speed
    };

    Rectangle newBound = (Rectangle){newPosition.x + 6 * 3.2, newPosition.y, 99.0f - 12 * 3.2, 105.5f};
    Rectangle newCollision = (Rectangle){newBound.x + 6.5 * 1.6, newBound.y + newBound.height - 3.2f * 2.5, 13 * 3.2, 6.4f};

    if (newCollision.x >= rect->x &&
        newCollision.x + newCollision.width <= rect->x + rect->width &&
        newCollision.y >= rect->y &&
        newCollision.y + newCollision.height <= rect->y + rect->height)
    {
        player->position = newPosition;
    }

    player->frameRect.y = player->currentSequence * player->frame_height;
    player->frameRect.x = (isMoving ? player->currentFrame : 0) * player->frame_width;
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "callie - test");

    Rectangle tempFloorRect = {
        (float)(SCREEN_WIDTH - 600) / 2,
        (float)(SCREEN_HEIGHT - 300) / 2,
        600, 300
    };

    Player player = InitPlayer(&tempFloorRect);

    Camera2D camera = { 0 };
    camera.target = (Vector2){
        player.position.x + (float)player.frame_width / 2,
        player.position.y + (float)player.frame_height / 2
    };
    camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while(!WindowShouldClose())
    {
        UpdatePlayer(&player, &tempFloorRect);

        player.bound = (Rectangle){player.position.x + 6 * 3.2, player.position.y, 99.0f - 12 * 3.2, 105.5f};
        player.collision = (Rectangle){player.bound.x + 6.5 * 1.6, player.bound.y + player.bound.height - 3.2f * 2.5, 13 * 3.2, 6.4f};

        camera.target = (Vector2){
            player.position.x + (float)player.frame_width / 2,
            player.position.y + (float)player.frame_height / 2
        };

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);
            DrawRectangleRec(tempFloorRect, GRAY);
            DrawRectangleLinesEx(player.bound, 1.0f, WHITE);
            DrawRectangleLinesEx(player.collision, 1.0f, RED);
            DrawTextureRec(player.texture, player.frameRect, player.position, WHITE);
        EndMode2D();

        DrawText(TextFormat("FPS: %d", GetFPS()), 16, 16, 16, WHITE);
        DrawText(TextFormat("X, Y: %f, %f", player.position.x, player.position.y), 16, 40, 16, WHITE);

        EndDrawing();
    }

    UnloadPlayer(&player);
    CloseWindow();

    return 0;
}
