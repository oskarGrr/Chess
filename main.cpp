#include "ChessApplication.h"

int main(int argCount, char* arguments[])
{
    ChessApp& app = ChessApp::getApp();
    app.run();
    return EXIT_SUCCESS;
}