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
    int money;
    Color color;
    int wins;

    Player(Rectangle r, Color c){
        rect = r;
        speed = 0.5f;
        cardsValue = 0;
        numHitsDefault = 3;
        numHitsLeft = numHitsDefault;
        bust = false;
        money = 0;
        color = c;
        wins = 0;
    }
};

struct Card {
    Image sprite;
    Texture2D cardTexture;
    Rectangle rect;    // the actual rect rect of the card
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

    int Reveal() {
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

            return value;
        }

        return 0;
    }

    void Draw() {
        //DrawRectangleRec(rect, BLUE);

        // Highlight color if player is moving card, else white
        Color colorHighlight = (connected) ? color : WHITE;

        DrawTexturePro(cardTexture,
                        source,
                        rect,
                        (Vector2){0, 0},
                        0.0,
                        colorHighlight);
    }

    void UpdatePosition(Vector2 pos) {
        rect = (Rectangle){pos.x - source.width / 2, pos.y - source.width / 2, source.width, source.height};
    }

    bool isColliding(Rectangle rectB) {
        return CheckCollisionRecs(rect, rectB);
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

void TryPlayBotCard(Card& card, Player& player, Player& bot) {
    if (!player.bust &&
        bot.cardsValue != 21 &&
        bot.cardsValue < player.cardsValue) {

        bot.cardsValue += card.Reveal();
        bot.bust = (bot.cardsValue > 21);
    }
}

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

    // Player variables
    Player player = Player((Rectangle){ 150, 250, 40, 40 }, RED);
    Player bot = Player((Rectangle){ 150, -250, 40, 40 }, YELLOW);

    // Initialize cards

    // Player cards
    Card card1 = Card((Vector2){10000, 10000}, BLUE);
    Card card2 = Card((Vector2){10000, 10000}, RED);
    Card card3 = Card((Vector2){10000, 10000}, GREEN); // "hit" card should not be seen until deck is clicked
    Card* cardHolding = nullptr;

    // Player card slots
    float spacing = 150;
    Rectangle cardSlot1 = {-45, -20,
                            card1.rect.width - 10,
                            card1.rect.height
                        };
    Rectangle cardSlot2 = cardSlot1;
    cardSlot2.x += spacing;
    Rectangle cardSlot3 = cardSlot2;
    cardSlot3.x += spacing;

    // Bot cards
    Card botCard1 = Card((Vector2){0, -110}, WHITE);
    bot.cardsValue += botCard1.Reveal();
    Card botCard2 = Card((Vector2){botCard1.rect.x + spacing * (float)1.35, -110}, WHITE);
    Card botCard3 = Card((Vector2){botCard2.rect.x + spacing * (float)1.35, -110}, WHITE); // "hit" card should not be seen until deck is clicked    Card* cardHolding = nullptr;

    // Bot card slots
    Rectangle botCardSlot1 = {botCard1.startingPos.x - 45,
                            botCard1.startingPos.y - 50,
                            botCard1.rect.width - 10,
                            botCard1.rect.height
                        };
    Rectangle botCardSlot2 = botCardSlot1;
    botCardSlot2.x += spacing;
    Rectangle botCardSlot3 = botCardSlot2;
    botCardSlot3.x += spacing;

    // Initialize camera
    raylib::Camera2D camera(
        raylib::Vector2(0, 0),  // position
        raylib::Vector2(player.rect.x, player.rect.y)  // target
    );
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.zoom = 0.75;

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
                                    cardHolding == nullptr &&
                                    player.numHitsLeft > 0 &&
                                    !player.bust) {
                if (!card1.faceUp) TryPickupCard(&cardHolding, &card1);
                else if (!card2.faceUp) TryPickupCard(&cardHolding, &card2);
                else if (!card3.faceUp) TryPickupCard(&cardHolding, &card3);
                player.numHitsLeft--;
            }

            else if (card1.isColliding(player.rect) && !card1.faceUp) {
                if (card1.isColliding(cardSlot1)) player.cardsValue += card1.Reveal();
                TryPickupCard(&cardHolding, &card1);
            }

            else if (card2.isColliding(player.rect) && !card2.faceUp) {
                if (card2.isColliding(cardSlot2)) player.cardsValue += card2.Reveal();
                TryPickupCard(&cardHolding, &card2);
            }

            else if (card3.isColliding(player.rect) && !card3.faceUp) {
                if (card3.isColliding(cardSlot3)) player.cardsValue += card3.Reveal();
                TryPickupCard(&cardHolding, &card3);
            }

            player.bust = (player.cardsValue > 21);
        }

        // Update variables ----------------------------------------------------------------------------------
        timer -= window.GetFrameTime();
        timerText = (to_string(((int)timer)+1)).c_str(); // counts 10 to 1, converted to C string;

        // Make bot decision to play card
        if (timer < 6.0) TryPlayBotCard(botCard2, player, bot);
        if (timer < 3.0) TryPlayBotCard(botCard3, player, bot);
        
        if (timer <= 0) {
            if (player.cardsValue > bot.cardsValue && !player.bust) player.wins++;
            else bot.wins++;

            card1.Reset();
            card2.Reset();
            card3.Reset();

            botCard1.Reset();
            bot.cardsValue = botCard1.Reveal();
            botCard2.Reset();
            botCard3.Reset();

            timer = cardResetTimer;
            cardHolding = nullptr;

            bot.numHitsLeft = bot.numHitsDefault;
            bot.bust = false;

            player.numHitsLeft = player.numHitsDefault;
            player.cardsValue = 0;
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
                DrawRectangleLinesEx(cardSlot1, 2, card1.color); // Player
                DrawRectangleLinesEx(cardSlot2, 2, card2.color);
                DrawRectangleLinesEx(cardSlot3, 2, card3.color);
                DrawRectangleLinesEx(botCardSlot1, 2, WHITE);   // Bot
                DrawRectangleLinesEx(botCardSlot2, 2, WHITE);
                DrawRectangleLinesEx(botCardSlot3, 2, WHITE);

                // Draw deck
                DrawTexturePro(deckTexture, deckSource, deckRect, (Vector2){0,0}, 0, GRAY);
                const char* cardsInDeck = (to_string(player.numHitsLeft)).c_str();
                DrawText(cardsInDeck,
                        deckRect.x + 42,
                        deckRect.y + 25,
                        50, WHITE);

                // Draw player(s)
                DrawRectangleRec(player.rect, player.color);
                DrawRectangleRec(bot.rect, bot.color);

                // Draw cards
                card1.Draw(); // Player
                card2.Draw();
                card3.Draw();
                botCard1.Draw(); // Bot
                botCard2.Draw();
                botCard3.Draw();
            }
            camera.EndMode();

            // Draw info
            DrawText(timerText, window.GetWidth() / 2 - 50, screenHeight / 100, 100, RED);

            // Player value
            string playerCardsValueText = "Your Value: " + to_string(player.cardsValue);
            if (player.bust) playerCardsValueText += " \n\nYOU BUSTED!)";
            DrawText(playerCardsValueText.c_str(), screenWidth / 100, screenHeight / 100, 25, WHITE);

            // Player wins
            string playerWinsText = "Wins: " + to_string(player.wins);
            DrawText(playerWinsText.c_str(), screenWidth / 100, screenHeight / 100 + 30, 25, WHITE);

            // Bot value
            string botCardsValueText = "Dealer Value: " + to_string(bot.cardsValue);
            if (bot.bust) botCardsValueText += " \n\n(BOT BUSTED!)";
            DrawText(botCardsValueText.c_str(), window.GetWidth() - 220, screenHeight / 100, 25, WHITE);

            // Bot wins
            string botWinsText = "Wins: " + to_string(bot.wins);
            DrawText(botWinsText.c_str(), window.GetWidth() - 110, screenHeight / 100 + 30, 25, WHITE);
        }
        EndDrawing();
    }

    return 0;
}