#include <raylib-cpp.hpp>
#include <BufferedInput.hpp>
#include "../addons/raygui.cpp"
#include <iostream>
#include "delegate.hpp"

cs381::Delegate<void()> PingButton;

#define GUI_VOLUMECONTROL_IMPLEMENTATION
#include "VolumeControl.h"

// the type of the main() function: void(*)(int, char**)
int main() {
    InitAudioDevice();

    PingButton += []() {
        std::cout << "Ping!" << std::endl;
    };

    raylib::Window window(300,350, "CS381 - Assignment 5");

    raylib::BufferedInput inputs;      // initialize Action Manager
    inputs["ping"] =                   // naming the action
        raylib::Action::key(KEY_SPACE) // several delegates to 1 input key
        .SetPressedCallback([]{        // call following delegate when pressed
            PingButton();
        }).move(); // use move since delegates cannot are move-only (not copyable)
    inputs["paste"] =
        raylib::Action::button_set(
            {raylib::Button::key(KEY_LEFT_CONTROL), raylib::Button::key(KEY_V)},
            true); // combo = true; buttons must be pressed simultaneously

    // EXTRA CREDIT: Custom audio off the Internet
    raylib::Sound vineBoom("../audio/Vine-boom-sound-effect.mp3");
    raylib::Music music("../audio/allstar-instrumental.mp3");
    raylib::Music dialogue("../audio/allstar-vocals.mp3");
    music.Play();
    dialogue.Play();

    PingButton += [&vineBoom]() {
        vineBoom.Play();
    };
    auto guiState = InitGuiVolumeControl();
    bool isDarkMode = false;
    int sliderSelect = 0;

    while (!window.ShouldClose()) {
        inputs.PollEvents();

        // EXTRA CREDIT: Add dark theme option for GUI
        if (IsKeyPressed(KEY_APOSTROPHE)) isDarkMode = !isDarkMode;

        // EXTRA CREDIT: Tab between sliders and change volume with arrow keys
        if (IsKeyPressed(KEY_TAB)) {
            sliderSelect += 1;
            sliderSelect %= 3;
            std::cout << sliderSelect << std::endl;
        }
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
            switch (sliderSelect) {
                case 0:
                    if (guiState.SFXSliderValue > 0) guiState.SFXSliderValue -= 0.1;
                    break;
                case 1:
                    if (guiState.MusicSliderValue > 0) guiState.MusicSliderValue -= .1;
                    break;
                case 2:
                    if (guiState.DialogueSliderValue > 0) guiState.DialogueSliderValue -= .1;
                    break;
            }
        }

        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
            switch (sliderSelect) {
                case 0:
                    if (guiState.SFXSliderValue < 100) guiState.SFXSliderValue += .1;
                    break;
                case 1:
                    if (guiState.MusicSliderValue < 100) guiState.MusicSliderValue += .1;
                    break;
                case 2:
                    if (guiState.DialogueSliderValue < 100) guiState.DialogueSliderValue += .1;
                    break;
            }
        }
        
        window.BeginDrawing();
        {
            if (isDarkMode) window.ClearBackground(BLACK);
            else window.ClearBackground(RAYWHITE);
            GuiVolumeControl(&guiState);

            music.Update();
            dialogue.Update();

            music.SetVolume(guiState.MusicSliderValue / 100);
            dialogue.SetVolume(guiState.DialogueSliderValue / 100);
            vineBoom.SetVolume(guiState.SFXSliderValue / 100);
        }
        window.EndDrawing();
    }

    return 0;
}