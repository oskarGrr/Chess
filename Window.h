#pragma once
#include "SDL.h"

class ChessApp;

class Window
{
private:
    friend class ChessApp;//only chess app should make windows
    Window(int width, int height, const char* title, Uint32 SDL_subsystems, Uint32 SDL_windowFlags);
    ~Window();
    Window(const Window&)=delete;
    Window(Window&&)=delete;

    SDL_Renderer* _renderer;
    SDL_Window* _window;
    const int _width, _height;

    void run();
};