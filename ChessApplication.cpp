#include "ChessApplication.h"
#include "PieceTypes.h"
#include <memory>
#include <iostream>
#include "SDL.h"

ChessApp ChessApp::s_theApplication{};

ChessApp::ChessApp()
    : _wnd(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HEIGHT, "Chess", SDL_INIT_VIDEO, 0u),
      _board{}, _squareSize(MAIN_WINDOW_WIDTH / 8), _promotionQueue({-1, false}),
      _circleTexture(nullptr), _redCircleTexture(nullptr)
{
    initCircleTexture(_squareSize / 6, 0x6A, 0x6A, 0x6A, 0x7F, &_circleTexture);
    initCircleTexture(_squareSize / 6, 0xDE, 0x31, 0x63, 0x7F, &_redCircleTexture);
}

ChessApp::~ChessApp()
{
    SDL_DestroyTexture(_circleTexture);
    SDL_DestroyTexture(_redCircleTexture);
}

void ChessApp::run()
{
    bool running = true;

    while(running)
    {
        SDL_RenderClear(_wnd._renderer);
        _board.drawSquares();
        _board.drawPieces();
        _board.drawIndicatorCircles();
        SDL_RenderPresent(_wnd._renderer);

        if(_promotionQueue.whichSquare > -1)
            promotionRoutine(_promotionQueue);
        
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
            {
                running = false;
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                _board.piecePickUpRoutine(e);
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                _board.piecePutDownRoutine(e);
                break;
            }
            }
        }   
        SDL_Delay(10);
    }
}

void ChessApp::initCircleTexture(
    int radius, 
    Uint8 RR, 
    Uint8 GG, 
    Uint8 BB, 
    Uint8 AA, 
    SDL_Texture** toInit)
{
    //this maintains the order of (where sdl/opengl looks for) the R,G,B,A bytes.
    //for rgba sdl should look at the same location for a given channel regarless of endianness.
    //I doubt this will ever be compiled on a big endian machine but I will leave it here anyway
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rMask = 0xff000000;
    Uint32 gMask = 0x00ff0000;
    Uint32 bMask = 0x0000ff00;
    Uint32 aMask = 0x000000ff;
    Uint32 circleColor = (RR << 24) + (GG << 16) + (BB << 8) + AA;
#else
    Uint32 rMask = 0x000000ff;
    Uint32 gMask = 0x0000ff00;
    Uint32 bMask = 0x00ff0000;
    Uint32 aMask = 0xff000000;
    Uint32 circleColor = (AA << 24) + (BB << 16) + (GG << 8) + RR;
#endif
    int diameter = radius * 2;
    SDL_Rect boundingBox{-radius, -radius, diameter, diameter};
    Uint32 pixelCount = diameter * diameter;//number of pixels in the bounding box
    auto pixels = std::make_unique<Uint32[]>(pixelCount);

    //loop over all the pixels in the bounding box and see which ones
    //are inside the circle by comparing the distance of the pixel
    //from the center of the box(squared) to the radius(squared)
    int radiusSquared = radius * radius;
    int xOffset = -radius, yOffset = -radius;
    for(int x = 0; x < diameter; ++x)
    {
        for(int y = 0; y < diameter; ++y)
        {
            //if the distance of the pixel to the center(squared)
            //is less than the radius(squared) we are inside the circle
            if((x-radius)*(x-radius) + (y-radius)*(y-radius) <= radiusSquared)
            {
                pixels[x + y * diameter] = circleColor;
            }
        }
    }

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
         pixels.get(), 
         diameter, 
         diameter, 
         32, 
         diameter * sizeof(Uint32), 
         rMask, gMask, bMask, aMask
    );
   
    if(!surface)
    {
        std::cerr << SDL_GetError();
        exit(1);
    }

    *toInit = SDL_CreateTextureFromSurface(
        s_theApplication.getCurrentRenderer(), 
        surface
    );

    SDL_FreeSurface(surface);
}

void ChessApp::promotionRoutine(promotionQueue q)
{
    Vector2i promotionSquareChessPos = {q.whichSquare % 8, q.whichSquare / 8};
    Vector2i promotionSquareScreenPos = chess2ScreenPos(promotionSquareChessPos);
    SDL_Renderer* renderer = ChessApp::getCurrentRenderer();
    
    SDL_Rect popupRect =
    {
        promotionSquareScreenPos.x - _squareSize * 0.5f,
        promotionSquareScreenPos.y - _squareSize * 0.5f,
        _squareSize,
        _squareSize * 4
    };

    bool opositeSidePromotion = q.wasPieceWhite != _board._playingAsWhite;
    if(opositeSidePromotion)
    {
        //move up three and a half squares
        popupRect.y = promotionSquareScreenPos.y - _squareSize * 3.5;
    }
    else
    {
        popupRect.y = promotionSquareScreenPos.y - _squareSize * 0.5f;
    }

    SDL_Texture* textures[4] =
    {
        Piece::s_textures[q.wasPieceWhite ? Piece::WQUEEN  : Piece::BQUEEN],
        Piece::s_textures[q.wasPieceWhite ? Piece::WROOK   : Piece::BROOK],
        Piece::s_textures[q.wasPieceWhite ? Piece::WKNIGHT : Piece::BKNIGHT],
        Piece::s_textures[q.wasPieceWhite ? Piece::WBISHOP : Piece::BBISHOP]
    };
    SDL_Rect sources[4]{};//how big are the textures before any scaling
    SDL_Rect destinations[4]{};//where will the textures be drawn and how big after scaling

    //fill in the source and destination rectancles for every texture
    for(int i = 0, yOffset = 0; i < 4; ++i, yOffset += _squareSize)
    {
        SDL_QueryTexture(textures[i], nullptr, nullptr, &sources[i].w, &sources[i].h);
        destinations[i] = 
        {
            promotionSquareScreenPos.x - static_cast<int>(sources[i].w * Piece::s_scale * 0.5f),
            promotionSquareScreenPos.y - static_cast<int>(sources[i].h * Piece::s_scale * 0.5f),
            static_cast<int>(sources[i].w * Piece::s_scale),
            static_cast<int>(sources[i].h * Piece::s_scale)
        };

        //slide the pieces down/up
        if(q.wasPieceWhite != _board._playingAsWhite)//up
        {
            destinations[i].y -= yOffset;
        }
        else//down
        {
            destinations[i].y += yOffset;
        }
    }

    bool hasSelectionBeenMade = false;
    while(!hasSelectionBeenMade)
    {
        SDL_RenderClear(renderer);
        _board.drawSquares();
        _board.drawPieces();

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &popupRect);

        for(int i = 0; i < 4; ++i)
            SDL_RenderCopy(renderer, textures[i], sources + i, destinations + i);

        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
                exit(0);
            
            if(e.type == SDL_MOUSEBUTTONDOWN)
            {
                if(ChessApp::isMouseOver(popupRect))
                {   
                    int y = 0;
                    SDL_GetMouseState(nullptr, &y);

                    //to avoid a memory leak by overiting the Piece* at promotionSquareChessPos
                    //(the pawn that promoted is still there) capture the pawn there before placing a new piece down   
                    _board.capturePiece(q.whichSquare);

                    //the promotion popup will look like this cascading down/up from the promotion square
                    // down 0  |Q|      up 3 |B|  
                    //      1  |R|         2 |N|
                    //      2  |N|         1 |R|
                    //      3  |B|         0 |Q|
                    int whichPiece = opositeSidePromotion ? 7 - y / _squareSize : y / _squareSize;
                    switch(whichPiece)
                    {
                    case 0:
                        _board.makeNewQueenAt(promotionSquareChessPos, q.wasPieceWhite);
                        hasSelectionBeenMade = true;
                        break;
                    case 1:
                        _board.makeNewRookAt(promotionSquareChessPos, q.wasPieceWhite);
                        hasSelectionBeenMade = true;
                        break;
                    case 2:
                        _board.makeNewKnightAt(promotionSquareChessPos, q.wasPieceWhite);
                        hasSelectionBeenMade = true;
                        break;
                    case 3:
                        _board.makeNewBishopAt(promotionSquareChessPos, q.wasPieceWhite);
                        hasSelectionBeenMade = true;
                        break;
                    }
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    //reset the promotion queue
    _promotionQueue = {-1, false};
}

void ChessApp::setPromotionQueue(promotionQueue q)
{
    s_theApplication._promotionQueue = q;
}

SDL_Texture* ChessApp::getCircleTexture()
{
    return s_theApplication._circleTexture;
}

SDL_Texture* ChessApp::getRedCircleTexture()
{
    return s_theApplication._redCircleTexture;
}

ChessApp& ChessApp::getApp()
{
    return s_theApplication;
}

Board& ChessApp::getBoard()
{
    return s_theApplication._board;
}

Uint32 ChessApp::getSquareSize()
{
    return s_theApplication._squareSize;
}

SDL_Renderer* ChessApp::getCurrentRenderer()
{
    return s_theApplication._wnd._renderer;
}

bool ChessApp::isMouseOver(SDL_Rect& r)
{
    int x = 0, y = 0;
    SDL_GetMouseState(&x, &y);
    return(x >= r.x && x <= r.x+r.w && y >= r.y && y <= r.h + r.y);
}

bool ChessApp::inRange(Vector2i chessPos)
{
    return (chessPos.x <= 7 && chessPos.x >= 0 && chessPos.y <= 7 && chessPos.y >= 0);
}

//take a chess position and flip it to the correct screen position
//based on which perspective the client is viewing the board from
Vector2i ChessApp::chess2ScreenPos(Vector2i pos)
{
    Vector2i ret = {pos.x, pos.y};

    if(s_theApplication._board._playingAsWhite)
    {
        ret.y = 7 - ret.y;
    }
    else
    {
        ret.x = 7 - ret.x;
    }

    //scale up to board size
    ret.x *= s_theApplication._squareSize;
    ret.y *= s_theApplication._squareSize;

    //move from top left to middle of square
    ret.y += s_theApplication._squareSize * 0.5f;
    ret.x += s_theApplication._squareSize * 0.5f;

    return ret;
}

Vector2i ChessApp::screen2ChessPos(Vector2i pos)
{
    Vector2i ret = 
    {
        pos.x / s_theApplication._squareSize, 
        pos.y / s_theApplication._squareSize
    };

    if(s_theApplication._board._playingAsWhite)
    {
        ret.y = 7 - ret.y;
    }
    else
    {
        ret.x = 7 - ret.x;
    }

    return ret;
}
