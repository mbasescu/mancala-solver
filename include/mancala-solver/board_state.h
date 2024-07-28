#pragma once

#include <exception>
#include <sstream>
#include <string>
#include <vector>

class SinglePlayerBoardState
{
public:
    SinglePlayerBoardState(const std::vector<int> pits, const int bank) : pits_{ pits }, bank_{ bank }
    {
    }

    void addStoneToPit(const std::size_t pit_id)
    {
        pits_.at(pit_id)++;
    }

    void clearStonesFromPit(const std::size_t pit_id)
    {
        pits_.at(pit_id) = 0;
    }

    std::size_t getNumPits() const
    {
        return pits_.size();
    }

    int getNumStonesInPit(const std::size_t pit_id) const
    {
        return pits_.at(pit_id);
    }

    int sumOfStonesInPits() const
    {
        int sum{ 0 };
        for (const int num_stones : pits_)
        {
            sum += num_stones;
        }

        return sum;
    }

    void addStonesToBank(const int num_stones)
    {
        bank_ += num_stones;
    }

    int getNumStonesInBank() const
    {
        return bank_;
    }

    std::string print() const
    {
        std::stringstream ss{};
        for (const int stone_count : pits_)
        {
            ss << "(" << stone_count << ") ";
        }

        ss << "[" << bank_ << "]";

        return ss.str();
    }

    //! Convenient for printing an opposing player's board
    std::string printReversed() const
    {
        std::stringstream ss{};

        ss << "[" << bank_ << "]";

        for (auto it = pits_.crbegin(); it != pits_.crend(); ++it)
        {
            ss << " (" << *it << ")";
        }

        return ss.str();
    }

private:
    std::vector<int> pits_;
    int bank_;
};

class BoardState
{
public:
    //! Constructs a board in which each player has `num_pits` each filled with `num_stones_per_pit`,
    //! starting with zero stones in the bank.
    BoardState(const std::size_t num_pits, const int num_stones_per_pit) :
                player_0_board_state_{ std::vector<int>(num_pits, num_stones_per_pit), /*bank*/ 0 },
                player_1_board_state_{ std::vector<int>(num_pits, num_stones_per_pit), /*bank*/ 0 }
    {
    }

    BoardState(const SinglePlayerBoardState& player_0_board_state, const SinglePlayerBoardState& player_1_board_state) :
                player_0_board_state_{ player_0_board_state },
                player_1_board_state_{ player_1_board_state }
    {
        if (player_0_board_state_.getNumPits() != player_1_board_state_.getNumPits())
        {
            std::stringstream msg{};
            msg << "Number of pits must be the same for `player_0_board_state` (" << player_0_board_state_.getNumPits()
                << ") and `player_1_board_state` (" << player_1_board_state_.getNumPits() << ")";

            throw std::invalid_argument(msg.str());
        }
    }

    std::size_t getNumPits() const
    {
        //! The number of pits is enforced on construction to be the same for both players
        return player_0_board_state_.getNumPits();
    }

    SinglePlayerBoardState& getPlayer0BoardState()
    {
        return player_0_board_state_;
    }

    SinglePlayerBoardState& getPlayer1BoardState()
    {
        return player_1_board_state_;
    }

    std::string print() const
    {
        std::stringstream ss{};
        ss << player_1_board_state_.printReversed() << std::endl;
        ss << "    " << player_0_board_state_.print() << std::endl;
    
        return ss.str();
    }

private:
    SinglePlayerBoardState player_0_board_state_;
    SinglePlayerBoardState player_1_board_state_;
};
