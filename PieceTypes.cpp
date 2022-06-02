#include "PieceTypes.h"
#include "ChessApplication.h"
#include "Board.h"
#include "SDL_image.h"
#include "SDL.h"
#include <iostream>

//constructors begin
Piece::Piece(bool side, Vector2i chessPos)
    : _isWhite(side), _chessPos(chessPos), _screenPos(ChessApp::chess2ScreenPos(chessPos)),
      _pseudoLegalMoves{}, _texture(nullptr), _sourceRect{}, _destRect{} 
{
    static bool texturesHaveBeenInitialized = false;
    if(!texturesHaveBeenInitialized)
    {
        initTextures();
        texturesHaveBeenInitialized = true;
    }
}

//just inits Piece::_destRect and Piece::_sourceRect for the ctors below
void Piece::initRects()
{
    int textureWidth = 0, textureHeight = 0;
    SDL_QueryTexture(_texture, nullptr, nullptr, &textureWidth, &textureHeight);
    _sourceRect = {0, 0, textureWidth, textureHeight};
    _destRect =
    {
        static_cast<int>(_screenPos.x - _sourceRect.w * s_scale * 0.5f),
        static_cast<int>(_screenPos.y - _sourceRect.h * s_scale * 0.5f), 
        static_cast<int>(_sourceRect.w * s_scale),
        static_cast<int>(_sourceRect.h * s_scale)
    };
}

Pawn::Pawn(bool side, Vector2i chessPos) : Piece(side, chessPos) 
{
    _texture = s_textures[side ? WPAWN : BPAWN];
    initRects();
}

Knight::Knight(bool side, Vector2i chessPos) : Piece(side, chessPos)
{
    _texture = s_textures[side ? WKNIGHT : BKNIGHT];
    initRects();
}

Rook::Rook(bool side, Vector2i chessPos) : Piece(side, chessPos) 
{
    _texture = s_textures[side ? WROOK : BROOK];
    initRects();
}

Bishop::Bishop (bool side, Vector2i chessPos) : Piece(side, chessPos) 
{
    _texture = s_textures[side ? WBISHOP : BBISHOP];
    initRects();
}

Queen::Queen(bool side, Vector2i chessPos) : Piece(side, chessPos) 
{
    _texture = s_textures[side ? WQUEEN: BQUEEN];
    initRects();
}

King::King(bool side, Vector2i chessPos) : Piece(side, chessPos) 
{
    _texture = s_textures[side ? WKING : BKING];
    initRects();
}
//constructors end

void Piece::destoryTextures()
{
    for(SDL_Texture* t : s_textures)
        SDL_DestroyTexture(t);
}

Piece::~Piece()
{
    static bool haveTexturesBeenFreed = false;
    if(!haveTexturesBeenFreed)
    {
        destoryTextures();
        haveTexturesBeenFreed = true;
    }
}

void Pawn::draw()
{
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &_destRect);
}

void Knight::draw()
{
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &_destRect);
}

void Rook::draw()
{
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &_destRect);
}

void Bishop::draw()
{
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &_destRect);
}

void King::draw()
{  
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &_destRect);
}

void Queen::draw()
{
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &_destRect);
}
//draw overrides end

//drawPieceOnMouse overrides begin
void Pawn::drawPieceOnMouse()
{
    int x = 0, y = 0;
    SDL_GetMouseState(&x, &y);
    SDL_Rect dest = {x - _destRect.w * 0.5f, y - _destRect.w * 0.5f, _destRect.w, _destRect.h};
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &dest);
}

void Knight::drawPieceOnMouse()
{
    int x = 0, y = 0;
    SDL_GetMouseState(&x, &y);
    SDL_Rect dest = {x - _destRect.w * 0.5f, y - _destRect.w * 0.5f, _destRect.w, _destRect.h};
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &dest);
}

void Rook::drawPieceOnMouse()
{
    int x = 0, y = 0;
    SDL_GetMouseState(&x, &y);
    SDL_Rect dest = {x - _destRect.w * 0.5f, y - _destRect.w * 0.5f, _destRect.w, _destRect.h};
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &dest);
}

void Bishop::drawPieceOnMouse()
{
    int x = 0, y = 0;
    SDL_GetMouseState(&x, &y);
    SDL_Rect dest = {x - _destRect.w * 0.5f, y - _destRect.w * 0.5f, _destRect.w, _destRect.h};
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &dest);
}

void King::drawPieceOnMouse()
{
    int x = 0, y = 0;
    SDL_GetMouseState(&x, &y);
    SDL_Rect dest = {x - _destRect.w * 0.5f, y - _destRect.w * 0.5f, _destRect.w, _destRect.h};
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &dest);
}

void Queen::drawPieceOnMouse()
{
    int x = 0, y = 0;
    SDL_GetMouseState(&x, &y);
    SDL_Rect dest = {x - _destRect.w * 0.5f, y - _destRect.w * 0.5f, _destRect.w, _destRect.h};
    SDL_RenderCopy(ChessApp::getCurrentRenderer(), _texture, &_sourceRect, &dest);
}
//drawPieceOnMouse overrides end

//calculatePseudoLegalMoves overrides begin
//used by rooks & queens
static void orthogonalSlide(int currFile, int currRank, Piece* slidingPiece, Board& b)
{
    //slide orthogonally left then right then down then up (from whites perspective)
    for(int i = 0; i < 4; ++i)
    {
        int offsetFile = currFile, offsetRank = currRank;

        if(i < 2)
            offsetFile = i == 0 ? currFile - 1 : currFile + 1;
        else//if i is 2 or 3
            offsetRank = i == 2 ? currRank - 1 : currRank + 1;

        for( ; ChessApp::inRange({offsetFile, offsetRank}); )
        {
            int offsetIndex = offsetRank * 8 + offsetFile;
            const Piece* const piece = b.getPieceAt(offsetIndex);

            if(!piece)//if there isnt a piece add it to the vector
            {
                slidingPiece->_pseudoLegalMoves.push_back(offsetIndex);
            }
            else//if there is a piece here
            {
                //if its the oposite color then add it to the vector
                if(slidingPiece->_isWhite != piece->_isWhite)
                    slidingPiece->_pseudoLegalMoves.push_back(offsetIndex);

                break;//stop sliding
            }

            //slide left or right or down or up depending on i 
            //(0 == left; 1 == right; 2 == down; 3 == up)
            if(i < 2)
                offsetFile = i == 0 ? offsetFile - 1 : offsetFile + 1;
            else//if i is 2 or 3
                offsetRank = i == 2 ? offsetRank - 1 : offsetRank + 1;
        }
    }
}

//used by queens & bishops
static void diagonalSlide(int currFile, int currRank, Piece* slidingPiece, Board& b)
{
    //slide diagonally bottom left then bottom right 
    //then top right then top left (from whites perspective)
    for(int i = 0; i < 4; ++i)
    {
        int offsetFile = currFile, offsetRank = currRank;

        offsetRank = (i < 2) ? offsetRank - 1 : offsetRank + 1;
        offsetFile = (i == 0 || i == 3) ? offsetFile - 1 : offsetFile + 1;

        for( ; ChessApp::inRange({offsetFile, offsetRank}); )
        {
            int offsetIndex = offsetRank * 8 + offsetFile;
            const Piece* const piece = b.getPieceAt(offsetIndex);

            if(!piece)//if there isnt a piece add it to the vector
            {
                slidingPiece->_pseudoLegalMoves.push_back(offsetIndex);
            }
            else//if there is a piece here
            {
                //if its the oposite color then add it to the vector
                if(slidingPiece->_isWhite != piece->_isWhite)
                    slidingPiece->_pseudoLegalMoves.push_back(offsetIndex);

                break;//stop sliding
            }

            if(i < 2)
            {
                --offsetRank;
                offsetFile = i == 0 ? offsetFile - 1 : offsetFile + 1;
            }
            else//if i is 2 or 3
            {
                ++offsetRank;
                offsetFile = i == 2 ? offsetFile + 1 : offsetFile - 1;
            }

        }
    }
}

void Pawn::calculatePseudoLegalMoves()
{
    Board& b = ChessApp::getBoard();
    _pseudoLegalMoves.clear();
    const auto [currFile, currRank] = _chessPos;
    int offsetFile = 0, offsetRank = _isWhite ? currRank + 1 : currRank - 1;

    //check the spot right in front of the pawn
    int offsetIndex = offsetRank * 8 + currFile;
    if(!b.getPieceAt(offsetIndex))
        _pseudoLegalMoves.push_back(offsetIndex);
    
    //if the pawn hasnt moved yet check if there isnt a piece two squares away
    if(currRank == 1 && _isWhite)
    {        
        if(!b.getPieceAt(offsetIndex + 8) && 
           !b.getPieceAt(offsetIndex))
            _pseudoLegalMoves.push_back(offsetIndex + 8);
    }
    else if(currRank == 6 && !_isWhite)
    {
        if(!b.getPieceAt(offsetIndex - 8) && 
           !b.getPieceAt(offsetIndex))
            _pseudoLegalMoves.push_back(offsetIndex - 8);
    }

    //next check if the pawn is attacking anything 
    //(see if there is an enemy piece to the top left or the top right of the pawn)

    //left side
    if(ChessApp::inRange({currFile - 1, offsetRank}))
    {
        offsetFile = currFile - 1;
        int offsetIndex = offsetRank * 8 + offsetFile;

        //if there is a piece to the top left of the pawn
        //or if the top left is an en passant target
        const Piece* const piece = b.getPieceAt(offsetIndex);
        if(piece)
        {
            //if the pieces are opposite sides
            if(_isWhite != piece->_isWhite)
                _pseudoLegalMoves.push_back(offsetIndex);
        }
        else if(offsetIndex == b.getEnPassantIndex())
        {
            _pseudoLegalMoves.push_back(offsetIndex);
        }
    }

    //right side
    if(ChessApp::inRange({currFile + 1, offsetRank}))
    {
        offsetFile = currFile + 1;
        int offsetIndex = offsetRank * 8 + offsetFile;

        //if there is a piece to the top left of the pawn
        //or if the top left is an en passant target
        const Piece* const piece = b.getPieceAt(offsetIndex);
        if(piece)
        {
            //if the pieces are opposite sides
            if(_isWhite != piece->_isWhite)
                _pseudoLegalMoves.push_back(offsetIndex);
        }
        else if(offsetIndex == b.getEnPassantIndex())
        {
            _pseudoLegalMoves.push_back(offsetIndex);
        }
    }

}

void Knight::calculatePseudoLegalMoves()
{
    Board& b = ChessApp::getBoard();
    _pseudoLegalMoves.clear();
    const auto [currFile, currRank] = this->_chessPos;
    int fileOffset = 0, rankOffset = 0, offsetIndex = 0;

    auto potentialPushBack = [&]()-> void
    {
        if(ChessApp::inRange({fileOffset, rankOffset}))
        {
            offsetIndex = rankOffset * 8 + fileOffset;
            const Piece* const piece = b.getPieceAt(offsetIndex);
            if(!piece)
            {
                _pseudoLegalMoves.push_back(offsetIndex);
            }
            else if(this->_isWhite != piece->_isWhite)
            {
                _pseudoLegalMoves.push_back(offsetIndex);
            }
        }
    };

    //check all 8 possible chess positions in this order
    //(this is from whites perspective)    
    //                 |_|8|_|7|_|                    
    //                 |1|_|_|_|6|                    
    //                 |_|_|N|_|_|                    
    //                 |2|_|_|_|5|                    
    //                 |_|3|_|4|_|                                

    fileOffset = currFile - 2; rankOffset = currRank + 1;
    potentialPushBack();//left two up one from knight
    
    rankOffset -= 2;
    potentialPushBack();//left two down one from knight
    
    ++fileOffset; --rankOffset;
    potentialPushBack();//left one and down two from knight

    fileOffset += 2;
    potentialPushBack();//right one and down two from kngith

    ++fileOffset; ++rankOffset;
    potentialPushBack();//right two and down one from knight

    rankOffset += 2;
    potentialPushBack();//right two and up one from kngiht

    --fileOffset; ++rankOffset;
    potentialPushBack();//right one and up two from knight

    fileOffset -= 2;
    potentialPushBack();//left one and up two from knight
}

void Rook::calculatePseudoLegalMoves()
{
    Board& b = ChessApp::getBoard();
    _pseudoLegalMoves.clear();
    const auto [currFile, currRank] = this->_chessPos;
    orthogonalSlide(currFile, currRank, this, b);
}

void Bishop::calculatePseudoLegalMoves()
{
    Board& b = ChessApp::getBoard();
    _pseudoLegalMoves.clear();
    const auto [currFile, currRank] = this->_chessPos;
    diagonalSlide(currFile, currRank, this, b);
}

void King::calculatePseudoLegalMoves()
{
    Board& b = ChessApp::getBoard();
    _pseudoLegalMoves.clear();
    const auto [currFile, currRank] = this->_chessPos;
    const int currIndex = currFile + currRank * 8;

    //check all 8 possible chess positions in this order  
    //(this is from whites perspective)                            
    //|3|5|8|                    
    //|2|K|7|                   
    //|1|4|6|                   

    for(int offsetFile = currFile - 1; offsetFile < currFile + 2; ++offsetFile)
    {
        for(int offsetRank = currRank - 1; offsetRank < currRank + 2; ++offsetRank)
        {
            if(!ChessApp::inRange({offsetFile, offsetRank}))
                continue;

            int offsetIndex = offsetRank * 8 + offsetFile;
            const Piece* const piece = b.getPieceAt(offsetIndex);

            if(piece == this)
                continue;                                     

            if(!piece)
                _pseudoLegalMoves.push_back(offsetIndex);        
            else if(piece->_isWhite != this->_isWhite)        
                _pseudoLegalMoves.push_back(offsetIndex);               
        }
    }

    if(_isWhite)
    {
        if(b.canCastle(Board::WLONG))
        {
            if(!b.getPieceAt(currIndex - 1) && !b.getPieceAt(currIndex - 2))
            {
                _pseudoLegalMoves.push_back(currIndex - 1);
                _pseudoLegalMoves.push_back(currIndex - 2);
            }
        }
        if(b.canCastle(Board::WSHORT))
        {
            if(!b.getPieceAt(currIndex + 1) && !b.getPieceAt(currIndex + 2))
            {
                _pseudoLegalMoves.push_back(currIndex + 1);
                _pseudoLegalMoves.push_back(currIndex + 2);
            }
        }
    }
    else//if *this is a black king
    {
        if(b.canCastle(Board::BLONG))
        {
            if(!b.getPieceAt(currIndex - 1) && !b.getPieceAt(currIndex - 2))
            {
                _pseudoLegalMoves.push_back(currIndex - 1);
                _pseudoLegalMoves.push_back(currIndex - 2);
            }
        }
        if(b.canCastle(Board::BSHORT))
        {
            if(!b.getPieceAt(currIndex + 1) && !b.getPieceAt(currIndex + 2))
            {
                _pseudoLegalMoves.push_back(currIndex + 1);
                _pseudoLegalMoves.push_back(currIndex + 2);
            }
        }
    }
}

void Queen::calculatePseudoLegalMoves()
{
    Board& b = ChessApp::getBoard();
    _pseudoLegalMoves.clear();
    const auto [currFile, currRank] = this->_chessPos;
    diagonalSlide(currFile, currRank, this, b);
    orthogonalSlide(currFile, currRank, this, b);
}
//calculatePseudoLegalMoves overrides end

void Piece::updateInternalPosition(Vector2i newChessPos)
{   
    _chessPos.x = newChessPos.x; _chessPos.y = newChessPos.y;
    _screenPos = ChessApp::chess2ScreenPos(newChessPos);
    _destRect.x = _screenPos.x - _destRect.w * 0.5f;
    _destRect.y = _screenPos.y - _destRect.h * 0.5f;
}

void Pawn::update(Vector2i newChessPos, Board& b)
{
    int newIndex = newChessPos.y * 8 + newChessPos.x;
    int oldIndex = _chessPos.y * 8 + _chessPos.x;

    //if the capture was an en passant move
    if(newIndex == b.getEnPassantIndex())
    {
        int rankOfPawnToCapture = _chessPos.y;
        b.capturePiece(rankOfPawnToCapture * 8 + newChessPos.x);
        b.resetEnPassant();
    }
    //if the move was a double push
    else if(abs(newIndex - oldIndex) == 16)
    {
        int enPassantRank = _isWhite ? newChessPos.y - 1 : newChessPos.y + 1;
        b.updateEnPassant(newChessPos.x, enPassantRank);
    }
    else//the move was a normal push/capture
    {
        //if the pawn reached the end of the board
        if(newChessPos.y == 7 || newChessPos.y == 0)
            ChessApp::setPromotionQueue({newIndex, _isWhite});

        b.resetEnPassant();
    }

    updateInternalPosition(newChessPos);
}

void Knight::update(Vector2i newChessPos, Board& b)
{
    updateInternalPosition(newChessPos);
    b.resetEnPassant();
}

void Rook::update(Vector2i newChessPos, Board& b)
{
    //remove castling rights for the correct side first.
    //the rook could technically go back to its starting square and then move
    //which would remove the castling rights for that side again but its okay
    if(_isWhite)
    {
        const Vector2i a1{0, 0}, h1{7, 0};
        if(_chessPos == a1)
            b.removeCastlingRights(Board::WLONG);    
        else if(_chessPos == h1)
            b.removeCastlingRights(Board::WSHORT);
    }
    else//if *this is a black rook
    {
        const Vector2i a8{0, 7}, h8{7, 7};
        if(_chessPos == a8)
            b.removeCastlingRights(Board::BLONG);
        else if(_chessPos == h8)
            b.removeCastlingRights(Board::BSHORT);
    }

    updateInternalPosition(newChessPos);
    b.resetEnPassant();
}

void Bishop::update(Vector2i newChessPos, Board& b)
{
    updateInternalPosition(newChessPos);
    b.resetEnPassant();
}

void Queen::update(Vector2i newChessPos, Board& b)
{
    updateInternalPosition(newChessPos);
    b.resetEnPassant();
}

void King::update(Vector2i newChessPos, Board& b)
{    
    if(newChessPos.x - _chessPos.x == 2)//if short castle
    {
        b.completeCastle(_isWhite ? Board::WSHORT : Board::BSHORT);
    }
    else if(newChessPos.x - _chessPos.x == -2)//if long castle
    {
        b.completeCastle(_isWhite ? Board::WLONG : Board::BLONG);
    }

    updateInternalPosition(newChessPos);
    b.resetEnPassant();
    if(_isWhite)
    {
        b.removeCastlingRights(Board::WLONG);
        b.removeCastlingRights(Board::WSHORT);
    }
    else//if *this is a black piece
    {
        b.removeCastlingRights(Board::BLONG);
        b.removeCastlingRights(Board::BSHORT);
    }
}

void Piece::initTextures()
{
    SDL_Renderer* renderer = ChessApp::getCurrentRenderer();
    if(!(s_textures[WPAWN]   = IMG_LoadTexture(renderer, "textures/whitePawn256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[WKNIGHT] = IMG_LoadTexture(renderer, "textures/whiteKnight256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[WROOK]   = IMG_LoadTexture(renderer, "textures/whiteRook256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[WBISHOP] = IMG_LoadTexture(renderer, "textures/whiteBishop256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[WQUEEN]  = IMG_LoadTexture(renderer, "textures/whiteQueen256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[WKING]   = IMG_LoadTexture(renderer, "textures/whiteKing256.png")))
        std::cerr << IMG_GetError();

    if(!(s_textures[BPAWN]   = IMG_LoadTexture(renderer, "textures/blackPawn256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[BKNIGHT] = IMG_LoadTexture(renderer, "textures/blackKnight256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[BROOK]   = IMG_LoadTexture(renderer, "textures/blackRook256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[BBISHOP] = IMG_LoadTexture(renderer, "textures/blackBishop256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[BQUEEN]  = IMG_LoadTexture(renderer, "textures/blackQueen256.png")))
        std::cerr << IMG_GetError();
    if(!(s_textures[BKING]   = IMG_LoadTexture(renderer, "textures/blackKing256.png")))
        std::cerr << IMG_GetError(); 
}