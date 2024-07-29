#pragma once

#include <cstddef>
#include <optional>
#include <ostream>

#include <board_state.h>

struct TurnResult
{
    bool valid{ true };
    bool ended_in_bank { true };

    static TurnResult makeInvalidResult()
    {
        TurnResult result{};
        result.valid = false;
        result.ended_in_bank = false;

        return result;
    }

    static TurnResult makeNotEndedInBankResult()
    {
        TurnResult result{};
        result.valid = true;
        result.ended_in_bank = false;

        return result;
    }

    static TurnResult makeEndedInBankResult()
    {
        TurnResult result{};
        result.valid = true;
        result.ended_in_bank = true;

        return result;
    }
};

std::ostream& operator<<(std::ostream& os, const TurnResult& turn_result);

class TurnExecutor
{
public:
    // Attempts to execute a turn, returning true if the turn was valid and executed. Returns false if
    // the turn was invalid.
    TurnResult playTurn(const std::size_t player_index, const std::size_t pit_index, BoardState& board_state) const
    {
        // There is only support for two players with indices `0` and `1`
        if ((player_index != 0) && (player_index != 1))
        {
            return TurnResult::makeInvalidResult();
        }

        if (pit_index >= board_state.getNumPits())
        {
            return TurnResult::makeInvalidResult();
        }

        SinglePlayerBoardState& active_player_board_state{ (player_index == 0) ? board_state.getPlayer0BoardState()
                                                                               : board_state.getPlayer1BoardState()};
        SinglePlayerBoardState& opposing_player_board_state{ (player_index == 0) ? board_state.getPlayer1BoardState()
                                                                               : board_state.getPlayer0BoardState()};

        int num_stones{ active_player_board_state.getNumStonesInPit(pit_index) };
        active_player_board_state.clearStonesFromPit(pit_index);
        if (num_stones <= 0)
        {
            return TurnResult::makeInvalidResult();
        }

        // Special case for the first turn since the starting pit index is nonzero
        const std::optional<TurnResult> result{ dropStonesOnActivePlayerBoard(/*starting_pit_index*/ pit_index + 1,
                                                    active_player_board_state, opposing_player_board_state, num_stones) };
        if (result.has_value())
        {
            return result.value();
        }

        while (num_stones > 0)
        {
            {
                const std::optional<TurnResult> result{
                    dropStonesOnOpposingPlayerBoard(opposing_player_board_state, num_stones) };
                if (result.has_value())
                {
                    return result.value();
                }
            }

            {
                const std::optional<TurnResult> result{ dropStonesOnActivePlayerBoard(/*starting_pit_index*/ 0,
                                                            active_player_board_state, opposing_player_board_state, num_stones) };
                if (result.has_value())
                {
                    return result.value();
                }
            }
        }

        throw std::logic_error("Error: exited subroutine with zero stones without returning successfully.");
    }

private:
    std::optional<TurnResult> dropStonesOnActivePlayerBoard(const std::size_t starting_pit_index,
                SinglePlayerBoardState& active_player_board_state, SinglePlayerBoardState& opposing_player_board_state, int& num_stones) const
    {
        std::size_t current_pit_index{ starting_pit_index };
        while (num_stones > 0)
        {
            if (current_pit_index == active_player_board_state.getNumPits())
            {
                active_player_board_state.addStonesToBank(/*num_stones*/ 1);
                --num_stones;

                if (num_stones == 0)
                {
                    return TurnResult::makeEndedInBankResult();
                }

                // Break out of the loop since we need to switch to the other player's pits
                break;
            }

            active_player_board_state.addStoneToPit(current_pit_index);
            ++current_pit_index;
            --num_stones;
        }

        // Check if we ended in an empty pit (would now contain one stone) on the active player's board
        if (num_stones == 0)
        {
            const std::size_t final_pit_index{ current_pit_index - 1 };
            const std::size_t opposing_pit_index{ opposing_player_board_state.getNumPits() - final_pit_index - 1 };
            const int num_stones_in_opposing_pit{ opposing_player_board_state.getNumStonesInPit(opposing_pit_index) };

            // Need to check against the previous pit because `current_pit_index` was incremented
            if ((active_player_board_state.getNumStonesInPit(final_pit_index) == 1) &&
                (num_stones_in_opposing_pit > 0))
            {
                active_player_board_state.addStonesToBank(1 + num_stones_in_opposing_pit);
                active_player_board_state.clearStonesFromPit(final_pit_index);
                opposing_player_board_state.clearStonesFromPit(opposing_pit_index);
            }
            
            return TurnResult::makeNotEndedInBankResult();
        }

        return std::nullopt;
    }

    std::optional<TurnResult> dropStonesOnOpposingPlayerBoard(SinglePlayerBoardState& opposing_player_board_state, int& num_stones) const
    {
        std::size_t current_pit_index{ 0 };
        while (num_stones > 0)
        {
            if (current_pit_index == opposing_player_board_state.getNumPits())
            {
                // Break out of the loop since we need to switch to the other player's pits
                break;
            }

            opposing_player_board_state.addStoneToPit(current_pit_index);
            ++current_pit_index;
            --num_stones;
        }

        if (num_stones == 0)
        {
            return TurnResult::makeNotEndedInBankResult();
        }

        return std::nullopt;
    }
};

//! Runs the core game mechanics, storing any persistent state about which player is active.
//! Assumes that the turn input corresponds to the active player.
class GameMechanicsExecutor
{
public:
    GameMechanicsExecutor(const TurnExecutor& turn_executor, const std::size_t starting_player_index) :
                    turn_executor_{ turn_executor }, active_player_index_{ starting_player_index }
    {
        if ((active_player_index_ != 0) && (active_player_index_ != 1))
        {
            throw std::invalid_argument("`GameMechanicsExecutor()`: `active_player_index` must be 0 or 1");
        }
    }

    //! Executes a turn for the active player, adjusting the `board_state` accordingly.
    //! Returns true for a valid turn, false for an invalid turn (i.e. `pit_index` was empty).
    bool playTurn(const std::size_t pit_index, BoardState& board_state)
    {
        const TurnResult result{ turn_executor_.playTurn(active_player_index_, pit_index, board_state) };
        if (!result.valid)
        {
            return false;
        }

        if (!result.ended_in_bank)
        {
            active_player_index_ = (active_player_index_ + 1) % 2;
        }

        return true;
    }

    std::size_t getActivePlayerIndex() const
    {
        return active_player_index_;
    }

    bool isGameFinished(const BoardState& board_state) const
    {
        const SinglePlayerBoardState& player_0_board_state{ board_state.getPlayer0BoardState() };
        if (isSinglePlayerBoardFinished(player_0_board_state))
        {
            return true;
        }

        const SinglePlayerBoardState& player_1_board_state{ board_state.getPlayer1BoardState() };
        if (isSinglePlayerBoardFinished(player_1_board_state))
        {
            return true;
        }

        return false;
    }

    //! Only returns with a value if the game is finished. Automatically cleans up `board_state` so
    //! that all stones end up in the banks. Returns `2` if the game ended in a tie.
    std::optional<std::size_t> getWinnerPlayerIndex(BoardState& board_state)
    {
        if (!isGameFinished(board_state))
        {
            return std::nullopt;
        }

        SinglePlayerBoardState& player_0_board_state{ board_state.getPlayer0BoardState() };
        player_0_board_state.addStonesToBank(player_0_board_state.sumOfStonesInPits());
        for (std::size_t i = 0; i < player_0_board_state.getNumPits(); ++i)
        {
            player_0_board_state.clearStonesFromPit(i);
        }

        SinglePlayerBoardState& player_1_board_state{ board_state.getPlayer1BoardState() };
        player_1_board_state.addStonesToBank(player_1_board_state.sumOfStonesInPits());
        for (std::size_t i = 0; i < player_1_board_state.getNumPits(); ++i)
        {
            player_1_board_state.clearStonesFromPit(i);
        }

        if (player_0_board_state.getNumStonesInBank() > player_1_board_state.getNumStonesInBank())
        {
            return 0;
        }
        else if (player_1_board_state.getNumStonesInBank() > player_0_board_state.getNumStonesInBank())
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }

private:
    bool isSinglePlayerBoardFinished(const SinglePlayerBoardState& single_player_board_state) const
    {
        bool pits_are_clear{ true };
        for (std::size_t i = 0; i < single_player_board_state.getNumPits(); ++i)
        {
            if(single_player_board_state.getNumStonesInPit(i) != 0)
            {
                pits_are_clear = false;
                break;
            }
        }

        return pits_are_clear;
    }

    TurnExecutor turn_executor_;
    std::size_t active_player_index_;
};