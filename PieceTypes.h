#pragma once
#include "SDL.h"
#include "ChessApplication.h"
#include <vector>

class Board;

//this class isnt responsible for ownership
//of the Pieces. the ChessApp::_Board is, because the board "holds" the pieces.
//This abstract Piece class is meant to serve as a base class for
//the concrete pieces to derive from, and isnt meant to be instantiated by itself
class Piece
{
public:
    Piece()=delete;
    Piece(const Piece&)=delete;
    Piece(Piece&&)=delete;
    Piece(bool side, Vector2i chessPos);
    virtual ~Piece();

    //indices into the s_textures array
    enum textureIndices : Uint32
    {
        WPAWN = 0, WKNIGHT, WROOK, WBISHOP, WQUEEN, WKING,
        BPAWN = 6, BKNIGHT, BROOK, BBISHOP, BQUEEN, BKING
    };

    inline static SDL_Texture* s_textures[12]{};//gets filled when pieces are made 
    inline static const float s_scale = 0.32f;//how much to scale down the textures

public:
    inline static Piece* s_pieceOnMouse = nullptr;//remember which piece is on the mouse

    const bool _isWhite;//black or white piece
    Vector2i _chessPos;//the file and rank (x,y) of where the piece is (0-7)
    Vector2i _screenPos;//location (on the screen) of the middle of the square that the piece is on
    std::vector<int> _pseudoLegalMoves;//the cached legal moves for a piece
    SDL_Texture* _texture;//every piece will have an SDL texture that will point to the correct piece texture
    SDL_Rect _sourceRect;
    SDL_Rect _destRect;

    virtual void draw()=0;
    virtual void update(Vector2i newChessPos, Board&)=0;
    virtual void calculatePseudoLegalMoves()=0;

    //figure out what concrete type is on the mouse
    //(and thus what texture to render) through the built in Vtables.
    //this is done so I dont have to check in every single draw() call above
    //if the pieceOnMouse == this and then draw that piece on the mouse instead.
    //(the above conditional is already checked for every piece in the wrapper function
    //Board::drawPieces(Board&) so it doesnt make sense to check again in in every draw overload)
    virtual void drawPieceOnMouse()=0;

protected:
    void updateInternalPosition(Vector2i);
    void initRects();//just called in the derived ctors to set up the texture sizes

private:
    static void initTextures();//called once in the base ctor
    static void destoryTextures();//called once in the dtor
};

class Pawn : public Piece
{
public:
    Pawn(bool side, Vector2i chessPos);
    Pawn()=delete;
    Pawn(const Pawn&)=delete;
    Pawn(Pawn&&)=delete;

private:
    void draw() override;
    void update(Vector2i newChessPos, Board&) override;
    void calculatePseudoLegalMoves() override;
    void drawPieceOnMouse() override;
};

class Knight : public Piece
{
public:
    Knight(bool side, Vector2i chessPos);
    Knight()=delete;
    Knight(const Knight&)=delete;
    Knight(Knight&&)=delete;

private:
    void draw() override;
    void update(Vector2i newChessPos, Board&) override;
    void calculatePseudoLegalMoves() override;
    void drawPieceOnMouse() override;
};

class Rook : public Piece
{
public:
    Rook(bool side, Vector2i chessPos);
    Rook()=delete;
    Rook(const Rook&)=delete;
    Rook(Rook&&)=delete;

private:
    void draw() override;
    void update(Vector2i newChessPos, Board&) override;
    void calculatePseudoLegalMoves() override;
    void drawPieceOnMouse() override;
};

class Bishop : public Piece
{
public:
    Bishop(bool side, Vector2i chessPos);
    Bishop()=delete;
    Bishop(const Bishop&)=delete;
    Bishop(Bishop&&)=delete;   

private:
    void draw() override;
    void update(Vector2i newChessPos, Board&) override;
    void calculatePseudoLegalMoves() override;
    void drawPieceOnMouse() override;
};

class Queen : public Piece
{
public:
    Queen(bool side, Vector2i chessPos);
    Queen()=delete;
    Queen(const Queen&)=delete;
    Queen(Queen&&)=delete;

private:
    void draw() override;
    void update(Vector2i newChessPos, Board&) override;
    void calculatePseudoLegalMoves() override;
    void drawPieceOnMouse() override;
};

class King : public Piece
{
public:
    King(bool side, Vector2i chessPos);
    King()=delete;
    King(const King&)=delete;
    King(King&&)=delete;

private:
    void draw() override;
    void update(Vector2i newChessPos, Board&) override;
    void calculatePseudoLegalMoves() override;
    void drawPieceOnMouse() override;
};