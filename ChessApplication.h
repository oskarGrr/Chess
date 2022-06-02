#pragma once
#include "Board.h"
#include "Window.h"
#include "SDL.h"

struct Vector2i 
{
    int x = 0, y = 0;
    inline bool operator==(const Vector2i& rhs) 
    {
        return x == rhs.x && y == rhs.y;
    }
};

#define MAIN_WINDOW_WIDTH 896
#define MAIN_WINDOW_HEIGHT 896

//singleton chessApp contains the board which owns the pieces
class ChessApp
{
private:
    ChessApp();
    static ChessApp s_theApplication;   

public:
    ChessApp(const ChessApp&)=delete;
    ChessApp(ChessApp&&)=delete;
    ~ChessApp();

    static ChessApp& getApp();
    static Board& getBoard();
    static Uint32 getSquareSize();
    static SDL_Renderer* getCurrentRenderer();

    static Vector2i chess2ScreenPos(Vector2i);
    static Vector2i screen2ChessPos(Vector2i);
    static bool inRange(Vector2i chessPos);
    static bool isMouseOver(SDL_Rect&);
    static void initCircleTexture(
        int radius, 
        Uint8 r, 
        Uint8 g, 
        Uint8 b, 
        Uint8 a, 
        SDL_Texture** toInit
    );
    static SDL_Texture* getCircleTexture();
    static SDL_Texture* getRedCircleTexture();

    void run();

    struct promotionQueue{int whichSquare; bool wasPieceWhite;};
    static void setPromotionQueue(promotionQueue);
    void promotionRoutine(promotionQueue);

private:
    Window _wnd;//my simple wrapper class for SDL window
    Uint32 _squareSize;//square size in pixels
    Board  _board;//the singleton board composed here as part of the app
    promotionQueue _promotionQueue;//-1 unless a promotion happened then set to the index of the promotion square
    SDL_Texture* _circleTexture;
    SDL_Texture* _redCircleTexture;
};