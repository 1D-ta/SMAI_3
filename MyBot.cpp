#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
#include <climits>
#include <algorithm>
#include <cmath>
#include <chrono>

using namespace std;
using namespace Desdemona;

class MyBot : public OthelloPlayer {
public:
    MyBot(Desdemona::Turn turn);
    virtual Move play(const OthelloBoard& board);

private:
    // Evaluation weights for different board positions
    static const int CORNER_WEIGHT = 25;
    static const int EDGE_WEIGHT = 10;
    static const int MOBILITY_WEIGHT = 5;

    // Get opponent's turn
    Desdemona::Turn getOpponentTurn() const {
    return (turn == Desdemona::Coin::BLACK) ? Desdemona::Coin::RED : Desdemona::Coin::BLACK;
}

    // Alpha-beta pruning with depth-limited search
    int alphaBetaPruning(const OthelloBoard& board, int depth, int alpha, int beta, bool isMaximizingPlayer);
    
    // Evaluate the board state
    int evaluateBoard(const OthelloBoard& board);
    
    // Helper functions
    int countPieces(const OthelloBoard& board, Desdemona::Turn player);
    bool isCorner(const Move& move);
    bool isEdge(const Move& move);
};

MyBot::MyBot(Desdemona::Turn turn) : OthelloPlayer(turn) {}

// Main play function using alpha-beta pruning
Move MyBot::play(const OthelloBoard& board) {
    list<Move> validMoves = board.getValidMoves(turn);
    
    if (validMoves.empty()) {
        return Move(-1, -1); // Pass if no valid moves (return an invalid move)
    }

    Move bestMove(-1, -1); // Initialize with an invalid move
    int bestScore = INT_MIN;
    int depth = 4; // Depth of search tree

    auto start = chrono::high_resolution_clock::now(); // Start timer

    for (const Move& move : validMoves) {
        OthelloBoard tempBoard = board;
        tempBoard.makeMove(turn, move);

        int score = alphaBetaPruning(tempBoard, depth - 1, INT_MIN, INT_MAX, false);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }

        // Check if 2 seconds have passed
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end - start;
        if (elapsed.count() > 2.0) {
            break; // Exit if time limit exceeded
        }
    }

    return bestMove;
}

// Alpha-beta pruning algorithm
int MyBot::alphaBetaPruning(const OthelloBoard& board, int depth, int alpha, int beta, bool isMaximizingPlayer) {
    // Terminal state or depth limit reached
    if (depth == 0) {
        return evaluateBoard(board);
    }

    // Get opponent turn using helper method
    Desdemona::Turn opponentTurn = getOpponentTurn();

    list<Move> validMoves = board.getValidMoves(isMaximizingPlayer ? turn : opponentTurn);
    
    if (validMoves.empty()) {
        // If no moves available, evaluate current board
        return evaluateBoard(board);
    }

    if (isMaximizingPlayer) {
        int maxEval = INT_MIN;
        for (const Move& move : validMoves) {
            OthelloBoard tempBoard = board;
            tempBoard.makeMove(turn, move);
            
            int eval = alphaBetaPruning(tempBoard, depth - 1, alpha, beta, false);
            maxEval = max(maxEval, eval);
            alpha = max(alpha, eval);

            if (beta <= alpha) {
                break; // Beta cut-off
            }
        }
        return maxEval;
    } else {
        int minEval = INT_MAX;
        for (const Move& move : validMoves) {
            OthelloBoard tempBoard = board;
            Desdemona::Turn opponentTurn = getOpponentTurn();
            tempBoard.makeMove(opponentTurn, move);
            
            int eval = alphaBetaPruning(tempBoard, depth - 1, alpha, beta, true);
            minEval = min(minEval, eval);
            beta = min(beta, eval);

            if (beta <= alpha) {
                break; // Alpha cut-off
            }
        }
        return minEval;
    }
}

// Board evaluation function
int MyBot::evaluateBoard(const OthelloBoard& board) {
    int myPieces = countPieces(board, turn);
    Desdemona::Turn opponentTurn = getOpponentTurn();
    int opponentPieces = countPieces(board, opponentTurn);
    
    int evaluation = myPieces - opponentPieces;

    // Bonus for controlling corners
    list<Move> validMoves = board.getValidMoves(turn);
    for (const Move& move : validMoves) {
        if (isCorner(move)) {
            evaluation += CORNER_WEIGHT;
        } else if (isEdge(move)) {
            evaluation += EDGE_WEIGHT;
        }
    }

    // Mobility bonus
    evaluation += MOBILITY_WEIGHT * (validMoves.size() - board.getValidMoves(opponentTurn).size());

    return evaluation;
}

// Count pieces for a specific player
int MyBot::countPieces(const OthelloBoard& board, Desdemona::Turn player) {
    int count = 0;
    // Assuming 8x8 board, replace with actual board size if different
    const int BOARD_SIZE = 8;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board.get(i, j) == player) {
                count++;
            }
        }
    }
    return count;
}

// Check if a move is a corner
bool MyBot::isCorner(const Move& move) {
    // Assuming 8x8 board, replace with actual board size if different
    const int BOARD_SIZE = 8;
    return (move.x == 0 || move.x == BOARD_SIZE - 1) && 
           (move.y == 0 || move.y == BOARD_SIZE - 1);
}

// Check if a move is on an edge
bool MyBot::isEdge(const Move& move) {
    // Assuming 8x8 board, replace with actual board size if different
    const int BOARD_SIZE = 8;
    return move.x == 0 || move.x == BOARD_SIZE - 1 || 
           move.y == 0 || move.y == BOARD_SIZE - 1;
}

// Required external functions for bot creation
extern "C" {
    OthelloPlayer* createBot(Desdemona::Turn turn) {
        return new MyBot(turn);
    }

    void destroyBot(OthelloPlayer* bot) {
        delete bot;
    }
}