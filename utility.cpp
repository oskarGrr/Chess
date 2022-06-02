#include "PieceTypes.h"
#include <iostream>

//return the file and rank (x and y) from a position on the screen in pixels.
//The pieces will be drawn in different screen positions for the client with
//the black pieces and the client with the white pieces (obviously). So Piece::screenPos
//will differ between clients. But the layout of where the pieces are in memory
//and the chess positions returned from this function will be the same for both
//clients even though they will be draw at different locations 
//(because they are viewing the board from different perspectives)
