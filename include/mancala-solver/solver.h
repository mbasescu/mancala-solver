#pragma once

#include <iostream>

#include <board_state.h>
#include <game_mechanics.h>

class Solver
{
public:
    //! Solves for the optimal pit index to choose for the current player. If the move guarantees a win even with
    //! perfect play by the opposing player, the second return value will be `true`. Otherwise, the move with the
    //! highest percentage of [winning + drawn] sub-branches is chosen, and the second return value will be `false`.
    //! Note that this solver does not currently provide the fastest sequence of moves to result in a win.
    std::pair<std::size_t, bool> solve(const BoardState& board_state, const GameMechanicsExecutor& game_mechanics_executor)
    {
        num_winning_branches_ = std::vector<std::size_t>(board_state.getNumPits(), 0);
        num_drawn_branches_ = std::vector<std::size_t>(board_state.getNumPits(), 0);
        num_total_branches_ = std::vector<std::size_t>(board_state.getNumPits(), 0);

        const std::size_t initial_active_player_index{ game_mechanics_executor.getActivePlayerIndex() };

        for (std::size_t i = 0; i < board_state.getNumPits(); ++i)
        {
            GameMechanicsExecutor game_mechanics_executor_i{ game_mechanics_executor };
            BoardState board_state_i{ board_state };

            const bool valid_turn{ game_mechanics_executor_i.playTurn(i, board_state_i) };
            if (!valid_turn)
            {
                continue;
            }
            const std::optional<std::size_t> winner_player_index{ game_mechanics_executor_i.getWinnerPlayerIndex(board_state_i) };
            if (winner_player_index.has_value())
            {
                if (winner_player_index.value() == initial_active_player_index)
                {
                    return std::make_pair(i, true);
                }

                continue;
            }

            if (solveInner(board_state_i, game_mechanics_executor_i, initial_active_player_index, /*initial_pit_index*/ i))
            {
                std::size_t num_total_branches{ 0 };
                for (const std::size_t num_branches : num_total_branches_)
                {
                    num_total_branches += num_branches;
                }
                
                std::cout << "Total num branches: " << num_total_branches << std::endl;
                return std::make_pair(i, true);
            }
        }

        double highest_win_and_draw_ratio{ 0.0 };
        std::size_t index_of_highest_win_and_draw_ratio{ 0 };

        for (std::size_t i = 0; i < board_state.getNumPits(); ++i)
        {
            const double win_and_draw_ratio{ static_cast<double>(num_winning_branches_.at(i)) / static_cast<double>(num_total_branches_.at(i)) };

            if (win_and_draw_ratio > highest_win_and_draw_ratio)
            {
                highest_win_and_draw_ratio = win_and_draw_ratio;
                index_of_highest_win_and_draw_ratio = i;
            }
        }

        return std::make_pair(index_of_highest_win_and_draw_ratio, false);
    }

    //! If the currently active player is the initially active player, returns true if there was a guaranteed win in any sub-branch. 
    //! If the currently active player is opposing the initially active player, return true if all sub-branches contain a guaranteed win
    //! for the initially active player.
    bool solveInner(const BoardState& board_state, const GameMechanicsExecutor& game_mechanics_executor, const std::size_t initial_active_player_index,
            const std::size_t initial_pit_index)
    {
        const std::size_t active_player_index{ game_mechanics_executor.getActivePlayerIndex() };
        const std::size_t initial_opposing_player_index{ (initial_active_player_index + 1) % 2 };

        for (std::size_t i = 0; i < board_state.getNumPits(); ++i)
        {
            BoardState board_state_i{ board_state };
            GameMechanicsExecutor game_mechanics_executor_i{ game_mechanics_executor };

            const bool turn_valid{ game_mechanics_executor_i.playTurn(i, board_state_i) };
            if (!turn_valid)
            {
                continue;
            }

            const std::optional<std::size_t> winner_player_index{ game_mechanics_executor_i.getWinnerPlayerIndex(board_state_i) };
            if (winner_player_index.has_value())
            {
                if (winner_player_index.value() == initial_active_player_index)
                {
                    ++num_winning_branches_.at(initial_pit_index);
                    ++num_total_branches_.at(initial_pit_index);

                    if (active_player_index == initial_active_player_index)
                    {
                        return true;
                    }
                }
                else if (winner_player_index.value() == initial_opposing_player_index)
                {
                    if (active_player_index == initial_opposing_player_index)
                    {
                        return false;
                    }
                }
                // Drawn case
                else
                {
                    ++num_drawn_branches_.at(initial_pit_index);
                }

                ++num_total_branches_.at(initial_pit_index);

                continue;
            }
                
            const bool guaranteed_win_for_initially_active_player{
                solveInner(board_state_i, game_mechanics_executor_i, initial_active_player_index, initial_pit_index)
            };
            // This case represents where a guaranteed win is found and the current move is up to the initially active player
            if (guaranteed_win_for_initially_active_player && (active_player_index == initial_active_player_index))
            {
                return true;
            }
            
            // This case represents where the non-initially-active (opposing) player has a move which prevents a guaranteed win
            if (!guaranteed_win_for_initially_active_player && (active_player_index != initial_active_player_index))
            {
                // Returning early here will prematurely eliminate some branches that would generate additional data in the case
                // where there is no guaranteed win given the `initial_pit_index`. This could be improved if necessary.
                return false;
            }
        }

        // This covers the case where every move is an immediate win for the `initial_active_player_index`
        if (active_player_index == initial_opposing_player_index)
        {
            return true;
        }

        // This covers the case where there are no immediate wins and (`active_player_index == initial_active_player_index`)
        return false;
    }

private:
    //! Size corresponds to the total number of pits, as the number of winning / total branches is tracked independently for each move choice
    std::vector<std::size_t> num_winning_branches_;
    std::vector<std::size_t> num_drawn_branches_;
    std::vector<std::size_t> num_total_branches_;
};