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
	Rectangle detection;
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

typedef struct Object
{
    Vector2 position;
    Texture2D texture;
    Rectangle detection;
    Rectangle frameRect;
} Object;

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

Object InitObject(Texture2D *objectTexture, Vector2 *position)
{
    Object object;

    object.texture = *objectTexture;
    object.position = *position;

    object.frameRect = (Rectangle){0.0f, 0.0f, object.texture.width, object.texture.height};

    object.detection = (Rectangle){
        position->x,
        position->y,
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

    Rectangle newBound = (Rectangle){newPosition.x + 6 * 3.2, newPosition.y, player->frame_width - 12 * 3.2, player->frame_height};
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

    player->bound = (Rectangle){
        player->position.x + 6 * 3.2,
        player->position.y,
        player->frame_width - 12 * 3.2,
        player->frame_height
    };

    player->collision = (Rectangle){
        player->bound.x + 6.5 * 1.6,
        player->bound.y + player->bound.height - 3.2f * 2.5,
        13 * 3.2,
        6.4f
    };

    player->detection = (Rectangle){
        player->position.x + (player->frame_width / 2) - (8 * 3.2f / 2),
        player->position.y + (player->frame_height / 2) - (8 * 3.2f / 2),
        8 * 3.2,
        8 * 3.2
    };
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "callie - test");
    InitAudioDevice();

    Rectangle tempFloorRect = {
        (float)(SCREEN_WIDTH - 600) / 2,
        (float)(SCREEN_HEIGHT - 300) / 2,
        600, 300
    };

    Player player = InitPlayer(&tempFloorRect);
    Music music = LoadMusicStream("assets/STOMP.ogg");

    Image penger_img = LoadImage("assets/Penger.png");
    ImageResize(&penger_img, penger_img.width * 2, penger_img.height * 2);
    Texture2D penger_texture = LoadTextureFromImage(penger_img);
    UnloadImage(penger_img);

    Vector2 penger_pos = (Vector2){
        tempFloorRect.x + (tempFloorRect.width - penger_texture.width) / 2,
        tempFloorRect.y + 20.0f - penger_texture.height
    };

    Object penger = InitObject(&penger_texture, &penger_pos);
    bool showDialog = false;

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

        camera.target = (Vector2){
            player.position.x + (float)player.frame_width / 2,
            player.position.y + (float)player.frame_height / 2
        };

        BeginDrawing();

        ClearBackground(BLACK);

        BeginMode2D(camera);
            DrawRectangleRec(tempFloorRect, GRAY);

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
            DrawText("Object Detected!", 16, 64, 16, GREEN);
        }
        else
        {
            showDialog = false;
             StopMusicStream(music);
        }

        if (showDialog)
        {
            if (!IsMusicStreamPlaying(music)) {
                PlayMusicStream(music);
            }

            UpdateMusicStream(music);

            float dialogBoxWidth = SCREEN_WIDTH * 0.6f;
            float dialogBoxHeight = SCREEN_HEIGHT * 0.2f;
            float dialogBoxX = (SCREEN_WIDTH - dialogBoxWidth) / 2;
            float dialogBoxY = SCREEN_HEIGHT - dialogBoxHeight - 50;

            DrawRectangle(dialogBoxX, dialogBoxY, dialogBoxWidth, dialogBoxHeight, BLACK);
            DrawRectangleLinesEx((Rectangle){dialogBoxX, dialogBoxY, dialogBoxWidth, dialogBoxHeight}, 2, WHITE);

            const char* dialogText = "Penger: Your mom's banger!";

            float textOffsetX = 16;
            float textOffsetY = 16;
            DrawText(dialogText, dialogBoxX + textOffsetX, dialogBoxY + textOffsetY, 24, WHITE);
        }

        DrawText(TextFormat("FPS: %d", GetFPS()), 16, 16, 16, WHITE);
        DrawText(TextFormat("X, Y: %f, %f", player.position.x, player.position.y), 16, 40, 16, WHITE);

        EndDrawing();
    }

    UnloadMusicStream(music);
    UnloadObject(&penger);
    UnloadPlayer(&player);
    CloseWindow();

    return 0;
}
