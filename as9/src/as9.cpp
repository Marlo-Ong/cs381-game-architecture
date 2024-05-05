#include "raylib-cpp.hpp"
#include <concepts>
#include <memory>
#include <optional>
#include <iostream>
#include <BufferedInput.hpp>
#include "ECS.hpp"
#include <iostream>
using namespace std;

void SetImageTexture(string, Texture2D&);

struct TransformComponent {
    Rectangle source;
    Rectangle dest;
    Vector2 origin; // anchor to middle of rect (assume square with w=100, h=100)
    float rotation = 0;
};

struct RenderingComponent {
    Color color;
    Texture2D texture;
};

struct InputComponent {
    raylib::BufferedInput inputs;
};

struct CardComponent {
    string value;
    string suit;
    int numAttempts;

    bool operator==(CardComponent& rhs) {
        return rhs.value == value;
    }
};

struct PlayerComponent {
    int id;
    cs381::Entity cards[52];
    int numCards = 0;
    int attemptsLeft;
    bool dead = false;
};

struct DeckComponent {
    cs381::Entity cards[52];
    int numCards = 0;
};

struct CardCountTextComponent {
    string text;
    int fontSize;
};

void DrawSystem(cs381::Scene<>& scene) {
    for(auto [rendering, transform]: cs381::SceneView<RenderingComponent, TransformComponent>{scene}) {
        DrawTexturePro(rendering.texture,
                        transform.source,
                        transform.dest,
                        transform.origin,
                        transform.rotation,
                        rendering.color);
    }

    // Draw cards on top
    for(auto [deck]: cs381::SceneView<DeckComponent>{scene}) {
        for (int i = 0; i < deck.numCards; i++) {
            RenderingComponent& rendering = scene.GetComponent<RenderingComponent>(deck.cards[i]);
            TransformComponent& transform = scene.GetComponent<TransformComponent>(deck.cards[i]);
            DrawTexturePro(rendering.texture,
                            transform.source,
                            transform.dest,
                            transform.origin,
                            transform.rotation,
                            rendering.color);
        }
    }

    for(auto [text, rendering, transform]: cs381::SceneView<CardCountTextComponent, RenderingComponent, TransformComponent>{scene}) {
        DrawText(text.text.c_str(), transform.dest.x, transform.dest.y, text.fontSize, rendering.color);
    }

    DrawFPS(10, 10);
}

void CardRandomAssignmentSystem(cs381::Scene<>& scene) {
    static string suits[4] = {"hearts", "spades", "diamonds", "clubs"};
    static string values[13] = {"ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king"};

    for(auto [card, rendering]: cs381::SceneView<CardComponent, RenderingComponent>{scene}) {

        // Set suit and value randomly
        string randSuit = suits[GetRandomValue(0, 3)];
        card.suit = randSuit;

        int randValIndex = GetRandomValue(0, 12);
        string randVal = values[randValIndex];
        card.value = values[randValIndex];

        // Set attempt count if face card
        card.numAttempts = 0;
        switch (card.value[0])
        {
            case 'a':
                card.numAttempts++;
            case 'k':
                card.numAttempts++;
            case 'q':
                card.numAttempts++;
            case 'j':
                card.numAttempts++;
                break;

            default:
                break;
        }

        // Set image of card
        string filepath = "../cards/card-" + randVal + "-" + randSuit + ".png";
        cout << "Set card: " << filepath << endl;
        SetImageTexture(filepath, rendering.texture);
    }
}

void TextUpdateSystem(cs381::Scene<>& scene) {
    for (auto [player, text] : cs381::SceneView<PlayerComponent, CardCountTextComponent>{scene}) {
        text.text = to_string(player.numCards);
    }

    for (auto [deck, text] : cs381::SceneView<DeckComponent, CardCountTextComponent>{scene}) {
        text.text = to_string(deck.numCards);
    }
}

bool SlapSystem(cs381::Scene<>& scene, PlayerComponent currentPlayer) {
    
    // Attempt slap
    for (auto [deck, _, transform] : cs381::SceneView<DeckComponent, InputComponent, TransformComponent>{scene}) {
        if (deck.numCards >= 3) {
            CardComponent& topCard = scene.GetComponent<CardComponent>(deck.cards[deck.numCards - 1]);
            CardComponent& secondTopCard = scene.GetComponent<CardComponent>(deck.cards[deck.numCards - 2]);
            CardComponent& thirdTopCard = scene.GetComponent<CardComponent>(deck.cards[deck.numCards - 3]);

            // Check double or sandwich
            if (topCard == secondTopCard || topCard == thirdTopCard) {
                cout << "Slapped!" << endl;
                return true;
            }
        }
    }

    return false;
}

bool GameOverCheckSystem(cs381::Scene<>& scene) {
    for (auto [player] : cs381::SceneView<PlayerComponent>{scene}) {
        if (player.numCards > 0) return false;
    }
    return true;
}

bool HandClickSystem(cs381::Scene<>& scene, PlayerComponent currentPlayer) {
    Vector2 mousePos = GetMousePosition();
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Clicked hand
        for (auto [player, _, playerTransform] : cs381::SceneView<PlayerComponent, InputComponent, TransformComponent>{scene}) {
            if (currentPlayer.id == player.id && CheckCollisionPointRec(mousePos, playerTransform.dest)) {
                // Place player card onto deck
                for (auto [deck, _, deckTransform] : cs381::SceneView<DeckComponent, InputComponent, TransformComponent>{scene}) {
                    if (player.numCards > 0 && player.attemptsLeft > 0) {
                        cout << "Player " << player.id << " tried placing card" << endl;

                        // Copy top card of player to top of deck
                        deck.cards[deck.numCards] = player.cards[player.numCards - 1];

                        bool isFaceCard = scene.GetComponent<CardComponent>(deck.cards[deck.numCards]).numAttempts > 0;
                        player.attemptsLeft = isFaceCard ? 0 : player.attemptsLeft - 1;

                        // Display card on top
                        Rectangle newDest = {deckTransform.dest.x + (deck.numCards * 5), deckTransform.dest.y, 100, 100};
                        scene.GetComponent<TransformComponent>(deck.cards[deck.numCards]).dest = newDest;

                        deck.numCards++;
                        player.numCards--;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
// MAIN AREA //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int main() {
    // Create window
    const int screenWidth = 800;
    const int screenHeight = 450;
    raylib::Window window(screenWidth, screenHeight, "CS381 - Assignment 9");

    cs381::Scene<> scene;

    // Define game params
    int totalNumCards = 52;
    int numPlayers = 4;

    // Create camera
    raylib::Camera2D camera(
        raylib::Vector2(0, 0), // position
        raylib::Vector2(0, 0)  // target
    );

    // Create deck
    auto deckEntity = scene.CreateEntity();
    Texture2D deckTexture;
    SetImageTexture("../icons/card-draw.png", deckTexture);
    Rectangle deckSource = { 0, 0, (float)deckTexture.width, (float)deckTexture.height };
    Rectangle deckRect = { screenWidth / 2 - 50, screenHeight / 2 - 50, deckSource.width, deckSource.height };
    scene.AddComponent<RenderingComponent>(deckEntity) = {GRAY, deckTexture};
    scene.AddComponent<TransformComponent>(deckEntity) = {deckSource, deckRect};
    scene.AddComponent<InputComponent>(deckEntity);
    scene.AddComponent<CardCountTextComponent>(deckEntity) = {"52", 15};
    DeckComponent& deck = scene.AddComponent<DeckComponent>(deckEntity);

    // Create cards
    for (int i = 0; i < totalNumCards; i++)
    {
        auto card = scene.CreateEntity();
        deck.cards[i] = card;
        deck.numCards++;
        scene.AddComponent<RenderingComponent>(card) = {WHITE};
        scene.AddComponent<CardComponent>(card);

        Rectangle source = {0.0, 0.0, 100.0, 100.0};
        Rectangle dest = {1000, 0.0, 100.0, 100.0};
        scene.AddComponent<TransformComponent>(card) = {source, dest};
    }
    CardRandomAssignmentSystem(scene);
    cout << "Card assignment complete" << endl << endl;

    // Create players
    Color playerColors[4] = {RED, BLUE, GREEN, YELLOW};
    float anchorX[4] = {0.20, 0.20, 0.80, 0.80};
    float anchorY[4] = {0.20, 0.80, 0.20, 0.80};
    cs381::Entity players[4];

    for (int i = 0; i < numPlayers; i++)
    {
        auto player = scene.CreateEntity();
        players[i] = player;
        scene.AddComponent<RenderingComponent>(player) = {playerColors[i], deckTexture};
        scene.AddComponent<PlayerComponent>(player) = {i};
        scene.AddComponent<InputComponent>(player);

        Rectangle source = {0.0, 0.0, 100.0, 100.0};
        float destWidth = screenWidth * anchorX[i] - 50;
        float destHeight = screenHeight * anchorY[i] - 50;
        Rectangle dest = {destWidth, destHeight, 100.0, 100.0};
        cout << "Create player at: " << destWidth << ", " << destHeight << endl;
        scene.AddComponent<TransformComponent>(player) = {source, dest};
        scene.AddComponent<CardCountTextComponent>(player) = {"0", 15};
    }

    // Assign cards to players
    for (auto [player] : cs381::SceneView<PlayerComponent>{scene}) {
        for (int i = 0; i < floor(totalNumCards / numPlayers); i++) {
            player.cards[i] = deck.cards[deck.numCards];
            deck.numCards--;
        }
        player.numCards = floor(totalNumCards / numPlayers);
    }

    // Play audio
    InitAudioDevice();
    raylib::Sound mus_BG("../audio/bg.mp3");
    raylib::Sound SFX_lose("../audio/bust.mp3");
    raylib::Sound SFX_putCard("../audio/draw.mp3");
    raylib::Sound SFX_slap("../audio/place.mp3");
    raylib::Sound SFX_cheer("../audio/cheer.wav");
    mus_BG.Play();

	// Main loop
	bool keepRunning = true;
    int currentPlayerIndex = 0;
    scene.GetComponent<PlayerComponent>(players[currentPlayerIndex]).attemptsLeft = 1;

	while(!window.ShouldClose() && keepRunning) {
        for (auto [player, rendering] : cs381::SceneView<PlayerComponent, RenderingComponent>{scene}) {
            if (player.id == currentPlayerIndex) {
                rendering.color = WHITE;
                if (HandClickSystem(scene, player)) SFX_putCard.Play();

                if (IsKeyPressed(KEY_SPACE) && deck.numCards >= 3) {
                    SFX_slap.Play();
                    if (SlapSystem(scene, player)) {
                        SFX_cheer.Play();
                        // Transfer whole deck into player hand
                        for (int i = 0; i < deck.numCards; i++) {
                            player.cards[player.numCards] = deck.cards[i];
                            scene.GetComponent<TransformComponent>(deck.cards[i]).dest.x = 1000;
                            player.numCards++;
                        }
                        deck.numCards = 0;
                        player.attemptsLeft = 1;
                    }

                    else {
                        player.numCards--;
                        SFX_lose.Play();
                    }
                }

                if (GameOverCheckSystem(scene)) {
                    keepRunning = false;
                };

                if (player.attemptsLeft == 0 || player.numCards == 0) {
                    currentPlayerIndex++;
                    currentPlayerIndex %= numPlayers;
                    int faceCardTries = scene.GetComponent<CardComponent>(deck.cards[deck.numCards - 1]).numAttempts;
                    cout << "Attempts: " << scene.GetComponent<CardComponent>(deck.cards[deck.numCards - 1]).numAttempts << endl;
                    scene.GetComponent<PlayerComponent>(players[currentPlayerIndex]).attemptsLeft = faceCardTries > 0 ? faceCardTries : 1;
                }
            }

            else rendering.color = playerColors[player.id];
            TextUpdateSystem(scene);
        }

		// RenderingComponent
		{
			// Clear screen
			window.ClearBackground(BLACK);

			camera.BeginMode();
			{
                DrawSystem(scene);
			}
			camera.EndMode();
		}
		window.EndDrawing();
	}

	return 0;
}

void SetImageTexture(string filepath, Texture2D& texture) {
    Image sprite = LoadImage(filepath.c_str());
    ImageResize(&sprite, 100, 100);
    texture = LoadTextureFromImage(sprite);
    UnloadImage(sprite);
}