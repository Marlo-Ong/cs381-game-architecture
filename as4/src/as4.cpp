// Author: Marlo Ongkingco
// CS381 Game Engine Architecture
// AS4 Ad-Hoc Game: Stupid Blackjack

#include "raylib-cpp.hpp"
#include <iostream>
#include <string>
#include <format>
using namespace std;

struct Card {
    Image sprite;
    Texture2D cardTexture;
    Rectangle transform;    // the actual rect transform of the card
    Rectangle source;       // rectangular crop of the image asset; does not change since using full picture

    bool connected;         // player is holding this card
    bool faceUp;
    Vector2 startingPos;
    int value;
    string suit;
    string name;            // "value" of "suit"

    Card(){}

    Card(Vector2 pos) {
        startingPos = pos;
        Reset();
    }

    void Reveal() {
        // Assigns a random card suit/value and updates the texture //

        if (!faceUp) {
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
            SetImageTexture(filepath);
            faceUp = true;
        }
    }

    void Draw() {
        //DrawRectangleRec(transform, BLUE);

        // Highlight yellow if player is moving card, else white
        Color colorHighlight = (connected) ? YELLOW : WHITE;

        DrawTexturePro(cardTexture,
                        source,
                        transform,
                        (Vector2){0, 0},
                        0.0,
                        colorHighlight);
    }

    void UpdatePosition(Vector2 pos) {
        transform = (Rectangle){pos.x - source.width / 2, pos.y - source.width / 2, source.width, source.height};
    }

    bool isColliding(Rectangle rect) {
        return CheckCollisionRecs(transform, rect);
    }

    void Reset() {
        connected = false;
        faceUp = false;
        SetImageTexture("../cards/card-blank.png");
        source = { 0, 0, (float)cardTexture.width, (float)cardTexture.height };
        UpdatePosition(startingPos);
    }

    void SetImageTexture(string filepath) {
        sprite = LoadImage(filepath.c_str());
        ImageResize(&sprite, 100, 100);
        cardTexture = LoadTextureFromImage(sprite);
        UnloadImage(sprite);
    }
};

void TryPickupCard(Card** holder, Card* cardPicked) {
    if ((*holder) != nullptr) {
        (**holder).connected = false;
        (*holder) = nullptr;
    }
    else {
        (*holder) = cardPicked;
        (*cardPicked).connected = true;
    }
}

int main()
{
    // Initialize window
    int screenWidth = 600;
    int screenHeight = 500;
    raylib::Window window(screenWidth, screenHeight, "CS381 Assignment 4");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    // Initialize cards
    Card card1 = Card((Vector2){-100, 0});
    card1.Reveal();
    Card card2 = Card((Vector2){100, 0});
    Card card3 = Card((Vector2){10000, 10000}); // "hit" card should not be seen until deck is clicked
    Card* cardHolding = nullptr;
    Rectangle cardSlot = {card1.startingPos.x - 50,
                            card1.startingPos.y - 50,
                            card1.transform.width - 15,
                            card1.transform.height,
                        };

    // Player variables
    Rectangle player = { 400, 280, 40, 40 };
    float playerSpeed = 0.5f;
    int playerCardsValue = card1.value;

    // Initialize camera
    raylib::Camera2D camera(
        raylib::Vector2(0, 0),  // position
        raylib::Vector2(player.x, player.y)  // target
    );
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.zoom = 1;

    // Initialize timer
    float cardResetTimer = 20;
    float timer = cardResetTimer;
    const char* timerText = "10";

    // Initialize deck
    Image deckSprite = LoadImage("../icons/stack.png");
    ImageResize(&deckSprite, 100, 100);
    Texture2D deckTexture = LoadTextureFromImage(deckSprite);
    UnloadImage(deckSprite);
    Rectangle deckSource = { 0, 0, (float)deckTexture.width, (float)deckTexture.height };
    Rectangle deckRect = { 100, 100, deckSource.width, deckSource.height };

    while (!WindowShouldClose())
    {
        // Player movement ----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) player.x += playerSpeed;
        else if (IsKeyDown(KEY_LEFT)) player.x -= playerSpeed;
        else if (IsKeyDown(KEY_UP)) player.y -= playerSpeed;
        else if (IsKeyDown(KEY_DOWN)) player.y += playerSpeed;

        // Check collision
        if (IsKeyPressed(KEY_ENTER)) {
            if (CheckCollisionRecs(player, deckRect)) {
                TryPickupCard(&cardHolding, &card3);
            }
            else if (card1.isColliding(player)) {
                TryPickupCard(&cardHolding, &card1);
            }
            else if (card2.isColliding(player)) {
                if (card2.isColliding(cardSlot) && !card2.faceUp) {
                    card2.Reveal();
                    playerCardsValue += card2.value;
                }
                else {
                    TryPickupCard(&cardHolding, &card2);
                }
            }
            else if (card3.isColliding(player)) {
                if (card3.isColliding(cardSlot) && !card3.faceUp) {
                    card3.Reveal();
                    playerCardsValue += card3.value;
                }
                else {
                    TryPickupCard(&cardHolding, &card3);
                }
            }
        }

        // Update variables ----------------------------------------------------------------------------------
        timer -= window.GetFrameTime();
        timerText = (to_string(((int)timer)+1)).c_str(); // counts 10 to 1, converted to C string;
        if (timer <= 0) {
            card1.Reset();
            card1.Reveal();
            card2.Reset();
            card3.Reset();
            timer = cardResetTimer;
            cardHolding = nullptr;
            playerCardsValue = card1.value;
        }
        if (cardHolding != nullptr){
            (*cardHolding).UpdatePosition((Vector2){player.x, player.y});
        }


        // Draw ----------------------------------------------------------------------------------
        camera.target = (Vector2){ player.x, player.y };
        BeginDrawing();
        {
            camera.BeginMode();
            {
                window.ClearBackground(BLACK);

                // Draw card slots
                //DrawRectangleLinesEx(cardSlot, 2, BLUE);
                DrawRectangleRec(cardSlot, RED);

                // Draw deck
                DrawTexturePro(deckTexture,deckSource, deckRect, (Vector2){0,0}, 0, WHITE);

                // Draw player
                DrawRectangleRec(player, RED);

                // Draw cards
                card1.Draw();
                card2.Draw();
                card3.Draw();
            }
            camera.EndMode();
            DrawText(timerText, screenWidth / 100, screenHeight / 100, 50, WHITE);
            string playerCardsValueText = "Your card value: " + to_string(playerCardsValue);
            DrawText(playerCardsValueText.c_str(), screenWidth / 100, screenHeight / 100 + 50, 50, WHITE);
        }
        EndDrawing();
    }

    return 0;
}