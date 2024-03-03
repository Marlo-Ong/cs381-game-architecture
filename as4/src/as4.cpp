// Author: Marlo Ongkingco
// CS381 Game Engine Architecture
// AS4 Ad-Hoc Game: Stupid Blackjack

#include "raylib-cpp.hpp"
#include <iostream>
#include <string>
#include <format>
using namespace std;

struct Player {
    Rectangle rect;
    float speed;
    int cardsValue;
    int numHitsDefault;
    int numHitsLeft;
    bool bust;

    Player(Rectangle r){
        rect = r;
        speed = 0.5f;
        cardsValue = 0;
        numHitsDefault = 1;
        numHitsLeft = numHitsDefault;
        bust = false;
    }
};

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
    Color color;

    Card(){}

    Card(Vector2 pos, Color newColor) {
        startingPos = pos;
        color = newColor;
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

        // Highlight color if player is moving card, else white
        Color colorHighlight = (connected) ? color : WHITE;

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
    Card card1 = Card((Vector2){-0, 0}, BLUE);
    card1.Reveal();
    Card card2 = Card((Vector2){75, 150}, RED);
    Card card3 = Card((Vector2){10000, 10000}, GREEN); // "hit" card should not be seen until deck is clicked
    Card* cardHolding = nullptr;
    float spacing = 150;
    Rectangle cardSlot1 = {card1.startingPos.x - 45,
                            card1.startingPos.y - 50,
                            card1.transform.width - 10,
                            card1.transform.height
                        };
    Rectangle cardSlot2 = cardSlot1;
    cardSlot2.x += spacing;
    Rectangle cardSlot3 = cardSlot2;
    cardSlot3.x += spacing;

    // Player variables
    Player player = Player((Rectangle){ 150, 250, 40, 40 });

    // Initialize camera
    raylib::Camera2D camera(
        raylib::Vector2(0, 0),  // position
        raylib::Vector2(player.rect.x, player.rect.y)  // target
    );
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.zoom = 1;

    // Initialize timer
    float cardResetTimer = 10;
    float timer = cardResetTimer;
    const char* timerText;

    // Initialize deck
    Image deckSprite = LoadImage("../icons/stack.png");
    ImageResize(&deckSprite, 100, 100);
    Texture2D deckTexture = LoadTextureFromImage(deckSprite);
    UnloadImage(deckSprite);
    Rectangle deckSource = { 0, 0, (float)deckTexture.width, (float)deckTexture.height };
    Rectangle deckRect = { 200, 100, deckSource.width, deckSource.height };

    while (!WindowShouldClose())
    {
        // Player movement ----------------------------------------------------------------------------------
        if (IsKeyDown(KEY_RIGHT)) player.rect.x += player.speed;
        else if (IsKeyDown(KEY_LEFT)) player.rect.x -= player.speed;
        else if (IsKeyDown(KEY_UP)) player.rect.y -= player.speed;
        else if (IsKeyDown(KEY_DOWN)) player.rect.y += player.speed;

        // Check collision
        if (IsKeyPressed(KEY_SPACE)) {

            if (CheckCollisionRecs(player.rect, deckRect) &&
                                    player.numHitsLeft > 0 &&
                                    !card3.faceUp &&
                                    !player.bust) {
                TryPickupCard(&cardHolding, &card3);
                player.numHitsLeft--;
            }

            else if (card2.isColliding(player.rect) && !card2.faceUp) {
                if (card2.isColliding(cardSlot2)) {
                    card2.Reveal();
                    player.cardsValue += card2.value;
                }
                TryPickupCard(&cardHolding, &card2);
            }

            else if (card3.isColliding(player.rect) && !card3.faceUp) {
                if (card3.isColliding(cardSlot3)) {
                    card3.Reveal();
                    player.cardsValue += card3.value;
                }
                TryPickupCard(&cardHolding, &card3);
            }

            player.bust = (player.cardsValue > 21);
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
            player.numHitsLeft = player.numHitsDefault;
            player.cardsValue = card1.value;
            player.bust = false;
        }
        if (cardHolding != nullptr){
            (*cardHolding).UpdatePosition((Vector2){player.rect.x, player.rect.y});
        }


        // Draw ----------------------------------------------------------------------------------
        camera.target = (Vector2){ player.rect.x, player.rect.y };
        BeginDrawing();
        {
            camera.BeginMode();
            {
                window.ClearBackground(BLACK);

                // Draw card slots
                DrawRectangleLinesEx(cardSlot1, 2, card1.color);
                DrawRectangleLinesEx(cardSlot2, 2, card2.color);
                DrawRectangleLinesEx(cardSlot3, 2, card3.color);

                // Draw deck
                DrawTexturePro(deckTexture, deckSource, deckRect, (Vector2){0,0}, 0, GRAY);
                const char* cardsInDeck = (to_string(player.numHitsLeft)).c_str();
                DrawText(cardsInDeck,
                        deckRect.x + 42,
                        deckRect.y + 25,
                        50, WHITE);

                // Draw player
                DrawRectangleRec(player.rect, RED);

                // Draw cards
                card1.Draw();
                card2.Draw();
                card3.Draw();
            }
            camera.EndMode();

            // Draw info
            DrawText(timerText, screenWidth / 100, screenHeight / 100, 50, WHITE);
            string playerCardsValueText = "Value: " + to_string(player.cardsValue);
            if (player.bust) playerCardsValueText += " (YOU BUSTED!)";
            DrawText(playerCardsValueText.c_str(), screenWidth / 100, screenHeight / 100 + 50, 25, WHITE);
        }
        EndDrawing();
    }

    return 0;
}