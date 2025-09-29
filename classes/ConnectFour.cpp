#include "ConnectFour.h"
#include <iostream>


ConnectFour::ConnectFour()
{
    _grid = new Grid(7, 6);
}

ConnectFour::~ConnectFour()
{
    delete _grid;
}

Bit* ConnectFour::PieceForPlayer(const int playerNumber)
{
    // depending on playerNumber load the "red.png" or the "yellow.png" graphic
    Bit *bit = new Bit();
    bit->LoadTextureFromFile(playerNumber == AI_PLAYER ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(playerNumber == AI_PLAYER ? 1 : 0));
    return bit;
}

void ConnectFour::setUpBoard()
{
    setNumberOfPlayers(2);
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;
    _grid->initializeSquares(80, "square.png");

    if (gameHasAI()) {
        setAIPlayer(AI_PLAYER);
    }

    startGame();
}

bool ConnectFour::actionForEmptyHolder(BitHolder &holder)
{
    if (holder.bit()) {
        return false;
    }
    Bit *bit = PieceForPlayer(getCurrentPlayer()->playerNumber() == 0 ? HUMAN_PLAYER : AI_PLAYER);
    if (bit) {

        //Gravity Logic TODO ignore mosue pos, place at bottom most cell
        int gridX = ((holder.getPosition().x /40) - 1) /2;
        int gridY = ((holder.getPosition().y /40) - 1) /2;
        int index = static_cast<int>(7 * gridY + gridX);
        if(index >= 35 || ownerAt(index)){ //Bottom Row or there's a piece beneath it
            bit->setPosition(holder.getPosition());
            holder.setBit(bit);
            endTurn();
            return true; 
        }
    }   
    return false;
}

bool ConnectFour::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    return false;
}

bool ConnectFour::canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{
    return false;
}

//
// free all the memory used by the game on the heap
//
void ConnectFour::stopGame()
{
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

//
// helper function for the winner check
//
Player* ConnectFour::ownerAt(int index ) const
{
    auto square = _grid->getSquare(index % 7, index / 7);
    if (!square || !square->bit()) {
        return nullptr;
    }
    return square->bit()->getOwner();
}

Player* ConnectFour::checkForWinner()
{
    for( int i=0; i<42; i++ ) { //Could change i to 39, time save would be microscpoic
        Player *player = ownerAt(i);

        //Right facing win check (3 Adjacent on the right)
        if(i % 7 <= 3){
            if(player && player == ownerAt(i+1) && player == ownerAt(i+2) && player == ownerAt(i+3)){
                return player;
            }
        }

        //Down facing win check (3 Adjacent downward)
        if(i <= 20){
            if(player && player == ownerAt(i+7) && player == ownerAt(i+14) && player == ownerAt(i+21)){
                return player;
            }
        }

        //Down and right facing win check (3 Adjacent going southeast)
        if(i <= 17 && i % 7 <= 3){
            if(player && player == ownerAt(i+8) && player == ownerAt(i+16) && player == ownerAt(i+24)){
                return player;
            }
        }

        //Down and left facing win check (3 Adjacent going southwest)
        if(i <= 20 && i % 7 >= 3){
            if(player && player == ownerAt(i+6) && player == ownerAt(i+12) && player == ownerAt(i+18)){
                return player;
            }
        }
    }
    return nullptr;
}

bool ConnectFour::checkForDraw()
{
   bool isDraw = true;
    // check to see if the board is full
    _grid->forEachSquare([&isDraw](ChessSquare* square, int x, int y) {
        if (!square->bit()) {
            isDraw = false;
        }
    });
    return isDraw;
}

//
// state strings
//
std::string ConnectFour::initialStateString()
{
    return  "0000000"
            "0000000"
            "0000000"
            "0000000"
            "0000000"
            "0000000";
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string ConnectFour::stateString()
{
    std::string s = "000000000000000000000000000000000000000000";
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        Bit *bit = square->bit();
        if (bit) {
            s[y * 7 + x] = std::to_string(bit->getOwner()->playerNumber()+1)[0];
        }
    });
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void ConnectFour::setStateString(const std::string &s)
{
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y*7 + x;
        int playerNumber = s[index] - '0';
        if (playerNumber) {
            square->setBit( PieceForPlayer(playerNumber-1) );
        } else {
            square->setBit( nullptr );
        }
    });
}


//
// this is the function that will be called by the AI
//
void ConnectFour::updateAI() 
{
    int initialA = -10000000;
    int initialB = 10000000;
    int bestVal = -1000;
    BitHolder* bestMove = nullptr;
    std::string state = stateString();

    // Traverse all cells, evaluate minimax function for all empty cells
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        int index = y * 7 + x;
        // Check if cell is empty
        if (state[index] == '0') {
            // Make the move
            state[index] = '2';
            int moveVal = -negamax(state, 0, initialA, initialB, HUMAN_PLAYER);
            // Undo the move
            state[index] = '0';
            // If the value of the current move is more than the best value, update best
            if (moveVal > bestVal) {
                bestMove = square;
                bestVal = moveVal;
            }
        }
    });

    // Make the best move
    if(bestMove) {
        if (actionForEmptyHolder(*bestMove)) {
        }
    }
}

bool ConnectFour::isAIBoardFull(const std::string& state) {
    return state.find('0') == std::string::npos;
}

int ConnectFour::evaluateAIBoard(const std::string& state) {
    for( int i=0; i<42; i++ ) { //Could change i to 39, time save would be microscpoic
        char first = state[i];

        //Right facing win check (3 Adjacent on the right)
        if(i % 7 <= 3){
            if(first != '0' && first == state[i+1] && first == state[i+2] && first == state[i+3]){
                return 30;
            }
        }

        //Down facing win check (3 Adjacent downward)
        if(i <= 20){
            if(first != '0' && first == state[i+7] && first == state[i+14] && first == state[i+21]){
                return 30;
            }
        }

        //Down and right facing win check (3 Adjacent going southeast)
        if(i <= 17 && i % 7 <= 3){
            if(first != '0' && first == state[i+8] && first == state[i+16] && first == state[i+24]){
                return 30;
            }
        }

        //Down and left facing win check (3 Adjacent going southwest)
        if(i <= 20 && i % 7 >= 3){
            if(first != '0' && first == state[i+6] && first == state[i+12] && first == state[i+18]){
                return 30;
            }
        }
    }
    return 0; // No winner
}

//
// player is the current player's number (AI or human)
//
int ConnectFour::negamax(std::string& state, int depth, int a, int b, int playerColor)
{
    //Initial depth
    if(depth >= 3){
        return 0;
    }

    int score = evaluateAIBoard(state);

    //If winner exists, return the scoring
    if(score){
        return -score;
    }

    //If board is full and no winner, DRAW
    if (isAIBoardFull(state)) {
        return 0;
    }

    int bestVal = -1000;
    for(int i=0; i<42; i++){
        if(state[i] == '0'){
            state[i] = playerColor == HUMAN_PLAYER ? '1' : '2';
            bestVal = std::max(bestVal, -negamax(state, depth+1, -b, -a, -playerColor));
            state[i] = '0';
            a = std::max(a, bestVal);
            if(a >= b){
                break;
            }
        }
    }
    return bestVal;
}
