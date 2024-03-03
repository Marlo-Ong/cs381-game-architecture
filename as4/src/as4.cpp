// Author: Marlo Ongkingco
// CS381 Game Engine Architecture
// AS4 Ad-Hoc Game: Stupid Blackjack

#include "raylib-cpp.hpp"
#include <iostream>
#include <string>
#include <format>
using namespace std;

struct Card {
    Image image;
    Texture2D texture;
    Rectangle bounding;
    Rectangle source;

    bool connected;
    Vector2 startingPos;
    int value;
    string suit;
    string name;

    Card(Vector2 pos) {
        startingPos = pos;
        Reset();
    }

    void AssignRandomCard() {
        string suits[4] = {"hearts", "spades", "diamonds", "clubs"};
        string values[13] = {"ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king"};
        int valuesInt[13] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10};
    
        string randSuit = suits[GetRandomValue(0, 3)];
        suit = randSuit;

        int randValIndex = GetRandomValue(0, 12);
        string randVal = values[randValIndex];
        value = valuesInt[randValIndex];

        name = randVal + " of " + randSuit;
        string filepath = "../cards/card-" + randVal + "-" + randSuit + ".png";
        image = LoadImage(filepath.c_str());
        ImageResize(&image, 100, 100);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    void Draw() {
        //DrawRectangleRec(bounding, BLUE);

        // Highlight yellow if player is moving card, else white
        Color colorHighlight = (connected) ? YELLOW : WHITE;

        DrawTexturePro(texture,
                        source,
                        bounding,
                        (Vector2){0, 0},
                        0.0,
                        colorHighlight);
    }

    void UpdatePosition(Vector2 pos) {
        bounding = (Rectangle){pos.x - source.width / 2, pos.y - source.width / 2, source.width, source.height};
    }

    bool isColliding(Rectangle rect) {
        return CheckCollisionRecs(bounding, rect);
    }

    void Reset() {
        connected = false;
        AssignRandomCard();
        source = { 0, 0, (float)texture.width, (float)texture.height };
        UpdatePosition(startingPos);
    }
};

int main()
{
    raylib::Window window(600, 500, "CS381 Assignment 4");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    int screenWidth = 600;
    int screenHeight = 500;
    Rectangle player = { 400, 280, 40, 40 };

    Card card1 = Card((Vector2){0, 0});
    Card card2 = Card((Vector2){0, 0});

    raylib::Camera2D camera(
        raylib::Vector2(0, 0),  // position
        raylib::Vector2(player.x + 20.0f, player.y + 20.0f)  // target
    );
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.zoom = 1;

    float cardResetTimer = 10;
    float timer = cardResetTimer;

    while (!WindowShouldClose())
    {
        // Player movement ----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) player.x += 2;
        else if (IsKeyDown(KEY_LEFT)) player.x -= 2;
        else if (IsKeyDown(KEY_UP)) player.y -= 2;
        else if (IsKeyDown(KEY_DOWN)) player.y += 2;
        if (IsKeyPressed(KEY_ENTER)) {
            if (card1.isColliding(player)) {
                card1.connected = !card1.connected;
            }
        }

        // Update variables ----------------------------------------------------------------------------------
        //timer -= window.GetFrameTime();
        // if (timer <= 0) {
        //     card1.Reset();
        //     card2.Reset();
        //     timer = cardResetTimer;
        // }

        if (card1.connected) card1.UpdatePosition((Vector2){player.x, player.y});


        // Draw ----------------------------------------------------------------------------------
        camera.target = (Vector2){ player.x, player.y };
        BeginDrawing();
        {
            camera.BeginMode();
            {
                window.ClearBackground(GRAY);

                DrawRectangle(0,0,200,200,YELLOW);
                DrawRectangleRec(player, RED);
                card1.Draw();

            }
            camera.EndMode();
        }
        EndDrawing();
    }

    return 0;
}