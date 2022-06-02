#include "Window.h"
#include "SDL.h"

Window::Window(int width, int height, const char* title, Uint32 SDL_subsystems, Uint32 SDL_windowFlags)
    : _renderer(nullptr), _window(nullptr), _width(width), _height(height)
{
    SDL_Init(SDL_subsystems);
    SDL_CreateWindowAndRenderer(width, height, SDL_windowFlags, &_window, &_renderer);
    SDL_SetWindowTitle(_window, title);
}

Window::~Window()
{
    SDL_DestroyWindow(_window);
    SDL_DestroyRenderer(_renderer);
    SDL_Quit();
}