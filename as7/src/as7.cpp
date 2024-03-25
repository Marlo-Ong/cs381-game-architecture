// Author: Marlo Ongkingco
// CS381 Game Engine Architecture
// AS4 Ad-Hoc Game: Stupid Blackjack

#include "raylib-cpp.hpp"
#include <iostream>
#include <string>
using namespace std;

struct Player {
    Rectangle rect;
    float speed;
    float velocityX;
    float velocityY;

    int cardsValue;
    int numHitsDefault;
    int numHitsLeft;
    bool bust;
    int money;
    Color color;
    int wins;
    int result;

    Player(Rectangle r, Color c){
        rect = r;
        speed = 2.5;
        cardsValue = 0;
        numHitsDefault = 3;
        numHitsLeft = numHitsDefault;
        bust = false;
        money = 500;
        color = c;
        wins = 0;
        velocityX = 0;
        velocityY = 0;
        result = money;
    }
};

struct Card {
    Image sprite;
    Texture2D cardTexture;
    Rectangle rect;    // the actual rect rect of the card
    Rectangle source;       // rectangular crop of the image asset; does not change since using full picture

    bool connected;         // player is holding this card
    bool faceUp;
    bool drawn;
    Vector2 startingPos;
    int value;
    string suit;
    string name;            // string "value"
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

            name = randVal;
            string filepath = "../cards/card-" + randVal + "-" + randSuit + ".png";
            SetImageTexture(filepath);
            faceUp = true;

            return value;
        }

        return 0;
    }

    void Draw() {
        //DrawRectangleRec(rect, BLUE);

        Color colorHighlight, baseColor; 
        baseColor = (value == 11 && faceUp) ? YELLOW : WHITE;   // Highlight yellow if ACE is toggled to 11
        colorHighlight = (connected) ? (Color){ color.r, color.g, color.b, 200 } : baseColor;       // Highlight color if picked up - overrides all

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
        if (CheckCollisionRecs(rect, rectB)) {
            Rectangle boxCollision = GetCollisionRec(rect, rectB);
            float boxArea = boxCollision.width * boxCollision.height;
            float slotArea = rectB.width * rectB.height;
            if (boxArea >= 0.5 * slotArea) return true; // card must be at least 50% within the slot
        }
        return false;
    }

    void Reset() {
        connected = false;
        faceUp = false;
        drawn = false;
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

    int ToggleAceValue() {
        if (value == 1) value = 11;
        else if (value == 11) value = 1;
        return value;
    }
};

void TryPlayBotCard(Card& card, Player& player, Player& bot) {
    if (!card.faceUp && !player.bust &&
        bot.cardsValue != 21 &&
        bot.cardsValue < player.cardsValue) {

        bot.cardsValue += card.Reveal();
    }

    // Bot toggles ace favorably
    if (card.name == "ace" && card.faceUp) {
        if (bot.bust && card.value == 11) {
            bot.cardsValue -= card.value;
            bot.cardsValue += card.ToggleAceValue();
        }
        else if (bot.cardsValue + 10 <= 21 && card.value == 1) {
            bot.cardsValue -= card.value;
            bot.cardsValue += card.ToggleAceValue();
        }
    }

    bot.bust = (bot.cardsValue) > 21;
}

void TryPickupCard(Card** holder, Card* cardPicked) {
    if ((*holder) != nullptr) {
        (**holder).connected = false;
        (*holder) = nullptr;
    }
    else {
        (*holder) = cardPicked;
        (*cardPicked).connected = true;
        (*cardPicked).drawn = true;
    }
}

string GetResultText(int result) {
    return (result < 0) ? "-$" + to_string(abs(result)) : "+$" + to_string(abs(result));
}

raylib::Color GetResultColor(int result) {
    raylib::Color resultColor = GREEN;
    if (result < 0) resultColor = RED;
    else if (result == 0) resultColor = GRAY;
    return resultColor;
}

int main()
{
    // Initialize window
    int screenWidth = 600;
    int screenHeight = 500;
    raylib::Window window(screenWidth, screenHeight, "CS381 Assignment 7");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    // Audio
    InitAudioDevice();
    raylib::Sound mus_BG("../audio/bg.mp3");
    raylib::Sound sfx_boing("../audio/boing.wav");
    raylib::Sound sfx_sell("../audio/sell.mp3");
    raylib::Sound sfx_draw("../audio/draw.mp3");
    raylib::Sound sfx_place("../audio/place.mp3");
    raylib::Sound sfx_win("../audio/money.mp3");
    raylib::Sound sfx_cheer("../audio/cheer.wav");
    raylib::Sound sfx_lose("../audio/lose.mp3");
    raylib::Sound sfx_bust("../audio/bust.mp3");
    mus_BG.Play();

    // Player variables
    Player player = Player((Rectangle){ 200, 250, 40, 40 }, RED);
    Player bot = Player((Rectangle){ 100, -250, 40, 40 }, YELLOW);
    int wager = 100;  // $100

    // Initialize cards

        // Player cards
        Card card1 = Card((Vector2){10000, 10000}, BLUE);
        Card card2 = Card((Vector2){10000, 10000}, RED);
        Card card3 = Card((Vector2){10000, 10000}, GREEN); // "hit" card should not be seen until deck is clicked
        Card* heldCard = nullptr;

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
        Card botCard3 = Card((Vector2){botCard2.rect.x + spacing * (float)1.35, -110}, WHITE); // "hit" card should not be seen until deck is clicked    Card* heldCard = nullptr;

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

    // Initialize timers
    float cardResetTimer = 10;
    float timer = cardResetTimer;
    const char* timerText = "0";
    float msgTimer = 0; // 1-second timer displaying bot's message


    // Miscellaneous items - deck, table, game borders
    Image deckSprite = LoadImage("../icons/card-draw.png");
    ImageResize(&deckSprite, 100, 100);
    Texture2D deckTexture = LoadTextureFromImage(deckSprite);
    UnloadImage(deckSprite);
    Rectangle deckSource = { 0, 0, (float)deckTexture.width, (float)deckTexture.height };
    Rectangle deckRect = { 300, 200, deckSource.width, deckSource.height };

    Image sellSprite = LoadImage("../icons/sell-card.png");
    ImageResize(&sellSprite, 100, 100);
    Texture2D sellTexture = LoadTextureFromImage(sellSprite);
    UnloadImage(sellSprite);
    Rectangle sellSource = { 0, 0, (float)sellTexture.width, (float)sellTexture.height };
    Rectangle sellRect = { -100, 200, sellSource.width, sellSource.height };

    Rectangle table = {-100, -200, 500, 350};
    Rectangle gameBorders = {-350, -450, 1000, 800};
    Rectangle gameBordersVisual = {gameBorders.x - 2 * player.rect.width,
                                gameBorders.y - 2 *player.rect.height,
                                gameBorders.width + 4 * player.rect.width,
                                gameBorders.height + 4 * player.rect.height};
    Vector2 center = {150, -50};

    // Titlescreen variables
    raylib::Color screenColor = {255, 255, 255, 255};
    float titleTimer = 0;
    float a = 1; // Alpha
    int scene = 0;
    float transitionDuration = 3; 
    bool transition = false;

    while (!WindowShouldClose())
    {    
        if (scene == 0) { // Title Screen Cutscene -----------------------------------------------------------------
            if (IsKeyDown(KEY_SPACE)) transition = true;

            if (transition) {
                titleTimer += window.GetFrameTime();
                a = 1 - (titleTimer / transitionDuration);
                if (titleTimer > transitionDuration) {
                    transition = false;
                    scene = 1;
                }
            }

            // Increase/decrease wager
            if (!transition && IsKeyPressed(KEY_DOWN)) wager = (wager <= 50) ? 50 : wager - 50;
            if (!transition && IsKeyPressed(KEY_UP)) wager += 50;
        }
        Rectangle fullScreen = {0, 0, (float)window.GetWidth(), (float)window.GetHeight()}; // Resize canvas if necessary

        if (scene == 1) { // Main Game Scene ----------------------------------------------------------------------
            // Player movement
            if (IsKeyDown(KEY_RIGHT)) player.velocityX += player.speed;
            else if (IsKeyDown(KEY_LEFT)) player.velocityX -= player.speed;
            if (IsKeyDown(KEY_UP)) player.velocityY -= player.speed;
            else if (IsKeyDown(KEY_DOWN)) player.velocityY += player.speed;

            // Move player continuously
            player.rect.x += player.velocityX * window.GetFrameTime();
            player.rect.y += player.velocityY * window.GetFrameTime();

            // Friction
            player.velocityX *= 0.995;
            player.velocityY *= 0.995;

            // If out of bounds, bounce back (point velocity to center)
            if (!CheckCollisionRecs(gameBorders, player.rect)) {
                player.velocityX = center.x - player.rect.x;
                player.velocityY = center.y - player.rect.y;
            }

            // Touch NPC = bounce
            if (CheckCollisionRecs(player.rect, bot.rect)) {
                player.velocityX *= -1.01;
                player.velocityY *= -1.01;
                sfx_boing.Play();
            }

            // User input
            if (IsKeyPressed(KEY_SPACE)) {
                // Try getting card from deck
                if (CheckCollisionRecs(player.rect, deckRect) &&
                                        heldCard == nullptr &&
                                        player.numHitsLeft > 0 &&
                                        !player.bust) {
                    if (!card1.drawn) TryPickupCard(&heldCard, &card1);
                    else if (!card2.drawn) TryPickupCard(&heldCard, &card2);
                    else if (!card3.drawn) TryPickupCard(&heldCard, &card3);
                    player.numHitsLeft--;
                    sfx_draw.Play();
                }

                // Try selling card
                if (CheckCollisionRecs(player.rect, sellRect) && heldCard != nullptr) {
                    player.money += wager / 5;
                    (*heldCard).UpdatePosition((Vector2){10000, 10000});
                    heldCard = nullptr;
                    sfx_sell.Play();
                }

                // Interact card 1
                else if (card1.isColliding(player.rect) && !card1.faceUp) {
                    if (card1.isColliding(cardSlot1)) player.cardsValue += card1.Reveal();
                    TryPickupCard(&heldCard, &card1);
                    sfx_place.Play();
                }

                // Interact card 2
                else if (card2.isColliding(player.rect) && !card2.faceUp) {
                    if (card2.isColliding(cardSlot2)) player.cardsValue += card2.Reveal();
                    TryPickupCard(&heldCard, &card2);
                    sfx_place.Play();
                }

                // Interact card 3
                else if (card3.isColliding(player.rect) && !card3.faceUp) {
                    if (card3.isColliding(cardSlot3)) player.cardsValue += card3.Reveal();
                    TryPickupCard(&heldCard, &card3);
                    sfx_place.Play();
                }

                // Toggle ace values
                if (card1.isColliding(player.rect) && card1.name == "ace" && card1.faceUp) {
                    player.cardsValue -= card1.value;
                    player.cardsValue += card1.ToggleAceValue();
                }
                if (card2.isColliding(player.rect) && card2.name == "ace" && card2.faceUp) {
                    player.cardsValue -= card2.value;
                    player.cardsValue += card2.ToggleAceValue();
                }
                if (card3.isColliding(player.rect) && card3.name == "ace" && card3.faceUp) {
                    player.cardsValue -= card3.value;
                    player.cardsValue += card3.ToggleAceValue();
                }

                player.bust = (player.cardsValue > 21);
                if (player.bust) sfx_bust.Play();
            }

            // Update variables ----------------------------------------------------------------------------------
            timer -= window.GetFrameTime();
            msgTimer -= window.GetFrameTime();
            timerText = (to_string(((int)timer)+1)).c_str(); // counts 10 to 1, converted to C string;

            // Make bot decision to play card
            if (timer < 6.0) TryPlayBotCard(botCard2, player, bot);
            if (timer < 3.0) TryPlayBotCard(botCard3, player, bot);
            
            if (timer <= 0) {

                // Evaluate winnings
                if ((!player.bust) && (bot.bust || player.cardsValue > bot.cardsValue)) {
                    if (player.cardsValue == 21) player.result = 2 * wager;
                    else player.result = wager / 2;
                    bot.result = -wager;
                    sfx_win.Play();
                }
                else if ((!bot.bust) && (player.bust || player.cardsValue < bot.cardsValue)) {
                    if (bot.cardsValue == 21) bot.result = 2 * wager;
                    else bot.result = wager / 2;
                    player.result = -wager;
                    sfx_lose.Play();
                }
                else {
                    player.result = 0;
                    bot.result = 0;
                }
                player.money += player.result;
                bot.money += bot.result;
                sfx_cheer.Play();

                // Reset card values and locations
                card1.Reset();
                card2.Reset();
                card3.Reset();

                botCard1.Reset();
                bot.cardsValue = botCard1.Reveal();
                botCard2.Reset();
                botCard3.Reset();


                // Reset players variables
                timer = cardResetTimer;
                heldCard = nullptr;

                bot.numHitsLeft = bot.numHitsDefault;
                bot.bust = false;

                player.numHitsLeft = player.numHitsDefault;
                player.cardsValue = 0;
                player.bust = false;
            }

            // Make held card (if exists) follow player 
            if (heldCard != nullptr){
                (*heldCard).UpdatePosition((Vector2){player.rect.x, player.rect.y});
            }
        }

        // Draw ----------------------------------------------------------------------------------
        camera.target = (Vector2){ player.rect.x, player.rect.y };
        BeginDrawing();
        {
            camera.BeginMode();
            {
                window.ClearBackground(BLACK);

                // Draw table and game borders
                DrawRectangleRec(table, DARKGREEN);
                DrawRectangleLinesEx(table, 5, BROWN);
                DrawRectangleLinesEx(gameBordersVisual, player.rect.width, BROWN);

                // Draw card slots
                DrawRectangleLinesEx(cardSlot1, 2, card1.color); // Player
                DrawRectangleLinesEx(cardSlot2, 2, card2.color);
                DrawRectangleLinesEx(cardSlot3, 2, card3.color);
                DrawRectangleLinesEx(botCardSlot1, 2, WHITE);   // Bot
                DrawRectangleLinesEx(botCardSlot2, 2, WHITE);
                DrawRectangleLinesEx(botCardSlot3, 2, WHITE);

                // Draw deck
                DrawTexturePro(deckTexture, deckSource, deckRect, (Vector2){0,0}, 0, GRAY);
                string cardsInDeck = to_string(player.numHitsLeft);
                DrawText(cardsInDeck.c_str(), deckRect.x + 42, deckRect.y + 25, 50, WHITE);
                DrawText("draws:", deckRect.x + 20, deckRect.y + 100, 25, GRAY);

                DrawTexturePro(sellTexture, sellSource, sellRect, (Vector2){0,0}, 0, GRAY);
                string sellValueStr = "sell card (+$" + to_string(wager / 5) + ")";
                DrawText(sellValueStr.c_str(), sellRect.x - 50, sellRect.y + 100, 25, GRAY);

                // Draw cards
                card1.Draw(); // Player
                card2.Draw();
                card3.Draw();
                botCard1.Draw(); // Bot
                botCard2.Draw();
                botCard3.Draw();

                // Draw player(s)
                DrawRectangleRec(bot.rect, bot.color);
                DrawRectangleRec(player.rect, player.color);

                // Bot message timer logic
                if (CheckCollisionRecs(player.rect, bot.rect) && msgTimer < 0) {
                    msgTimer = 0.5;
                    player.money += 1; // steal $1
                    bot.money -= 1;
                }
                if (msgTimer > 0) DrawText("Watch it, bud!", bot.rect.x - 60, bot.rect.y - 40, 25, WHITE);
            }
            camera.EndMode();

            // Draw info
            DrawText(timerText, window.GetWidth() / 2 - 50, screenHeight / 100, 100, RED);

            // Player value
            string playerCardsValueText = "Your Value: " + to_string(player.cardsValue);
            if (player.bust) playerCardsValueText += " \n\nYOU BUSTED!)";
            DrawText(playerCardsValueText.c_str(), screenWidth / 100, screenHeight / 100, 25, WHITE);

            // Player money
            string playerWinsText = "Money: $" + to_string(player.money);
            DrawText(playerWinsText.c_str(), screenWidth / 100, screenHeight / 100 + 30, 25, WHITE);

            // Result texts
            float resultDuration = 6; // Normalized fade out from 10 to X seconds
            if (timer > resultDuration) {
                string playerResultText = GetResultText(player.result);
                raylib::Color playerResultColor = GetResultColor(player.result);
                string botResultText = GetResultText(bot.result);
                raylib::Color botResultColor = GetResultColor(bot.result);

                float alph = 1 - ((cardResetTimer - timer) / (cardResetTimer - resultDuration));

                DrawText(playerResultText.c_str(),
                            screenWidth / 100,
                            screenHeight / 100 + 60, 
                            25, playerResultColor.Alpha(alph));
                DrawText(botResultText.c_str(),
                            window.GetWidth() - 210,
                            screenHeight / 100 + 60, 
                            25, botResultColor.Alpha(alph));
            }
            
            // Bot value
            string botCardsValueText = "Dealer Value: " + to_string(bot.cardsValue);
            if (bot.bust) botCardsValueText += " \n\n(BOT BUSTED!)";
            DrawText(botCardsValueText.c_str(), window.GetWidth() - 210, screenHeight / 100, 25, WHITE);

            // Bot money
            string botWinsText = "Money: $" + to_string(bot.money);
            DrawText(botWinsText.c_str(), window.GetWidth() - 210, screenHeight / 100 + 30, 25, WHITE);

            if (scene == 0) {
                DrawRectangleRec(fullScreen, screenColor.Alpha(a));
                raylib::Color blackAlpha = BLACK;
                DrawText("Stupid Blackjack", window.GetWidth() / 2 - 200, window.GetHeight() / 2 - 100, 50, ((raylib::Color)BLACK).Alpha(a));
                DrawText("(press SPACE to start\ngambling your life away!)", window.GetWidth() / 2 - 150, window.GetHeight() / 2, 25, ((raylib::Color)GRAY).Alpha(a));
                string wagerStr = "Wager: $" + to_string(wager);
                DrawText(wagerStr.c_str(), window.GetWidth() / 2 - 100, window.GetHeight() / 2 + 100, 25, ((raylib::Color)GREEN).Alpha(a));
                DrawText("(Press UP or DOWN to change your bet.)", window.GetWidth() / 2 - 250, window.GetHeight() / 2 + 150, 25, ((raylib::Color)GRAY).Alpha(a));
            }
        }
        EndDrawing();
    }
    return 0;
}