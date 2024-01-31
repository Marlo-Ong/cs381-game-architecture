#include "raylib-cpp.hpp"

int main() {
    raylib::Window window(1000,1000, "CS381 - Assignment 0");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    raylib::Font text;

    while (!window.ShouldClose()) {
        window.BeginDrawing();
        window.ClearBackground(RAYWHITE);
        raylib::Vector2 textSize = text.MeasureText("It doesn't matter what the text says, \nas long as it is obvious that it is there!", 20, 0);
        raylib::Vector2 center(window.GetWidth()/2 - textSize.GetX()/2, window.GetHeight()/2 - textSize.GetY()/2);
        text.DrawText("It doesn't matter what the text says, \nas long as it is obvious that it is there!", center, 20, 0, DARKPURPLE);
        window.EndDrawing();
    }

    return 0;
}
