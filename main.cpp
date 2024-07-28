#include <iostream>

#include <board_state.h>
#include <game_mechanics.h>

// Board layout (pit indices for each player are specified within the `( )` markings)
//
// Player 1: [ ] (5) (4) (3) (2) (1) (0)
// Player 0:     (0) (1) (2) (3) (4) (5) [ ]
//
// Rules:
//
// Play starts with either player 0 or player 1. For their turn, the active player specifies a pit index
// corresponding to their side of the board (pits are denoted by `( )`). All of the stones are removed from
// that pit and dropped one by one in a counterclockwise fashion into the adjacent pits and banks
// (banks are denoted by `[ ]`). The opposing player's bank is skipped. If the final stone lands in the
// active player's bank, they continue play with another turn. If the final stone lands in an empty pit on
// the active player's side and the opposing pit contains stones, that stone and all stones in the opposing pit
// are removed and placed in the active player's bank.
// The game finishes when all pits on either player's side are empty, at which point any
// remaining stones in a player's pits are placed in that player's bank. Bank stones are then tallied
// for each player, and the player with the most stones wins.

// Outline:
// - BoardState, board / game representation, allows for board state to be manipulated
// - TurnExecutor, takes turn action specified as [player identifier, pit index]
//   to play as the turn and manipulates board state in compliance with game mechanics.
//   Should return a boolean or indication about whether play passes to the next player
//   based on whether the final stone of the turn lands in the bank.
//   Helpful functionality:
//     - Ability to know opposing pit index (i.e. pit 2 corresponds to the opposing player's pit 3) for
//       the clearing rule
//     - Knowledge of board size (to know when to wrap around when dropping stones)
// - GameMechanicsExecutor
//     - Determines which player is active
//     - Accepts turn input for the active player and passes it to the turn executor
//     - Deals with turn input which is invalid given the current game state (e.g. a player cannot play
//       a turn starting from an empty pit). May use the `TurnExecutor` to determine validity.
//     - Determines whether the game is over and handles cleanup of the board state
// 
// The above is sufficient to allow for gameplay to be executed. Sample program flow for manual turns:
// 
// BoardState board_state{ /*num_pits*/ 6, /*num_stones_per_pit*/ 4 };
// GameMechanicsExecutor game_mechanics_executor{ /*starting_player_index*/ 0};
//
// std::cout << board_state.print() << std::endl << std::endl;
// game_mechanics_executor.playTurn(/*player_index*/ 0, /*pit_index*/ 0);
//
// std::cout << board_state.print() << std::endl << std::endl;
// game_mechanics_executor.playTurn(/*player_index*/ 1, /*pit_index*/ 2);
// ...


BoardState makeTestBoardState()
{
    return BoardState{ /*player_0_board_state*/ SinglePlayerBoardState{ std::vector<int>{ 4, 4, 4, 4, 4, 4 }, 0 }, 
                       /*player_1_board_state*/ SinglePlayerBoardState{ std::vector<int>{ 4, 4, 4, 4, 4, 4 }, 0 }};
}

int main(int argc, char** argv)
{
    BoardState board_state{ makeTestBoardState() };
    const TurnExecutor turn_executor{};

    std::cout << board_state.printForPlayer0() << std::endl;

    for (std::size_t i = 0; i < 10; ++i)
    {
        const std::size_t player_index{ i % 2 };
        const std::size_t pit_index{ static_cast<std::size_t>(std::rand() % 7) };
        const TurnResult result{ turn_executor.playTurn(player_index, pit_index, board_state) };

        std::cout << "Turn - player_index: " << player_index << ", pit_index: " << pit_index << std::endl;
        std::cout << "Result:\n" << result << std::endl << std::endl;
        std::cout << board_state.printForPlayer0() << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
    }
    
    return 0;
}