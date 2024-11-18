#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
#include <list>
#include <chrono>
using namespace std;
using namespace Desdemona;
#include <thread>
#include <vector>
#include <mutex>
#include <chrono>

class MyBot: public OthelloPlayer {
public:
	MyBot(Turn turn);
	virtual Move play(const OthelloBoard& board);
	virtual int beta(const OthelloBoard& board, int depth, int alpha_min, int beta_max);
	virtual int alpha(const OthelloBoard& board, int depth, int alpha_min, int beta_max);
	virtual int heuristic(const OthelloBoard& board, Turn turn);
	virtual int heu_stat(const OthelloBoard& board, Turn turn);
	virtual int heu_brute(const OthelloBoard& board, Turn turn);
	virtual int mobilityHeuristic(const OthelloBoard& board, Turn turn);
	virtual int cornerHeuristic(const OthelloBoard& board, Turn turn);
	virtual int evaluateBeta(const OthelloBoard& board, int depth, int alpha, int beta);

private:
	int move_num = 0;
	Turn my_color;
};

MyBot::MyBot(Turn turn) : OthelloPlayer(turn) {}

Move MyBot::play(const OthelloBoard& board) {
	move_num++;
	if (move_num == 1) my_color = turn;
    
	// Calculate dynamic depth based on the number of available moves
	int dynamicDepth = 2; // Default depth
	list<Move> moves = board.getValidMoves(turn);
	int numMoves = moves.size();
    
	if (numMoves < 10) {
    	dynamicDepth = 3; // Increase depth for early game
	} else if (numMoves < 20) {
    	dynamicDepth = 3; // Moderate depth for mid-game
	} else if (numMoves < 40) {
    	dynamicDepth = 2; // Decrease depth for late-mid game
	}
    
	// Set up variables for time measurement
	auto startTime = std::chrono::high_resolution_clock::now();
	auto endTime = startTime + std::chrono::seconds(2); // 2-second time limit
    
	int max_a = -400;
	int a;
	Move next_move = *moves.begin();
    
	for (list<Move>::iterator it = moves.begin(); it != moves.end(); ++it) {
    	Turn trn = turn;
    	OthelloBoard my_board = board;
    	my_board.makeMove(trn, *it);
    	a = evaluateBeta(my_board, dynamicDepth, max_a, 400);
   	 
    	// Check if time limit has been reached
    	auto currentTime = std::chrono::high_resolution_clock::now();
    	if (currentTime >= endTime) {
        	// If time limit exceeded, return the current best move
        	return next_move;
    	}
   	 
    	if (a > max_a) {
        	max_a = a;
        	next_move = *it;
    	}
	}
    
	return next_move;
}


int MyBot::beta(const OthelloBoard& board, int depth, int alpha_min, int beta_max){
  Turn turn_b = (turn==RED)?BLACK:RED;
  list<Move> moves = board.getValidMoves( turn_b );
  list<Move>::iterator it = moves.begin();
  int min_b = beta_max;
  int b;
  for(int i=0; i < moves.size(); it++, i++){
	Turn trn_b = turn_b;
	OthelloBoard my_board = board;
	my_board.makeMove(trn_b,*it);
	b = alpha(my_board,depth-1,alpha_min,min_b);
	if(b<min_b){
  	min_b = b;
  	if(min_b<alpha_min)return alpha_min;
	}
  }
  return min_b;
}

int MyBot::evaluateBeta(const OthelloBoard& board, int depth, int alpha_min, int beta_min) {
    using namespace std::chrono;
    auto startTime = steady_clock::now();
    long timeLimit = 30; // 2 seconds in milliseconds

    Turn opponentTurn = (turn==RED) ? BLACK : RED;
    list<Move> validMoves = board.getValidMoves(opponentTurn);
	// cout << validMoves.size() << endl;
    int minBeta = beta_min;

    for (const auto& move : validMoves) {
        // Check the elapsed time
        auto currentTime = steady_clock::now();
        long elapsedTime = duration_cast<milliseconds>(currentTime - startTime).count();
        if (elapsedTime >= timeLimit) {
            // Time limit reached, return current best score
			// cout << "time limit" << endl;
            return minBeta;
        }

        OthelloBoard tempBoard = board;
        tempBoard.makeMove(opponentTurn, move);
        int score = alpha(tempBoard, depth - 1, alpha_min, minBeta);
        if (score < minBeta) {
            minBeta = score;
            if (minBeta <= alpha_min) return alpha_min;
        }
    }
    return minBeta;
}




int MyBot::alpha(const OthelloBoard& board, int depth, int alpha_min, int beta_max){

	using namespace std::chrono;
    auto startTime = steady_clock::now();
    long timeLimit = 100; // 2 seconds in milliseconds

  if(depth == 0){
	OthelloBoard my_board = board;
	return heuristic(my_board,turn);
  }
  list<Move> moves = board.getValidMoves( turn );
  list<Move>::iterator it = moves.begin();
  int max_a = alpha_min;
  int a;
  for(int i=0; i < moves.size(); it++, i++){
	// Check the elapsed time
        auto currentTime = steady_clock::now();
        long elapsedTime = duration_cast<milliseconds>(currentTime - startTime).count();
        if (elapsedTime >= timeLimit) {
            // Time limit reached, return current best score
			// cout << "time limit" << endl;
            return max_a;
        }

	Turn trn = turn;
	OthelloBoard my_board = board;
	my_board.makeMove(trn,*it);
	a = beta(my_board,depth,max_a,beta_max);
	if(a>max_a){
  	max_a = a;
  	if(max_a>beta_max)return beta_max;
	}
  }
  return max_a;
}

int MyBot::heuristic(const OthelloBoard& board, Turn turn) {
	if (move_num < 20) {
    	return heu_stat(board, turn);
	} else if (move_num < 40) {
    	return heu_stat(board, turn) + mobilityHeuristic(board, turn);
	} else {
    	return heu_brute(board, turn);
	}
}

int MyBot::heu_stat(const OthelloBoard& board, Turn turn) {
	// Positional values assigned to each square on the board
	const int positionalValues[8][8] = {
    	{20, -3, 11, 8, 8, 11, -3, 20},
    	{-3, -7, -4, 1, 1, -4, -7, -3},
    	{11, -4, 2, 2, 2, 2, -4, 11},
    	{ 8,  1, 2, -3, -3, 2,  1,  8},
    	{ 8,  1, 2, -3, -3, 2,  1,  8},
    	{11, -4, 2, 2, 2, 2, -4, 11},
    	{-3, -7, -4, 1, 1, -4, -7, -3},
    	{20, -3, 11, 8, 8, 11, -3, 20}
	};

	// const double positionalValues[8][8] = {
    // 	{1.01, -0.43, 0.38, 0.07, 0.00, 0.42, -0.2, 1.02},
    // 	{-0.27, -0.74, -0.16, -0.14, -0.13, -0.25, -0.65, -0.39},
    // 	{0.56, -0.3, 0.12, 0.05, -0.04, 0.07, -0.15, 0.48},
    // 	{ 0.01,  -0.08, 0.01, -0.01, -0.04, -0.02,  -0.12,  0.03},
    // 	{ -0.1,  -0.08, 0.01, -0.01, -0.03, 0.02,  -0.04,  -0.2},
    // 	{0.59, -0.23, 0.06, 0.01, 0.04, 0.06, -0.19, 0.35},
    // 	{-0.06, -0.55, -0.18, -0.08, -0.15, -0.31, -0.82, -0.58},
    // 	{0.96, -0.42, 0.67, -0.02, -0.03, 0.81, -0.51, 1.01}
	// };

	double positionalScore = 0;
	for (int x = 0; x < 8; ++x) {
    	for (int y = 0; y < 8; ++y) {
        	if (board.get(x, y) == turn) {
            	positionalScore += positionalValues[x][y];
        	} else if (board.get(x, y) == other(turn)) {
            	positionalScore -= positionalValues[x][y];
        	}
    	}
	}

	int mobilityScore = mobilityHeuristic(board, turn);
	int cornerScore = cornerHeuristic(board, turn);
	return positionalScore + 2 * mobilityScore + 5 * cornerScore;
}


int MyBot::heu_brute(const OthelloBoard& board, Turn turn) {
	// More aggressive positional values for the late game
	const int positionalValues[8][8] = {
    	{25, -5, 10, 7, 7, 10, -5, 25},
    	{-5, -10, -3, 1, 1, -3, -10, -5},
    	{10, -3, 5, 3, 3, 5, -3, 10},
    	{ 7,  1, 3, 0, 0, 3,  1,  7},
    	{ 7,  1, 3, 0, 0, 3,  1,  7},
    	{10, -3, 5, 3, 3, 5, -3, 10},
    	{-5, -10, -3, 1, 1, -3, -10, -5},
    	{25, -5, 10, 7, 7, 10, -5, 25}
	};

	// const double positionalValues[8][8] = {
    // 	{1.01, -0.43, 0.38, 0.07, 0.00, 0.42, -0.2, 1.02},
    // 	{-0.27, -0.74, -0.16, -0.14, -0.13, -0.25, -0.65, -0.39},
    // 	{0.56, -0.3, 0.12, 0.05, -0.04, 0.07, -0.15, 0.48},
    // 	{ 0.01,  -0.08, 0.01, -0.01, -0.04, -0.02,  -0.12,  0.03},
    // 	{ -0.1,  -0.08, 0.01, -0.01, -0.03, 0.02,  -0.04,  -0.2},
    // 	{0.59, -0.23, 0.06, 0.01, 0.04, 0.06, -0.19, 0.35},
    // 	{-0.06, -0.55, -0.18, -0.08, -0.15, -0.31, -0.82, -0.58},
    // 	{0.96, -0.42, 0.67, -0.02, -0.03, 0.81, -0.51, 1.01}
	// };

	double positionalScore = 0;
	for (int x = 0; x < 8; ++x) {
    	for (int y = 0; y < 8; ++y) {
        	if (board.get(x, y) == turn) {
            	positionalScore += positionalValues[x][y];
        	} else if (board.get(x, y) == other(turn)) {
            	positionalScore -= positionalValues[x][y];
        	}
    	}
	}

	int mobilityScore = mobilityHeuristic(board, turn);
	int cornerScore = cornerHeuristic(board, turn);
	return positionalScore + 2 * mobilityScore + 5 * cornerScore;
}


int MyBot::mobilityHeuristic(const OthelloBoard& board, Turn turn) {
	int myMoves = board.getValidMoves(turn).size();
	int opponentMoves = board.getValidMoves(other(turn)).size();
	return myMoves - opponentMoves;
}


int MyBot::cornerHeuristic(const OthelloBoard& board, Turn turn) {
	int myCorners = 0, opponentCorners = 0;
	const int corners[4][2] = {{0, 0}, {0, 7}, {7, 0}, {7, 7}};

	for (int i = 0; i < 4; ++i) {
    	int x = corners[i][0], y = corners[i][1];
    	if (board.get(x, y) == turn) {
        	++myCorners;
    	} else if (board.get(x, y) == other(turn)) {
        	++opponentCorners;
    	}
	}

	return myCorners - opponentCorners;
}


extern "C" {
	OthelloPlayer* createBot(Turn turn) {
    	return new MyBot(turn);
	}

	void destroyBot(OthelloPlayer* bot) {
    	delete bot;
	}
}