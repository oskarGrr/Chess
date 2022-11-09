#pragma once
#include "Vector2i.h"
#include "Board.h"
#include "Window.h"
#include "WavSound.h"
#include "SDL.h"
#include "imgui.h"

#define NUM_OF_PIECE_TEXTURES 12 //6 types of pieces * 2 for both sides

//singleton chessApp that contains the board (which owns/"holds" the pieces)
class ChessApp
{
public:
//indecies into the array of SDL textures for the different pieces.
//scoped to ChessApp but is an unscoped enum to allow for convenient 
//implicit conversion to the underlying type 
//(the textures are in the single chess app instance as m_pieceTextures)
enum TextureIndices : Uint32
{
    WPAWN, WKNIGHT, WROOK, WBISHOP, WQUEEN, WKING,
    BPAWN, BKNIGHT, BROOK, BBISHOP, BQUEEN, BKING, INVALID
};

private:

    ChessApp();//called before main() and thus also inits the board and the pieces before main()
    static ChessApp s_theApplication;   

public:

    ChessApp(ChessApp const&)=delete;
    ChessApp(ChessApp&&)=delete;
    ChessApp& operator=(ChessApp const&)=delete;
    ChessApp& operator=(ChessApp&&)=delete;
    ~ChessApp();

    void run();//main application loop

    //getter methods
    inline static ChessApp& getApp(){return s_theApplication;};
    inline static Board& getBoard(){return s_theApplication.m_board;};
    inline static Uint32 getSquareSize(){return s_theApplication.m_squareSize;};
    inline auto const& getTextures() const {return m_pieceTextures;}
    inline static SDL_Renderer* getCurrentRenderer(){return s_theApplication.m_wnd.m_renderer;};
    inline static bool isPromotionWndOpen(){return s_theApplication.m_wnd.m_promotionWindowIsOpen;}

    //helper methods 
    inline static int chessPos2Index(Vec2i const pos){return pos.y * 8 + pos.x;};
    inline static Vec2i index2ChessPos(int const index){return{index % 8,index / 8};};
    static Vec2i chess2ScreenPos(Vec2i const);
    static Vec2i screen2ChessPos(Vec2i const);
    static bool isScreenPositionOnBoard(Vec2i const);//tells if a screen pos is on the board or if its off the screen/board or is over an imgui window
    static bool inRange(Vec2i const chessPos);//tells if a chess position is within the range of a chess board (0-7 ranks and a-h files)
    static bool isMouseOver(SDL_Rect const&);//tells wether mouse position is over a given rectangle

    //play audio methods
    inline static void playChessMoveSound(){s_theApplication.m_pieceMoveSound.playFullSound();}
    inline static void playChessCastleSound(){s_theApplication.m_pieceCastleSound.playFullSound();}
    inline static void playChessCaptureSound(){s_theApplication.m_pieceCaptureSound.playFullSound();}

    //allow a promotion window to open on the next iteration of the main loop
    inline static void queuePromotionWndToOpen(){s_theApplication.m_wnd.m_promotionWindowIsOpen = true;}

private:

    //methods that are called at the begin and end of renderAllTheThings() 
    void endFrame();
    void beginFrame();
    void renderAllTheThings();
    bool processEvents();
    void flipBoard();
    void drawSquares();
    void drawPiecesNotOnMouse();
    void drawPieceOnMouse();
    void drawMoveIndicatorCircles();
    void initCircleTexture(int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Texture** toInit);
    void loadPieceTexturesFromDisk();
    void promotionRoutine();

    //called from renderPromotionWnd() when a piece is clicked on
    template<typename pieceTy> void finalizePromotion(Vec2i const& promotionSquare, bool const wasCapture);

    //the only way the promotion window should be closed is via the user clicking on a button to select
    //a piece to promote to then that move will be completed. so this should only be called when one
    //of the buttons in the promotion window is clicked on.
    inline void closePromotionWindow(){s_theApplication.m_wnd.m_promotionWindowIsOpen = false;}

    Uint32 const m_chessBoardWidth;
    Uint32 const m_chessBoardHeight;
    Uint32 m_squareSize;//square size in pixels
    float  m_menuBarHeight;
    Window   m_wnd;//my simple wrapper class for SDL window
    WavSound m_pieceMoveSound;
    WavSound m_pieceCastleSound;
    WavSound m_pieceCaptureSound;
    std::array<Uint8, 4> m_lightSquareColor;
    std::array<Uint8, 4> m_darkSquareColor;
    Board  m_board;//the singleton board composed here as part of the ChessApp instance
    SDL_Texture* m_circleTexture;
    SDL_Texture* m_redCircleTexture;
    float const  m_pieceTextureScale;
    std::array<SDL_Texture*, NUM_OF_PIECE_TEXTURES> m_pieceTextures;//the textures for the chess pieces
    std::array<Vec2i,        NUM_OF_PIECE_TEXTURES> m_pieceTextureSizes;//width and height of the above textures
};