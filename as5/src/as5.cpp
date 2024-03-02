#include "raylib-cpp.hpp"
#define GUI_VOLUMECONTROL_IMPLEMENTATION
#include "VolumeControl.h"
#include <iostream>
#include "delegate.hpp"

cs381::Delegate<void()> PingButton;

// the type of the main() function: void(*)(int, char**)
int main() {
    PingButton += []() {
        std::cout << "Ping!" << std::endl;
    };
    raylib::Window window(600,400, "CS381 - Assignment 5");

    raylib::Sound ping("../audio/flap.wav");
    PingButton += [&ping]() {
        ping.Play();
    };
    raylib::Music music("../audio/air-raid.mp3");
    auto guiState = InitGuiVolumeControl();

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        {
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
