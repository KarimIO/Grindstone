#include "DLLGame.hpp"
#include <stdexcept>

DLLGame::DLLGame() {
    initialize("Game");

    auto fnLaunchGame = (void (*)())getFunction("launchGame");
    if (!fnLaunchGame) {
        throw std::runtime_error("Cannot start game!\n");
    }

    auto fnDeleteGame = (void (*)())getFunction("launchGame");
    if (!fnLaunchGame) {
        throw std::runtime_error("Cannot start game!\n");
    }

    fnLaunchGame();
}

DLLGame::~DLLGame() {
}