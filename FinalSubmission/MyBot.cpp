// SMAI Assignment 3
// Vandita Lodha and Suhani Jain

/*
* @file botTemplate.cpp
* @author Arun Tejasvi Chaganty <arunchaganty@gmail.com>
* @date 2010-02-04
* Template for users to create their own bots
*/

#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
using namespace std;
using namespace Desdemona;


// new stuff

#include <ctime>
#include <list>

#define INF 1e18

Turn my;

clock_t start,finish;
OthelloBoard globalBoard;


// Function to check if a move is valid in a specific direction
bool canMove(char self, char opp, char *str)
{
	if (str[0] != opp) return false; 

	for (int ctr = 1; ctr < 8; ctr++)
    {
		
		if (str[ctr] == 'e') return false; // If an empty cell ('e') is encountered, the move is invalid
		if (str[ctr] == self) return true; // If a piece of the current player ('self') is encountered, the move is valid
	}

	return false; // If neither condition is met, the move is invalid
}

// Function to check if placing a piece at (startx, starty) is a legal move
bool isLegalMove(char self, char opp, char grid[8][8], int startx, int starty)
{
	if (grid[startx][starty] != 'e') return false; // A move is only valid on an empty cell ('e')

	char str[10]; // Array to hold a sequence of cells in a direction
	int x, y, dx, dy, i;

	// Iterate through all possible directions (8 total: diagonals, vertical, horizontal)
	for (dy = -1; dy <= 1; dy++)      // dy represents the vertical direction (-1, 0, 1)
		for (dx = -1; dx <= 1; dx++)  // dx represents the horizontal direction (-1, 0, 1)
        {
			if (!dy && !dx) continue; // Skip the case where both dx and dy are 0 (no movement)

			str[0] = '\0'; // Reset the direction string for the new direction

			for (i = 1; i < 8; i++)
            {
				// Calculate the new coordinates based on the direction (dx, dy)
				x = startx + i * dx;
				y = starty + i * dy;

				if (x >= 0 && y >= 0 && x < 8 && y < 8) // If the new coordinates are within bounds, store the cell value in `str`
					str[i - 1] = grid[x][y];
				else // If out of bounds, mark the cell as invalid (0) and break 
					str[i - 1] = 0;
			}
			
			if (canMove(self, opp, str)) return true; // Check if the current direction forms a valid move
		}

	return false; // If no valid direction is found, the move is illegal
}

// Function to count the number of valid moves available for the current player
int numValidMoves(char self, char opp, char grid[8][8])
{
	int valid_move_count = 0;
	int i, j;

	for (i = 0; i < 8; i++)           
		for (j = 0; j < 8; j++)    
			if (isLegalMove(self, opp, grid, i, j)) 
				valid_move_count++; 

	return valid_move_count;
}

double othelloBoardEvaluator(char grid[8][8])
{
    // Define the colors for the player and opponent
    char me = 'm', opp = 'y';

    int myTiles = 0, oppTiles = 0, i, j, k, myFrontTiles = 0, oppFrontTiles = 0, x, y;
    
    double p = 0.0, c = 0.0, l = 0.0, m = 0.0, f = 0.0, d = 0.0; // Variables to store weighted components of the score

    // Directions for checking neighbors (frontier disks)
    int X1[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int Y1[] = {0, 1, 1, 1, 0, -1, -1, -1};

    // Predefined heuristic weight matrix for board evaluation
    int V[8][8] =
    {
        { 20, -3, 11, 8, 8, 11, -3, 20 },
        { -3, -7, -4, 1, 1, -4, -7, -3 },
        { 11, -4, 2, 2, 2, 2, -4, 11 },
        { 8, 1, 2, -3, -3, 2, 1, 8 },
        { 8, 1, 2, -3, -3, 2, 1, 8 },
        { 11, -4, 2, 2, 2, 2, -4, 11 },
        { -3, -7, -4, 1, 1, -4, -7, -3 },
        { 20, -3, 11, 8, 8, 11, -3, 20 }
    };

    // Calculate piece difference, frontier disks, and disk weights
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++)
        {
            if (grid[i][j] == me) // Count and evaluate tiles for the player
            {
                d += V[i][j]; // Add weight of tile from the heuristic matrix
                myTiles++;
            }
            else if (grid[i][j] == opp) // Count and evaluate tiles for the opponent
            {
                d -= V[i][j]; // Subtract weight of tile
                oppTiles++;
            }
            if (grid[i][j] != 'e') // Check for frontier tiles
            {
                for (k = 0; k < 8; k++)
                {
                    x = i + X1[k]; y = j + Y1[k];
                    if (x >= 0 && x < 8 && y >= 0 && y < 8 && grid[x][y] == 'e') // Check if neighbor is empty
                    {
                        if (grid[i][j] == me) myFrontTiles++;
                        else oppFrontTiles++;
                        break; // Exit loop as one frontier tile found
                    }
                }
            }
        }

    // Calculate the piece difference as a percentage
    if (myTiles > oppTiles) 
        p = (100.0 * myTiles) / (myTiles + oppTiles);
    else if (myTiles < oppTiles) 
        p = -(100.0 * oppTiles) / (myTiles + oppTiles);

    // Calculate the frontier tiles score
    if (myFrontTiles > oppFrontTiles) 
        f = -(100.0 * myFrontTiles) / (myFrontTiles + oppFrontTiles);
    else if (myFrontTiles < oppFrontTiles) 
        f = (100.0 * oppFrontTiles) / (myFrontTiles + oppFrontTiles);

    // Corner occupancy evaluation
    myTiles = oppTiles = 0; // Reset tile counts
    if (grid[0][0] == me) myTiles++;
    else if (grid[0][0] == opp) oppTiles++;
    if (grid[0][7] == me) myTiles++;
    else if (grid[0][7] == opp) oppTiles++;
    if (grid[7][0] == me) myTiles++;
    else if (grid[7][0] == opp) oppTiles++;
    if (grid[7][7] == me) myTiles++;
    else if (grid[7][7] == opp) oppTiles++;
    c = 25 * (myTiles - oppTiles); // Weight for corner tiles

    // Corner closeness evaluation
    myTiles = oppTiles = 0; // Reset counts
    if (grid[0][0] == 'e') // Top-left corner
    {
        if (grid[0][1] == me) myTiles++;
        else if (grid[0][1] == opp) oppTiles++;
        if (grid[1][1] == me) myTiles++;
        else if (grid[1][1] == opp) oppTiles++;
        if (grid[1][0] == me) myTiles++;
        else if (grid[1][0] == opp) oppTiles++;
    }
    if (grid[0][7] == 'e') // Top-right corner
    {
        if (grid[0][6] == me) myTiles++;
        else if (grid[0][6] == opp) oppTiles++;
        if (grid[1][6] == me) myTiles++;
        else if (grid[1][6] == opp) oppTiles++;
        if (grid[1][7] == me) myTiles++;
        else if (grid[1][7] == opp) oppTiles++;
    }
    if (grid[7][0] == 'e') // Bottom-left corner
    {
        if (grid[7][1] == me) myTiles++;
        else if (grid[7][1] == opp) oppTiles++;
        if (grid[6][1] == me) myTiles++;
        else if (grid[6][1] == opp) oppTiles++;
        if (grid[6][0] == me) myTiles++;
        else if (grid[6][0] == opp) oppTiles++;
    }
    if (grid[7][7] == 'e') // Bottom-right corner
    {
        if (grid[6][7] == me) myTiles++;
        else if (grid[6][7] == opp) oppTiles++;
        if (grid[6][6] == me) myTiles++;
        else if (grid[6][6] == opp) oppTiles++;
        if (grid[7][6] == me) myTiles++;
        else if (grid[7][6] == opp) oppTiles++;
    }
    l = -10 * (myTiles - oppTiles); // Weight for corner closeness

    // Mobility evaluation
    myTiles = numValidMoves(me, opp, grid); // My mobility
    oppTiles = numValidMoves(opp, me, grid); // Opponent mobility
    if (myTiles > oppTiles) 
        m = (100.0 * myTiles) / (myTiles + oppTiles);
    else if (myTiles < oppTiles) 
        m = -(100.0 * oppTiles) / (myTiles + oppTiles);

    // Final weighted score calculation
    double score = (11 * p) + (850.724 * c) + (382.026 * l) + (86.922 * m) + (78.396 * f) + (10 * d);
    return score; // Return the evaluation score
}

double testMyMove(OthelloBoard board, Move move, Turn turn, short level, double alpha, double beta)
{
    // Record the current time to check for timeout
    finish = clock();
    // If the computation exceeds 1.95 seconds, return a fallback value based on the level
    if(((double)(finish-start)/CLOCKS_PER_SEC) > 1.9)
    {
        if(level & 1) return -INF; // If it's the opponent's turn, return -INF
        return INF; // If it's my turn, return INF
    }
    
    // If maximum depth is reached, evaluate the board using the custom evaluator
    if(level == 6)
    {
        char grid[8][8];
        for(int i = 0; i < 8; i++)
        {
            for(int j = 0; j < 8; j++)
            {
                // Map board turns to grid symbols
                Coin findTurn = board.get(i, j);
                if(findTurn == turn) grid[i][j] = 'y';
                else if(findTurn == other(turn)) grid[i][j] = 'm';
                else grid[i][j] = 'e';
            }
        }
        return othelloBoardEvaluator(grid); // Evaluate the board and return the score
    }

    // Apply the move to the board and switch the turn to the opponent
    board.makeMove(turn, move);
    turn = other(turn);

    // Get the list of valid moves for the opponent
    list<Move> newMoves = board.getValidMoves(turn);
    list<Move>::iterator iter = newMoves.begin();

    // Initialize the return value to -INF (or INF for minimizing levels)
    double ret = -INF;
    if(level & 1) ret *= -1; // If it's a minimizing level, negate the initial value

    // If no moves are available, return the current evaluation
    if(!(newMoves.size())) return ret;

    // Iterate over all valid moves
    for(; iter != newMoves.end(); iter++)
    {
        // Recursively evaluate the next move
        double curr = testMyMove(board, *iter, turn, level + 1, alpha, beta);

        // Alpha-beta pruning logic
        if(level & 1) // Minimizing level
        {
            ret = min(ret, curr); // Update the best minimizing value
            beta = min(beta, ret); // Update beta value
        }
        else // Maximizing level
        {
            ret = max(ret, curr); // Update the best maximizing value
            alpha = max(alpha, ret); // Update alpha value
        }

        // Prune the search tree if beta is less than or equal to alpha
        if(beta <= alpha) break;
    }

    // Return the best value found at this level
    return ret; 
}

double tester(OthelloBoard board, Turn turn)
{
    char grid[8][8];
    for(int i = 0; i < 8; i++)
    {
        for(int j = 0; j < 8; j++)
        {
            Coin findTurn = board.get(i, j);
            if(findTurn == turn) grid[i][j] = 'm';
            else if(findTurn == other(turn)) grid[i][j] = 'y';
            else grid[i][j] = 'e';
        }
    }
    // Evaluate the board using the custom evaluator
    return othelloBoardEvaluator(grid);
}

bool compare(Move a, Move b)
{
    // Create two copies of the global board for comparison
    OthelloBoard One = globalBoard, Two = globalBoard;

    // Apply the moves 'a' and 'b' on the respective board copies
    One.makeMove(my, a);
    Two.makeMove(my, b);

    // Compare the evaluation scores of the two resulting boards
    return tester(One, my) > tester(Two, my);
}


// old stuff, modified

class MyBot: public OthelloPlayer
{
    public:
        /**
         * Initialisation routines here
         * This could do anything from open up a cache of "best moves" to
         * spawning a background processing thread. 
         */
        MyBot( Turn turn );

        /**
         * Play something 
         */
        virtual Move play( const OthelloBoard& board );
    private:
};

MyBot::MyBot( Turn turn )
    : OthelloPlayer( turn )
{
}

Move MyBot::play(const OthelloBoard& board)
{
    start = clock();
    list<Move> moves = board.getValidMoves(turn); // Get all valid moves for the current turn
    my = turn; // Set the global turn variable to the current player's turn
    globalBoard = board; // Store the current board state globally for evaluation    
    moves.sort(compare); // Sort the moves based on their evaluation scores
    list<Move>::iterator it = moves.begin(); // Initialize iterator to point to the first move in the list   
    Move bestMove((*it).x, (*it).y); // Set the best move to the first move in the sorted list as a starting point
    double retVal = -INF; // Initialize the best value as negative infinity
    double MAX = INF, MIN = -INF; // Define alpha-beta pruning bounds
    OthelloBoard copyBoard = board; // Create a copy of the board for testing moves
    short level = 1; // Start at level 1 of the minimax search

    // Iterate over all sorted moves
    for(; it != moves.end(); it++)
    {
        // Test the current move using minimax with alpha-beta pruning
        double currValue = testMyMove(copyBoard, *it, turn, level, MIN, MAX);

        // Update the best move and its value if the current value is better
        if(currValue > retVal)
        {
            retVal = currValue;
            bestMove = *it;
        }

        // Reset the board state for the next iteration
        copyBoard = board;
    }

    return bestMove;
}


// The following lines are _very_ important to create a bot module for Desdemona

extern "C" {
    OthelloPlayer* createBot( Turn turn )
    {
        return new MyBot( turn );
    }

    void destroyBot( OthelloPlayer* bot )
    {
        delete bot;
    }
}