#include "Board.h"
#include "SDL.h"
#include "PieceTypes.h"
#include "ChessApplication.h"
#include <cctype>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <iostream>

Board::Board()
    : _livePieces{}, _capturedPieces{}, _enPassantIndex(-1), 
      _isWhitesTurn(true), _castlingRights(0u), _playingAsWhite(false)
{
    //load the board with the fen string
    const std::string startingFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; 
      
    //set up board variables and allocate the pieces
    loadFENIntoBoard(startingFEN);

    //update the pieces internal legal moves
    Board::updatePseudoLegalMoves();
}

//give the pieces back :(
Board::~Board()
{
    for(Piece* piece : _livePieces)
        delete piece;
    
    for(Piece* piece : _capturedPieces)   
        delete piece;    
}

void Board::piecePickUpRoutine(const SDL_Event& mouseEvent)
{
    if(mouseEvent.button.button != SDL_BUTTON_LEFT || Piece::s_pieceOnMouse)
        return;

    Vector2i chessPos{};
    chessPos = ChessApp::screen2ChessPos({mouseEvent.button.x, mouseEvent.button.y});
    Sint32 index = chessPos.y * 8 + chessPos.x;
    Piece* p = _livePieces[index];

    if(p && p->_isWhite == _isWhitesTurn)
        Piece::s_pieceOnMouse = p;
}

void Board::piecePutDownRoutine(const SDL_Event& mouseEvent)
{
    if(mouseEvent.button.button != SDL_BUTTON_LEFT || !Piece::s_pieceOnMouse)
        return;

    Vector2i chessPos{};
    chessPos = ChessApp::screen2ChessPos({mouseEvent.button.x, mouseEvent.button.y});

    if(requestMove(chessPos))
        commitMove(chessPos);
    
    Piece::s_pieceOnMouse = nullptr;
}

bool Board::requestMove(Vector2i& potentialChessPos)
{
    int potentialIndex = potentialChessPos.y * 8 + potentialChessPos.x;
    auto end = Piece::s_pieceOnMouse->_pseudoLegalMoves.end();

    if(std::find(Piece::s_pieceOnMouse->_pseudoLegalMoves.begin(), end, potentialIndex) != end)
        return true;

    

    return false;
}

void Board::commitMove(Vector2i& chessPos)
{
    int newIndex = chessPos.y * 8 + chessPos.x;
    int oldIndex = Piece::s_pieceOnMouse->_chessPos.y * 8 + Piece::s_pieceOnMouse->_chessPos.x;
    Piece::s_pieceOnMouse->update(chessPos, ChessApp::getBoard());
    movePiecePointers(oldIndex, newIndex);//this will move the piece to the captured pool if there is one at newIndex
    toggleTurn();
    updatePseudoLegalMoves();
    Piece::s_pieceOnMouse = nullptr;
}

void Board::updateEnPassant(int file, int rank)
{
    if(ChessApp::inRange({file, rank}))
    {
        _enPassantIndex = rank * 8 + file;
    }
    else
    {
        _enPassantIndex = -1;
    }
}

int Board::getEnPassantIndex()
{
    return _enPassantIndex;
}

void Board::resetEnPassant()
{
    _enPassantIndex = -1;
}

//this function can be used to load a FEN with less than 6 fields
//(like just the pysical positions of the pieces). but if it detects
//more than 6 fields in the FEN string it will throw and exception
void Board::loadFENIntoBoard(const std::string& FEN)
{
    Board& b = ChessApp::getBoard();

    //map the chars in a fen string to a function pointer
    //that points to the factory function for each derived piece type  
    typedef void(Board::*fpFactory)(const Vector2i&, bool);
    std::unordered_map<char, fpFactory> fenMapping = 
    {
        {'p', &Board::makeNewPawnAt},  {'n', &Board::makeNewKnightAt},
        {'r', &Board::makeNewRookAt},  {'b', &Board::makeNewBishopAt},
        {'q', &Board::makeNewQueenAt}, {'k', &Board::makeNewKingAt}
    };

    int file = 0, rank = 7;
    auto it = FEN.begin();

    //this loop handles the first field (positional field) in the FEN string
    for( ; it != FEN.end(); ++it)
    {   
        if(*it == ' ')//if the next field was reached
        {
            ++it;
            break;
        }
        if(*it == '/')
        {
            --rank;
            file=0;
        }
        else if(isdigit(*it))
        {
            //ascii digit - 48 maps the char to the cooresponding decimal value
            file += *it - 48;
        }
        else//if c is an upper or lowercase letter
        {
            bool side = isupper(*it);
            char c = tolower(*it);
            (b.*fenMapping[c])({file, rank}, side);
            ++file;
        }
    }
    
    //if the FEN string only had a positional field
    if(it == FEN.end())
        return;

    //handle whos turn it is to move
    //if c is 'w' then its whites turn otherwise its blacks
    _isWhitesTurn = (*it == 'w');
    
    //if the turn field wasnt the last field then jump over the space after it
    if(++it != FEN.end())
        ++it;
    else return;

    //handle castling rights
    for( ; it != FEN.end(); ++it)
    {
        if(*it == ' ')
            break;

        switch(*it)
        {
        case 'K':
            _castlingRights |= WSHORT;
            break;
        case 'Q':
            _castlingRights |= WLONG;
            break;    
        case 'k':      
            _castlingRights |= BSHORT;
            break;     
        case 'q':      
            _castlingRights |= BLONG;
        }
    }

    //if the castle rights field wasnt the last field then jump over the space after it
    if(it != FEN.end())
        ++it;
    else return;

    //handle possible en passant target
    if(*it != '-')
    {
        char c = tolower(*it);
        int x = (c - 97);
        ++it;
        int y = *it - 48;
        --y;//rank and file start from zero so decrement is needed here
        _enPassantIndex = y * 8 + x;
    }

    //if the FEN string doesnt have the last two fields
    if(it == FEN.end())
        return;

    //set the halfmove clock
    //(not implemented yet)

    //set the fullmove clock
    //(not implemented yet)
}

void Board::updatePseudoLegalMoves()
{
    for(auto* p : _livePieces)
        if(p) p->calculatePseudoLegalMoves();    
}

bool Board::canCastle(castleRights cr)
{
    return _castlingRights & cr;
}

void Board::removeCastlingRights(castleRights cr)
{
    //This could be _castlingRights -= cr; but if it was and
    //the same castling rights get removed twice by accident
    //then that would mess up the _castlingRights bits.
    //so I will use bitwise operators instead

    _castlingRights &= ~cr;
}

void Board::completeCastle(castleRights cr)
{
    //do one more check to make sure we have castle rights
    if(!canCastle(cr))
        return;
    
    //now all we have to do is pull the rook over
    //since the king was moved when we set
    //that piece down with commitmove()
    
    //if we have castle rights there will be an unmoved rook in the corner
    int rookIndex = -1;
    switch(cr)
    {
    case WLONG:
        rookIndex = 0;
        removeCastlingRights(WLONG);
        removeCastlingRights(WSHORT);
        break;

    case WSHORT:
        rookIndex = 7;
        removeCastlingRights(WSHORT);
        removeCastlingRights(WLONG);
        break;

    case BLONG:
        rookIndex = 56;
        removeCastlingRights(BLONG);
        removeCastlingRights(BSHORT);
        break;

    case BSHORT:
        rookIndex = 63;
        removeCastlingRights(BSHORT);
        removeCastlingRights(BLONG);
    }

    Piece* rook = getPieceAt(rookIndex);
    Vector2i newPos{rookIndex % 8 + ((cr == WSHORT || cr == BSHORT) ? -2 : 3), rookIndex / 8};
    rook->update(newPos, ChessApp::getBoard());
    movePiecePointers(rookIndex, newPos.x + newPos.y * 8);
}

void Board::drawSquares()
{
    Uint32 squareSize = ChessApp::getSquareSize();
    SDL_Rect square = {0, 0, squareSize, squareSize};
    SDL_Renderer* currRenderer = ChessApp::getCurrentRenderer();

    for(int i = 0; i < 8; ++i, square.x += squareSize)
    {
        for(int j = 0; j < 8; ++j, square.y += squareSize)
        {
            if( !((i+j) & 1) )
            {
                SDL_SetRenderDrawColor(currRenderer, 229, 242, 208, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(currRenderer, 94, 138, 67, 255);
            }
            
            SDL_RenderFillRect(currRenderer, &square);
        }
        square.y = 0;
    }
}

void Board::drawPieces()
{
    Piece* pom = Piece::s_pieceOnMouse;
    for(Piece* piece : _livePieces)
    {
        //if the piece is there and it isnt the piece on the mouse
        //(the piece on the mouse draw is defered to the end of the loop)
        if(piece && piece != pom)
            piece->draw();
    }
    
    //draw here afterwards so that the piece on the mouse looks like its hovering over other pieces.
    //if we already determine that there is a piece on the mouse and which piece it is, then
    //there isnt a need to check again in every draw overload if this == pieceOnMouse.
    //so instead let built in Vtable figure out which concrete type is attached to the mouse
    if(pom)
        pom->drawPieceOnMouse();
}

void Board::drawIndicatorCircles()
{
    if(!Piece::s_pieceOnMouse)
        return;

    SDL_Renderer* renderer = ChessApp::getCurrentRenderer();
    SDL_Texture* circle = ChessApp::getCircleTexture();
    SDL_Texture* redCircle = ChessApp::getRedCircleTexture();
    SDL_Rect circleSource{0, 0, 0, 0};
    SDL_QueryTexture(circle, nullptr, nullptr, &circleSource.w, &circleSource.h);

    for(auto i : Piece::s_pieceOnMouse->_pseudoLegalMoves)
    {
        Vector2i circlePos = ChessApp::chess2ScreenPos({i % 8, i / 8});
        SDL_Rect circleDest
        {
            circlePos.x - circleSource.w * 0.5f, 
            circlePos.y - circleSource.w * 0.5f, 
            circleSource.w, 
            circleSource.h
        };

        //if there is an enemy piece draw red circle instead
        if(_livePieces[i] && _livePieces[i]->_isWhite != _isWhitesTurn)
        {
            SDL_RenderCopy(renderer, redCircle, &circleSource, &circleDest);
            continue;
        }

        SDL_RenderCopy(renderer, circle, &circleSource, &circleDest);
    }
}

void Board::capturePiece(int index)
{
    static int back;

    //make sure we havent run out of room
    if(back > 63)
    {
        std::cerr << "somehow the captured pieces buffer ran out of room. This shouldnt happen...\n";
        exit(2);//JUST FREAKING ABORT MISSION
    }

    //if there isnt a piece here just leave
    if(!_livePieces[index])
        return;
    
    _capturedPieces[back++] = _livePieces[index];
    _livePieces[index] = nullptr;
}

void Board::movePiecePointers(int sourceIndex, int destinationIndex)
{
    //if there is a piece at the destination move it to the array of captured pieces
    if(_livePieces[destinationIndex])
        capturePiece(destinationIndex);

    _livePieces[destinationIndex] = _livePieces[sourceIndex];
    _livePieces[sourceIndex] = nullptr;
}

Piece* const Board::getPieceAt(Vector2i& chessPos)
{
    return _livePieces[chessPos.y * 8 + chessPos.x];
}

Piece* const Board::getPieceAt(int index)
{
    return _livePieces[index];
}

void Board::toggleTurn()
{
    _isWhitesTurn = !_isWhitesTurn;
}

//factory methods for placing a piece at the specified location on the board
//these should only be called in the boards constructor ideally to follow RAII.
//But there is a call to one of the major pieces upon promotion
//all of the memory will be freed in the boards destructor
void Board::makeNewPawnAt(const Vector2i& pos, bool side)
{
    int index = pos.y * 8 + pos.x;

    //this shouldnt happen but if it does just move the piece to the captured pool
    if(_livePieces[index])
        capturePiece(index);

    _livePieces[index] = new Pawn(side, pos);
}

void Board::makeNewKnightAt(const Vector2i& pos, bool side)
{
    int index = pos.y * 8 + pos.x;

    //this shouldnt happen but if it does just move the piece to the captured pool
    if(_livePieces[index])
        capturePiece(index);

    _livePieces[index] = new Knight(side, pos);
}

void Board::makeNewRookAt(const Vector2i& pos, bool side)
{
    int index = pos.y * 8 + pos.x;

    //this shouldnt happen but if it does just move the piece to the captured pool
    if(_livePieces[index])
        capturePiece(index);

    _livePieces[index] = new Rook(side, pos);
}

void Board::makeNewBishopAt(const Vector2i& pos, bool side)
{
    int index = pos.y * 8 + pos.x;

    //this shouldnt happen but if it does just move the piece to the captured pool
    if(_livePieces[index])
        capturePiece(index);

    _livePieces[index] = new Bishop(side, pos);
}

void Board::makeNewQueenAt(const Vector2i& pos, bool side)
{
    int index = pos.y * 8 + pos.x;

    //this shouldnt happen but if it does just move the piece to the captured pool
    if(_livePieces[index])
        capturePiece(index);

    _livePieces[index] = new Queen(side, pos);
}

void Board::makeNewKingAt(const Vector2i& pos, bool side)
{
    int index = pos.y * 8 + pos.x;

    //this shouldnt happen but if it does just move the piece to the captured pool
    if(_livePieces[index])
        capturePiece(index);

    _livePieces[index] = new King(side, pos);
}