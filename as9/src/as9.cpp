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
    float posX;
    float posY;
    Vector2 origin; // anchor to middle of rect (assume square with w=100, h=100)
    float rotation = 0;
};

struct Rendering {
    Color color;
    Texture2D texture;
};

struct InputComponent {
    raylib::BufferedInput inputs;
};

struct CardComponent {
    bool isPlayerTurn;

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
    int numCards;
};

struct Player1TagComponent {};
struct Player2TagComponent {};
struct Player3TagComponent {};
struct Player4TagComponent {};

struct DeckTagComponent {};

struct DeckComponent {
    cs381::Entity cards[52];
    int numCards;
};

struct TextComponent {
    string text;
    int fontSize;
};

void DrawSystem(cs381::Scene<>& scene) {
    for(auto [rendering, transform]: cs381::SceneView<Rendering, TransformComponent>{scene}) {
        DrawTexturePro(rendering.texture,
                        transform.source,
                        transform.dest,
                        transform.origin,
                        transform.rotation,
                        rendering.color);
    }

    for(auto [text, rendering, transform]: cs381::SceneView<TextComponent, Rendering, TransformComponent>{scene}) {
        DrawText(text.text.c_str(), int(transform.posX), int(transform.posY), text.fontSize, rendering.color);
    }

    DrawFPS(10, 10);
}

void CardRandomAssignmentSystem(cs381::Scene<>& scene) {
    static string suits[4] = {"hearts", "spades", "diamonds", "clubs"};
    static string values[13] = {"ace", "2", "3", "4", "5", "6", "7", "8", "9", "10", "jack", "queen", "king"};

    for(auto [card, _, rendering]: cs381::SceneView<CardComponent, DeckTagComponent, Rendering>{scene}) {

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
    for (auto [player, text] : cs381::SceneView<PlayerComponent, TextComponent>{scene}) {
        text.text = to_string(player.numCards);
    }
}

bool CheckSlap(DeckComponent, PlayerComponent, cs381::Scene<>&);

bool MouseClickSystem(cs381::Scene<>& scene, PlayerComponent currentPlayer) {
    Vector2 mousePos = GetMousePosition();
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        cout << "Clicked at " << mousePos.x << ", " << mousePos.y << endl;

        // Clicked deck
        for (auto [deck, _, transform] : cs381::SceneView<DeckComponent, InputComponent, TransformComponent>{scene}) {
            if (CheckCollisionPointRec(mousePos, transform.dest)) {
                cout << "Deck was clicked" << endl;
                CheckSlap(deck, currentPlayer, scene);
            }
        }

        for (auto [player, _, transform] : cs381::SceneView<PlayerComponent, InputComponent, TransformComponent>{scene}) {
            if (currentPlayer.id == player.id && CheckCollisionPointRec(mousePos, transform.dest)) {
                cout << "Player clicked own hand" << endl;
                // Place player card onto deck
                for (auto [deck] : cs381::SceneView<DeckComponent>{scene}) {
                    if (player.numCards > 0) {
                        deck.cards[deck.numCards] = player.cards[player.numCards];
                        deck.numCards++;
                        player.numCards--;
                        return true;
                    }
                }
            }
        }

        return false;
    }
    

    // for (auto [input, transform] : cs381::SceneView<InputComponent, TransformComponent>{scene}) {
    //     input.inputs["click"] =
    //     raylib::Action::mouse_button(MOUSE_BUTTON_LEFT)
    //         .SetPressedCallback([&mousePos = mousePos, &transform = transform]{
    //             if (CheckCollisionPointRec(mousePos, transform.dest)) {
    //                 cout << "YEP: " << transform.dest.x << ", " << transform.dest.y << endl;
    //             }
    //     }).move();
    // }
}

bool CheckSlap(DeckComponent deck, PlayerComponent player, cs381::Scene<>& scene) {
    if (deck.numCards < 3) return false;
    CardComponent topCard = scene.GetComponent<CardComponent>(deck.cards[deck.numCards]);
    CardComponent secondTopCard = scene.GetComponent<CardComponent>(deck.cards[deck.numCards - 1]);
    CardComponent thirdTopCard = scene.GetComponent<CardComponent>(deck.cards[deck.numCards - 2]);

    // Check double
    if (topCard == secondTopCard || topCard == thirdTopCard) {
        return true;
    }
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
    int numCards = 52;
    int numPlayers = 4;

    // Create camera
    raylib::Camera2D camera(
        raylib::Vector2(0, 0), // position
        raylib::Vector2(0, 0)  // target
    );

    // Create deck
    auto deck = scene.CreateEntity();
    Texture2D deckTexture;
    SetImageTexture("../icons/card-draw.png", deckTexture);
    Rectangle deckSource = { 0, 0, (float)deckTexture.width, (float)deckTexture.height };
    Rectangle deckRect = { screenWidth / 2 - 50, screenHeight / 2 - 50, deckSource.width, deckSource.height };
    scene.AddComponent<Rendering>(deck) = {GRAY, deckTexture};
    scene.AddComponent<TransformComponent>(deck) = {deckSource, deckRect};
    DeckComponent& deckData = scene.AddComponent<DeckComponent>(deck);
    scene.AddComponent<InputComponent>(deck);

    // Create cards
    for (int i = 0; i < numCards; i++)
    {
        auto card = scene.CreateEntity();
        deckData.cards[i] = card;
        scene.AddComponent<Rendering>(card) = {WHITE};
        scene.AddComponent<CardComponent>(card);
        scene.AddComponent<DeckTagComponent>(card);

        Rectangle source = {0.0, 0.0, 100.0, 100.0};
        Rectangle dest = {float((15 * i) - (0.5 * screenWidth)), 0.0, 100.0, 100.0};
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
        scene.AddComponent<Rendering>(player) = {playerColors[i], deckTexture};
        scene.AddComponent<PlayerComponent>(player) = {i};
        scene.AddComponent<InputComponent>(player);

        Rectangle source = {0.0, 0.0, 100.0, 100.0};
        float destWidth = screenWidth * anchorX[i] - 50;
        float destHeight = screenHeight * anchorY[i] - 50;
        Rectangle dest = {destWidth, destHeight, 100.0, 100.0};
        cout << "Create player at: " << destWidth << ", " << destHeight << endl;
        scene.AddComponent<TransformComponent>(player) = {source, dest, destWidth, destHeight};
        scene.AddComponent<TextComponent>(player) = {"0", 15};
    }

    // Assign cards to players
    for (auto [player] : cs381::SceneView<PlayerComponent>{scene}) {
        for (int i = 0; i < floor(numCards / numPlayers); i++) {
            player.cards[i] = deckData.cards[deckData.numCards];
            deckData.numCards--;
        }
        player.numCards = floor(numCards / numPlayers);
        deckData.numCards = 0;
    }

    // Play audio
    InitAudioDevice();
    raylib::Sound mus_BG("../audio/bg.mp3");
    mus_BG.Play();

	// Main loop
	bool keepRunning = true;
    int currentPlayerIndex = -1;
    float delay = 3;
    float timer = 0;
	while(!window.ShouldClose() && keepRunning) {
        timer += window.GetFrameTime();
        if (timer > delay) {
            timer = 0;
            currentPlayerIndex++;
            currentPlayerIndex %= numPlayers;
            cout << "Current player: Player " << currentPlayerIndex << endl;
        }

        for (auto [player] : cs381::SceneView<PlayerComponent>{scene}) {
            if (player.id == currentPlayerIndex) {
                if (MouseClickSystem(scene, player)) {
                    scene.GetComponent<TransformComponent>(deckData.cards[deckData.numCards-1]).dest = deckRect; 
                };
                TextUpdateSystem(scene);
            }
        }

		// Rendering
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