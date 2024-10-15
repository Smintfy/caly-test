#include "include/raylib.h"
#include "include/raymath.h"
#include <stdio.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_FPS 60
#define FRAMES_SPEED 6

#define TILE_SIZE 128
#define MAP_WIDTH 7
#define MAP_HEIGHT 4

int tileMap[MAP_HEIGHT][MAP_WIDTH] =
{
    {1, 0, 1, 0, 1, 0, 0},
    {1, 1, 1, 1, 1, 0, 1},
    {0, 1, 0, 1, 0, 0, 1},
    {1, 1, 0, 1, 1, 1, 1},
};

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
	Rectangle detection;
	int speed;

	// Animation related
	Rectangle frameRect;
	int currentFrame;
	int currentSequence;
	int framesCounter;

	// helper
	float width;
	float height;
} Player;

typedef struct Object
{
    Vector2 position;
    Texture2D texture;
    Rectangle detection;
    Rectangle frameRect;
} Object;

Player InitPlayer()
{
	Player player;

	player.texture = LoadTexture("assets/Acly.png");

	player.width = (float)player.texture.width / 3;      // The width of each individual sprite
	player.height = (float)player.texture.height / 4;    // The height of each individual sprite

    int spawnX = 1;
    int spawnY = 1;

    player.position = (Vector2){
        spawnX * TILE_SIZE,
        spawnY * TILE_SIZE
    };

	player.speed = 4;
	player.frameRect = (Rectangle){0.0f, 0.0f, player.width, player.height};
	player.framesCounter = 0;
	player.currentSequence = 0;
	player.currentFrame = 0;

	return player;
}

Object InitObject(Texture2D *objectTexture, Vector2 tilePosition)
{
    Object object;

    object.texture = *objectTexture;

    object.position = (Vector2)
    {
        tilePosition.x * TILE_SIZE,
        tilePosition.y * TILE_SIZE
    };

    object.frameRect = (Rectangle)
    {
        0.0f,
        0.0f,
        object.texture.width,
        object.texture.height
    };

    object.detection = (Rectangle)
    {
        object.position.x,
        object.position.y,
        object.texture.width,
        object.texture.height
    };

    return object;
}

void UnloadPlayer(Player *player)
{
    UnloadTexture(player->texture);
}

void UnloadObject(Object *object)
{
    UnloadTexture(object->texture);
}

void DrawTileMap()
{
    for(int y = 0; y < MAP_HEIGHT; y++)
    {
        for(int x = 0; x < MAP_WIDTH; x++)
        {
            if (tileMap[y][x] == 1) DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY);
        }
    }
}

bool CheckCollisionTileMap(Rectangle playerRect)
{
    int tileStartX = (int)floorf(playerRect.x / TILE_SIZE);
    int tileStartY = (int)floorf(playerRect.y / TILE_SIZE);
    int tileEndX = (int)floorf((playerRect.x + playerRect.width - 1) / TILE_SIZE);
    int tileEndY = (int)floorf((playerRect.y + playerRect.height - 1) / TILE_SIZE);

    // Check if player is outside the map
    if (tileStartX < 0 || tileStartY < 0 || tileEndX >= MAP_WIDTH || tileEndY >= MAP_HEIGHT) return true;

    for (int y = tileStartY; y <= tileEndY; y++)
    {
        for (int x = tileStartX; x <= tileEndX; x++)
        {
            // Check current tile
            if (tileMap[y][x] == 0)
            {
                Rectangle tileRect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                if (CheckCollisionRecs(playerRect, tileRect)) return true;
            }
        }
    }
    return false;
}

void UpdatePlayer(Player *player)
{
    Vector2 playerDirection = {0.0f, 0.0f};
    bool isMoving = false;

    player->frameRect.x = 0.0f;
    player->framesCounter++;

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
    }

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
        }
    }

    Vector2 newPosition = {
        player->position.x + playerDirection.x * player->speed,
        player->position.y + playerDirection.y * player->speed
    };

    Rectangle newCollision = {
        newPosition.x + 6.5f * 1.6f + 6 * 3.2f,
        newPosition.y + player->height - 3.2f * 2.5f,
        13 * 3.2f,
        6.4f
    };

    if (!CheckCollisionTileMap(newCollision)) player->position = newPosition;

    player->bound = (Rectangle){
        player->position.x + 6 * 3.2f,
        player->position.y,
        player->width - 12 * 3.2f,
        player->height
    };

    player->collision = (Rectangle){
        player->bound.x + 6.5f * 1.6f,
        player->bound.y + player->bound.height - 3.2f * 2.5f,
        13 * 3.2f,
        6.4f
    };

    player->detection = (Rectangle){
        player->position.x + (player->width / 2) - (8 * 3.2f / 2),
        player->position.y + (player->height / 2) - (8 * 3.2f / 2),
        8 * 3.2f,
        8 * 3.2f
    };

    player->frameRect.y = player->currentSequence * player->height;
    player->frameRect.x = (isMoving ? player->currentFrame : 0) * player->width;
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "callie - test");
    InitAudioDevice();

    Player player = InitPlayer();

    // Credit to https://penger.city/
    Music music = LoadMusicStream("assets/STOMP.ogg");
    Image pengerImage = LoadImage("assets/Penger.png");
    ImageResize(&pengerImage, pengerImage.width * 2, pengerImage.height * 2);
    Texture2D pengerTexture = LoadTextureFromImage(pengerImage);
    UnloadImage(pengerImage);

    Vector2 pengerTilePosition = {2.0f + 0.25f , 0.0f};
    Object penger = InitObject(&pengerTexture, pengerTilePosition);

    bool showDialog = false;

    Camera2D camera = { 0 };
    camera.target = (Vector2)
    {
        player.position.x + (float)player.width / 2,
        player.position.y + (float)player.height / 2
    };
    camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while(!WindowShouldClose())
    {
        UpdatePlayer(&player);

        camera.target = (Vector2)
        {
            player.position.x + (float)player.width / 2,
            player.position.y + (float)player.height / 2
        };

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);
            DrawTileMap();

            // Penger
            DrawTextureRec(penger.texture, penger.frameRect, penger.position, WHITE);
            DrawRectangleLinesEx(penger.detection, 1.0f, WHITE);

            // Player
            DrawTextureRec(player.texture, player.frameRect, player.position, WHITE);
            DrawRectangleLinesEx(player.detection, 2.0f, BLUE);
            DrawRectangleLinesEx(player.bound, 2.0f, WHITE);
            DrawRectangleLinesEx(player.collision, 2.0f, RED);
        EndMode2D();

        if (CheckCollisionRecs(player.detection, penger.detection))
        {
            if (GetKeyPressed() == KEY_E) showDialog = !showDialog;
            DrawText("Object Detected!", 16, 88, 16, GREEN);
        }
        else
        {
            showDialog = false;
             StopMusicStream(music);
        }

        if (showDialog)
        {
            if (!IsMusicStreamPlaying(music))
            {
                PlayMusicStream(music);
            }

            UpdateMusicStream(music);

            float dialogBoxWidth = SCREEN_WIDTH * 0.6f;
            float dialogBoxHeight = SCREEN_HEIGHT * 0.2f;
            float dialogBoxX = (SCREEN_WIDTH - dialogBoxWidth) / 2;
            float dialogBoxY = SCREEN_HEIGHT - dialogBoxHeight - 50;

            DrawRectangle(dialogBoxX, dialogBoxY, dialogBoxWidth, dialogBoxHeight, BLACK);
            DrawRectangleLinesEx((Rectangle){dialogBoxX, dialogBoxY, dialogBoxWidth, dialogBoxHeight}, 2, WHITE);

            const char* dialogText = "Penger: I'm Penger. Your mom's banger!";

            float textOffsetX = 16;
            float textOffsetY = 16;
            DrawText(dialogText, dialogBoxX + textOffsetX, dialogBoxY + textOffsetY, 24, WHITE);
        }

        DrawText(TextFormat("FPS: %d", GetFPS()), 16, 16, 16, WHITE);
        DrawText(TextFormat("X, Y: %f, %f", player.position.x, player.position.y), 16, 40, 16, WHITE);
        DrawText(TextFormat("Tx, Ty: %d, %d", (int)floor(player.collision.x / TILE_SIZE), (int)floor(player.collision.y / TILE_SIZE)), 16, 64, 16, WHITE);

        EndDrawing();
    }

    UnloadMusicStream(music);
    UnloadObject(&penger);
    UnloadPlayer(&player);
    CloseWindow();

    return 0;
}
