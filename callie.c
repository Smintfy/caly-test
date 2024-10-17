#include "include/raylib.h"
#include "include/raymath.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define MAX_FPS 60
#define FRAMES_SPEED 6

#define TILE_SIZE 128
#define CHUNK_SIZE 256

static int framesCounter = 0;
static int hotbarAmount = 8;
static int hotbarSize = 64;
static int spacing = 8;

typedef enum { DOWN, LEFT, RIGHT, UP } PlayerSequence;

typedef struct Player
{
	Vector2 position;
	Texture2D texture;
	Rectangle collision, detection;
	float width, height;
	int speed;
	bool allowMove;

	// Animation related
	Rectangle frameRect;
	int currentFrame, currentSequence;

	// Stats
	int health;
} Player;

typedef struct Object
{
    Vector2 position;
    Texture2D texture;
    Rectangle detection, frameRect, collision; // TODO: collision
} Object;

typedef struct Map
{
    int *tileMap;
    int width, height;
} Map;

typedef struct Item
{
    char name[32];
    int id;
} Item;

typedef struct Inventory
{
    Item items[16];
    int itemCount;
    Item hotbarItems[8];
    int activeSlot;
} Inventory;

Player InitPlayer()
{
	Player player;

    player.texture = LoadTexture("resources/textures/Hololive.png");

    player.width = (float)player.texture.width / 3;      // The width of each individual sprite
    player.height = (float)player.texture.height / 4;    // The height of each individual sprite

    int spawnX = 1;
    int spawnY = 1;

    player.position = (Vector2){
        spawnX * TILE_SIZE,
        spawnY * TILE_SIZE
    };

    player.health = 100;

    // TODO: Fix idle frame
    // Even off-setting the frameRect to player width doesn't help.
    // NOTE: Temporary fix
    // I basically switched first and second frame
    player.frameRect = (Rectangle){player.width, 0.0f, player.width, player.height};
    player.allowMove = true;
	player.speed = 4;
	player.currentSequence = DOWN;
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

void InitInventory(Inventory *inventory)
{
    inventory->itemCount = 0;
    inventory->activeSlot = 0;

    Item dagger = {"Dagger", 1};

    inventory->items[inventory->itemCount++] = dagger;

    // Add items to the inventory
    for (int i = 0; i < hotbarAmount; i++) {
        if (i < inventory->itemCount) {
            inventory->hotbarItems[i] = inventory->items[i];
        } else {
            Item emptyItem = {"Empty", 0}; // Placeholder for empty slots
            inventory->hotbarItems[i] = emptyItem;
        }
    }
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

void DrawInventoryHotbar(Inventory *inventory)
{
    int totalWidth = (hotbarSize + spacing) * hotbarAmount - spacing;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;

    for (int i = 0; i < hotbarAmount; i++) {
        Rectangle slotRect = {
            startX + i * (hotbarSize + spacing),
            SCREEN_HEIGHT - hotbarSize - 16,
            hotbarSize,
            hotbarSize,
        };

        DrawRectangleRec(slotRect, (Color){255, 255, 255, 125});

        // Draw the item name if there's an item in the slot
        if (strcmp(inventory->hotbarItems[i].name, "Empty") != 0 && inventory->activeSlot == i) {
            DrawText(inventory->hotbarItems[i].name, slotRect.x + 2, slotRect.y - 24, 16, WHITE);
        }

        if (i == inventory->activeSlot) {
            DrawRectangleLinesEx(slotRect, 4.0f, WHITE);
        }
    }
}

void DrawChunkBorder(int mapWidth, int mapHeight, Rectangle *collisionRect)
{
    Color color;

    int numChunksX = (int)ceil((float)(mapWidth * TILE_SIZE) / CHUNK_SIZE);
    int numChunksY = (int)ceil((float)(mapHeight * TILE_SIZE) / CHUNK_SIZE);

    for (int i = 0; i < numChunksX * numChunksY; i++) {
        int chunkX = i % numChunksX;
        int chunkY = i / numChunksX;
        float posX = chunkX * CHUNK_SIZE;
        float posY = chunkY * CHUNK_SIZE;

        if (collisionRect->x >= posX && collisionRect->x < posX + CHUNK_SIZE &&
            collisionRect->y >= posY && collisionRect->y < posY + CHUNK_SIZE) {
            color = GREEN;
        } else {
            color = PINK;
        }

        DrawRectangleLinesEx((Rectangle){posX, posY, CHUNK_SIZE, CHUNK_SIZE}, 1.0f, color);
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

void UpdateInventoryHotbar(Vector2 *mousePos, Inventory *inventory)
{
    int totalWidth = (hotbarSize + spacing) * hotbarAmount - spacing;
    int startX = (SCREEN_WIDTH - totalWidth) / 2;
    Rectangle hotbarRect = (Rectangle){startX, SCREEN_HEIGHT - hotbarSize - 16, totalWidth, hotbarSize};

    // Check if the mouse is within the hotbar area
    if (CheckCollisionPointRec(*mousePos, hotbarRect)) {
        // DrawText("Mouse Hotbar Hover", 16, 88, 16, GREEN);
        for (int i = 0; i < hotbarAmount; i++) {
            // Calculate the position of the current slot
            Rectangle slotRect = (Rectangle){startX + i * (hotbarSize + spacing), SCREEN_HEIGHT - hotbarSize - 16, hotbarSize, hotbarSize};

            if (CheckCollisionPointRec(*mousePos, slotRect)) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) inventory->activeSlot = i;
                break;
            }
        }
    }
}

void UpdatePlayer(Player *player, int *mapTile, int mapWidth, int mapHeight)
{
    framesCounter++;

    Vector2 playerDirection = {0.0f, 0.0f};
    bool isMoving = false;

    if (!player->allowMove) return;

    if (framesCounter >= (MAX_FPS / FRAMES_SPEED)) {
        framesCounter = 0;
        player->currentFrame++;

        if (player->currentFrame > 2) player->currentFrame = 0;

        player->frameRect.x = (float)player->currentFrame * player->width;
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

    Vector2 newPosition = player->position;

    // Check horizontal movement
    newPosition.x += playerDirection.x * player->speed;
    if (!CheckCollisionTileMap(&(Rectangle){newPosition.x + player->width / 4, player->collision.y, player->width / 2, 12}, mapTile, mapWidth, mapHeight)) {
        player->position.x = newPosition.x;
    }

    // Check vertical movement
    newPosition.y = player->position.y + playerDirection.y * player->speed;
    if (!CheckCollisionTileMap(&(Rectangle){player->collision.x, newPosition.y + player->height - 16, player->width / 2, 12}, mapTile, mapWidth, mapHeight)) {
        player->position.y = newPosition.y;
    }

    player->collision = (Rectangle){
        player->position.x + player->width / 4,
        player->position.y + player->height - 16,
        player->width / 2,
        12
    };

    player->detection = (Rectangle){
        player->position.x + (player->width / 2) - (8 * 3.2f / 2),
        player->position.y + (player->height / 2),
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
    Inventory playerInventory;

    int mapWidth, mapHeight;
    int *testMap = LoadTileMap("resources/maps/Test.txt", &mapWidth, &mapHeight);

    Player player = InitPlayer();
    InitInventory(&playerInventory);

    // Credit to https://penger.city/
    Music pengerMusic = LoadMusicStream("resources/audio/STOMP.ogg");
    Image pengerImage = LoadImage("resources/textures/Penger.png");
    ImageResize(&pengerImage, pengerImage.width * 2, pengerImage.height * 2);
    Texture2D pengerTexture = LoadTextureFromImage(pengerImage);
    UnloadImage(pengerImage);
    Vector2 pengerTilePosition = {2.0f + 0.25f , 0.0f - 0.25f};
    Object penger = InitObject(&pengerTexture, pengerTilePosition);

    // Sbeaker
    // Credit to https://youtu.be/XbCCKAKdSJM?si=NvlzzLTRX21VySf2
    Music speakerMusic = LoadMusicStream("resources/audio/IDOL_PROJECT.ogg");
    Image speakerImage = LoadImage("resources/textures/Speaker.png");
    ImageResize(&speakerImage, speakerImage.width * 2, speakerImage.height * 2);
    Texture2D speakerTexture = LoadTextureFromImage(speakerImage);
    UnloadImage(speakerImage);
    Vector2 speakerTilePosition = {5.0f + 0.25f , 3.0f - 0.25f};
    Object speaker = InitObject(&speakerTexture, speakerTilePosition);

    bool showDialog = false;

    Camera2D camera = { 0 };
    camera.target = (Vector2){
        player.position.x + (float)player.width / 2,
        player.position.y + (float)player.height / 2
    };
    camera.offset = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
    camera.zoom = 1.0f;
    const float CAMERA_SPEED = 0.05f;

    SetTargetFPS(60);

    printf("%f %f", player.frameRect.x, player.frameRect.y);

    while(!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        UpdatePlayer(&player, testMap, mapWidth, mapHeight);
        UpdateInventoryHotbar(&mousePos, &playerInventory);

        // Update the target position smoothly
        camera.target.x += (player.position.x + (float)player.width / 2 - camera.target.x) * CAMERA_SPEED;
        camera.target.y += (player.position.y + (float)player.height / 2 - camera.target.y) * CAMERA_SPEED;


        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);
            DrawTileMap(testMap, mapWidth, mapHeight);

            // Debug
            DrawChunkBorder(mapWidth, mapHeight, &player.collision);

            // Objects
            DrawTextureRec(penger.texture, penger.frameRect, penger.position, WHITE);
            DrawRectangleLinesEx(penger.detection, 1.0f, WHITE);
            DrawTextureRec(speaker.texture, speaker.frameRect, speaker.position, WHITE);
            DrawRectangleLinesEx(speaker.detection, 1.0f, (Color){255, 255, 255, 75});

            // Player
            DrawTextureRec(player.texture, player.frameRect, player.position, WHITE);
            DrawRectangleLinesEx(player.detection, 2.0f, BLUE);
            DrawRectangleLinesEx(player.collision, 2.0f, RED);

            // Speaker
            float distanceToSpeaker = Vector2Distance(
                (Vector2){player.detection.x + player.detection.width / 2, player.detection.y + player.detection.height / 2},
                (Vector2){speaker.position.x + (float)speaker.texture.width / 2, speaker.position.y + (float)speaker.texture.height / 2}
            );

            float volume = 1.0f;
            const float MAX_DISTANCE = 450.0f;

            if (distanceToSpeaker < 0) {
                volume = 1.0f;
            } else if (distanceToSpeaker <= MAX_DISTANCE) {
                volume = 1.0f - (distanceToSpeaker - 0) / (MAX_DISTANCE - 0);
            } else {
                volume = 0.0f;
            }

            UpdateMusicStream(speakerMusic);
            SetMusicVolume(speakerMusic, volume);

            if (distanceToSpeaker <= 500) {
                if (!IsMusicStreamPlaying(speakerMusic)) PlayMusicStream(speakerMusic);
                float alpha = 255.0f;

                if (distanceToSpeaker > 0) {
                    alpha = 255.0f * (1.0f - (distanceToSpeaker - 0) / (MAX_DISTANCE - 0));
                    if (alpha < 0) alpha = 0;
                }

                Color lineColor = (Color){255, 255, 255, (unsigned char)alpha};
                DrawLineEx(
                    (Vector2){player.detection.x + player.detection.width / 2, player.detection.y + player.detection.height / 2},
                    (Vector2){speaker.position.x + (float)speaker.texture.width / 2, speaker.position.y + (float)speaker.texture.height / 2},
                    1.5f,
                    lineColor
                );
            }
        EndMode2D();

        // Player stats
        DrawText(TextFormat("HP: %d", player.health), 16, SCREEN_HEIGHT - 40, 32, WHITE);
        DrawInventoryHotbar(&playerInventory);

        if (CheckCollisionRecs(player.detection, penger.detection)) {
            if (GetKeyPressed() == KEY_E) showDialog = !showDialog;
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) showDialog = false;
            DrawText("Object Detected!", 16, 88, 16, GREEN);
        }

        // TODO: Extract this to a separate dialog primitive
        if (showDialog) {
            player.allowMove = false;

            if (!IsMusicStreamPlaying(pengerMusic)) PlayMusicStream(pengerMusic);
            UpdateMusicStream(pengerMusic);

            float dialogBoxWidth = SCREEN_WIDTH * 0.6f;
            float dialogBoxHeight = SCREEN_HEIGHT * 0.2f;
            float dialogBoxX = (SCREEN_WIDTH - dialogBoxWidth) / 2;
            float dialogBoxY = SCREEN_HEIGHT - dialogBoxHeight - 16;

            DrawRectangle(dialogBoxX, dialogBoxY, dialogBoxWidth, dialogBoxHeight, BLACK);
            DrawRectangleLinesEx((Rectangle){dialogBoxX, dialogBoxY, dialogBoxWidth, dialogBoxHeight}, 2, WHITE);

            const char *dialogText = "Penger: I'm Penger. Your mom's banger!";

            // Text padding
            float textOffsetX = 16;
            float textOffsetY = 16;
            DrawText(dialogText, dialogBoxX + textOffsetX, dialogBoxY + textOffsetY, 24, WHITE);
        } else {
            StopMusicStream(pengerMusic);
            player.allowMove = true;
        }

        DrawTexture(player.texture, SCREEN_WIDTH - player.texture.width - 16, 0, WHITE);
        DrawRectangleLinesEx((Rectangle){SCREEN_WIDTH - 48.0f * 3 + player.frameRect.x - 16, player.frameRect.y, player.frameRect.width, player.frameRect.height}, 2.0f, RED);
        DrawTextureRec(player.texture, player.frameRect, (Vector2){SCREEN_WIDTH - player.texture.width - player.width - 32, 0}, WHITE);

        // Debug
        DrawText(TextFormat("FPS: %d", GetFPS()), 16, 16, 16, WHITE);
        DrawText(TextFormat("X, Y: %f, %f", player.position.x, player.position.y), 16, 40, 16, WHITE);
        DrawText(TextFormat("Tx, Ty: %d, %d", (int)floor(player.collision.x / TILE_SIZE), (int)floor(player.collision.y / TILE_SIZE)), 16, 64, 16, WHITE);

        EndDrawing();
    }

    UnloadTileMap(testMap);
    UnloadMusicStream(pengerMusic); UnloadMusicStream(speakerMusic);
    UnloadObject(&penger); UnloadObject(&speaker);
    UnloadPlayer(&player);
    CloseWindow();

    return 0;
}
