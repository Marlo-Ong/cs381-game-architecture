#include "raylib-cpp.hpp"
#include "raygui.cpp"
#include "delegate.hpp"
#include <iostream>
#include <BufferedInput.hpp>

cs381::Delegate<void()> PingButton;

#define GUI_VOLUMECONTROL_IMPLEMENTATION
#include "VolumeControl.h"

// the type of the main() function: void(*)(int, char**)
int main() {
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
            {raylib::Button(KEY_LEFT_CONTROL), raylib::Button(KEY_V)},
            true); // combo = true; buttons must be pressed simultaneously

    raylib::Sound ping("../audio/flap.wav");
    PingButton += [&ping]() {
        ping.Play();
    };
    raylib::Music music("../audio/air-raid.mp3");
    auto guiState = InitGuiVolumeControl();
    
    PingButton();

    while (!window.ShouldClose()) {
        inputs.PollEvents();
        
        window.BeginDrawing();
        {
            window.ClearBackground((raylib::Color)RAYWHITE);
            float volume = 0.0f;
            GuiVolumeControl(&guiState);
            ping.Play();
            music.Update();
            music.SetVolume(volume);
        }
        window.EndDrawing();
    }

    return 0;
}
