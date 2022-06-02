#pragma once
#include "SDL.h"
#include <string>
#include <vector>

struct Vector2i;
class Piece;

//singleton Board only instantiated as a member of the chessApplication.
class Board
{
    friend class ChessApp;//only chessApp should call the private ctor
private:
    Board();

public:
    ~Board();//free the pieces
    Board(const Board&)=delete;
    Board(Board&&)=delete;

    void piecePickUpRoutine(const SDL_Event&);
    void piecePutDownRoutine(const SDL_Event& e);
    bool requestMove(Vector2i& potentialChessPos);
    void commitMove(Vector2i& chessPos);
    void movePiecePointers(int oldIndex, int newIndex);//move the pieces on the board
    Piece* const getPieceAt(Vector2i& chessPos);//does NOT check if the input is inrange
    Piece* const getPieceAt(int index);//overload if you want to use 0-63. doesnt check if index in in range
    void toggleTurn();//change whos turn it is to move
    void capturePiece(int index);//moves a pointer to the captured array
    void loadFENIntoBoard(const std::string&);
    void updatePseudoLegalMoves();//updates the pieces internal pseudo legal move vectors

    void updateEnPassant(int file, int rank);
    int  getEnPassantIndex();
    void resetEnPassant();

    void drawSquares();
    void drawPieces();
    void drawIndicatorCircles();

    //factory methods for placing a piece at the specified location on the board
    //these should only be called in the boards constructor ideally to follow RAII.
    //But there is a call to one of the major pieces upon promotion
    //all of the memory will be freed in the boards destructor
    void makeNewPawnAt  (const Vector2i& pos, bool side);
    void makeNewKnightAt(const Vector2i& pos, bool side);
    void makeNewRookAt  (const Vector2i& pos, bool side);
    void makeNewBishopAt(const Vector2i& pos, bool side);
    void makeNewQueenAt (const Vector2i& pos, bool side);
    void makeNewKingAt  (const Vector2i& pos, bool side);  

    //bit masks to indicate what castling rights are available 
    enum castleRights : Uint32
    {
        WSHORT = 0b1, WLONG = 0b10,
        BSHORT = 0b100, BLONG = 0b1000
    };

    bool canCastle(castleRights);//supply with one of the enums above and will respond with true or false
    void removeCastlingRights(castleRights);
    void completeCastle(castleRights);

private:
    Piece* _livePieces[64];//the pointers will be null if there isnt a piece there
    Piece* _capturedPieces[64];//where the pointers go to be deleted in the destructor
    int    _enPassantIndex;//index of the square with en passant target (-1 = no target)
    bool   _isWhitesTurn;//used to remember whos turn is it
    Uint32 _castlingRights;//bitwise AND with the enumerations above
    bool   _playingAsWhite;//is the user playing as white or black
};