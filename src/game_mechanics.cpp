#include <game_mechanics.h>

std::ostream& operator<<(std::ostream& os, const TurnResult& turn_result)
{
    os << "valid: " << turn_result.valid << "\n";
    os << "ended_in_bank: " << turn_result.ended_in_bank;

    return os;
}
