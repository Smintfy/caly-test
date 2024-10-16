#include "include/raylib.h"
#include "include/raymath.h"
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define MAX_FPS 60
#define FRAMES_SPEED 6
#define TILE_SIZE 128

typedef enum { DOWN, UP, RIGHT, LEFT } PlayerSequence;

typedef struct Player
{
	Vector2 position;
	Texture2D texture;
	Rectangle bound, collision, detection;
	float width, height;
	int speed;
	bool allowMove;

	// Animation related
	Rectangle frameRect;
	int currentFrame, currentSequence, framesCounter;
} Player;

typedef struct Object
{
    Vector2 position;
    Texture2D texture;
    Rectangle detection, frameRect, collision;
} Object;

Player InitPlayer()
{
	Player player;

	player.texture = LoadTexture("resources/textures/Acly.png");

	player.width = (float)player.texture.width / 3;      // The width of each individual sprite
	player.height = (float)player.texture.height / 4;    // The height of each individual sprite

    int spawnX = 1;
    int spawnY = 1;

    player.position = (Vector2){
        spawnX * TILE_SIZE,
        spawnY * TILE_SIZE
    };

    player.allowMove = true;
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

    object.position = (Vector2){
        tilePosition.x * TILE_SIZE,
        tilePosition.y * TILE_SIZE
    };

    object.frameRect = (Rectangle){
        0.0f,
        0.0f,
        object.texture.width,
        object.texture.height
    };

    object.detection = (Rectangle){
        object.position.x,
        object.position.y,
        object.texture.width,
        object.texture.height
    };

    return object;
}

int *LoadTileMap(const char *filePath, int *width, int *height)
{
    FILE *file = fopen(filePath, "r");

    if (file == NULL) {
        printf("ERROR: Failed to open file %s\n", filePath);
        return NULL;
    }

    fscanf(file, "%d", width);
    fscanf(file, "%d", height);

    int *tileMap = (int *)malloc((*width) * (*height) * sizeof(int));

    for (int i = 0; i < (*width) * (*height); i++)
        fscanf(file, "%d", &tileMap[i]);

    fclose(file);
    return tileMap;
}

void UnloadTileMap(int *tileMap)
{
    free(tileMap);
}

void UnloadPlayer(Player *player)
{
    UnloadTexture(player->texture);
}

void UnloadObject(Object *object)
{
    UnloadTexture(object->texture);
}

void DrawTileMap(int *tileMap, int width, int height)
{
    for (int i = 0; i < width * height; i++) {
        int x = i % width;
        int y = i / width;
        if (tileMap[i] == 1) DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, GRAY);
    }
}

bool CheckCollisionTileMap(Rectangle *playerRect, int *tileMap, int mapWidth, int mapHeight)
{
    int tileStartX = (int)floorf(playerRect->x / TILE_SIZE);
    int tileStartY = (int)floorf(playerRect->y / TILE_SIZE);
    int tileEndX = (int)floorf((playerRect->x + playerRect->width - 1) / TILE_SIZE);
    int tileEndY = (int)floorf((playerRect->y + playerRect->height - 1) / TILE_SIZE);

    // Check if player is outside the map
    if (tileStartX < 0 || tileStartY < 0 || tileEndX >= mapWidth || tileEndY >= mapHeight) return true;

    for (int y = tileStartY; y <= tileEndY; y++) {
        for (int x = tileStartX; x <= tileEndX; x++) {
            int index = (y * mapWidth) + x;
            // Check current tile
            if (tileMap[index] == 0) {
                Rectangle tileRect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                if (CheckCollisionRecs(*playerRect, tileRect)) return true;
            }
        }
    }
    return false;
}

// bool CheckCollisionObject(Rectangle *playerRect, Rectangle *objectRect)
// {

// }

void UpdatePlayer(Player *player, int *mapTile, int mapWidth, int mapHeight)
{
    if (!player->allowMove) {
        return;
    }

    Vector2 playerDirection = {0.0f, 0.0f};
    bool isMoving = false;

    player->frameRect.x = 0.0f;
    player->framesCounter++;

    if (player->framesCounter >= (MAX_FPS / FRAMES_SPEED)) {
        player->framesCounter = 0;
        player->currentFrame++;

        if (player->currentFrame > 2) player->currentFrame = 0;
    }

    if (IsKeyDown(KEY_W)) {
        playerDirection.y = -1.0f;
        player->currentSequence = UP;
        isMoving = true;
    }
    if (IsKeyDown(KEY_S)) {
        playerDirection.y = 1.0f;
        player->currentSequence = DOWN;
        isMoving = true;
    }

    // Prevent diagonal movement
    if (playerDirection.y == 0.0f) {
        if (IsKeyDown(KEY_A)) {
            playerDirection.x = -1.0f;
            player->currentSequence = LEFT;
            isMoving = true;
        }
        if (IsKeyDown(KEY_D)) {
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
        newPosition.x + 6.4f * 1.6f + 6 * 3.2f,
        newPosition.y + player->height - 3.2f * 2.4f,
        13 * 3.2f,
        6.4f
    };

    if (!CheckCollisionTileMap(&newCollision, mapTile, mapWidth, mapHeight))
        player->position = newPosition;

    player->bound = (Rectangle){
        player->position.x + 6 * 3.2f,
        player->position.y,
        player->width - 12 * 3.2f,
        player->height
    };

    player->collision = (Rectangle){
        player->bound.x + 6.4f * 1.6f,
        player->bound.y + player->bound.height - 3.2f * 2.4f,
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

    int mapWidth, mapHeight;
    int *testMap = LoadTileMap("resources/maps/Test.txt", &mapWidth, &mapHeight);

    Player player = InitPlayer();

    // Credit to https://penger.city/
    Music music = LoadMusicStream("resources/audio/STOMP.ogg");
    Image pengerImage = LoadImage("resources/textures/Penger.png");
    ImageResize(&pengerImage, pengerImage.width * 2, pengerImage.height * 2);
    Texture2D pengerTexture = LoadTextureFromImage(pengerImage);
    UnloadImage(pengerImage);

    Vector2 pengerTilePosition = {2.0f + 0.25f , 0.0f - 0.25f};
    Object penger = InitObject(&pengerTexture, pengerTilePosition);

    bool showDialog = false;

    Camera2D camera = { 0 };
    camera.target = (Vector2){
        player.position.x + (float)player.width / 2,
        player.position.y + (float)player.height / 2
    };
    camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    camera.zoom = 1.0f;

    SetTargetFPS(60);

    while(!WindowShouldClose()) {
        UpdatePlayer(&player, testMap, mapWidth, mapHeight);

        camera.target = (Vector2){
            player.position.x + (float)player.width / 2,
            player.position.y + (float)player.height / 2
        };

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);
            DrawTileMap(testMap, mapWidth, mapHeight);

            // Penger
            DrawTextureRec(penger.texture, penger.frameRect, penger.position, WHITE);
            DrawRectangleLinesEx(penger.detection, 1.0f, WHITE);

            // Player
            DrawTextureRec(player.texture, player.frameRect, player.position, WHITE);
            DrawRectangleLinesEx(player.detection, 2.0f, BLUE);
            DrawRectangleLinesEx(player.bound, 2.0f, WHITE);
            DrawRectangleLinesEx(player.collision, 2.0f, RED);
        EndMode2D();

        if (CheckCollisionRecs(player.detection, penger.detection)) {
            if (GetKeyPressed() == KEY_E) showDialog = !showDialog;
            DrawText("Object Detected!", 16, 88, 16, GREEN);
        } else {
            showDialog = false;
            StopMusicStream(music);
        }

        if (showDialog) {
            player.allowMove = false;

            if (!IsMusicStreamPlaying(music)) PlayMusicStream(music);
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
        } else {
            player.allowMove = true;
        }

        DrawText(TextFormat("FPS: %d", GetFPS()), 16, 16, 16, WHITE);
        DrawText(TextFormat("X, Y: %f, %f", player.position.x, player.position.y), 16, 40, 16, WHITE);
        DrawText(TextFormat("Tx, Ty: %d, %d", (int)floor(player.collision.x / TILE_SIZE), (int)floor(player.collision.y / TILE_SIZE)), 16, 64, 16, WHITE);

        EndDrawing();
    }

    UnloadTileMap(testMap);
    UnloadMusicStream(music);
    UnloadObject(&penger);
    UnloadPlayer(&player);
    CloseWindow();

    return 0;
}
