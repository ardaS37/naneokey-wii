#include "game/types.hpp"

namespace nane
{
const char* SeatName(Seat seat)
{
    switch (seat)
    {
    case Seat::South:
        return "Sark";
    case Seat::West:
        return "Garp";
    case Seat::North:
        return "Simal";
    case Seat::East:
        return "Cenup";
    }

    return "?";
}

std::string TileLabel(const Tile& tile)
{
    const char* color = "K";
    switch (tile.color)
    {
    case TileColor::Red:
        color = "K";
        break;
    case TileColor::Blue:
        color = "M";
        break;
    case TileColor::Yellow:
        color = "Y";
        break;
    case TileColor::Black:
        color = "Y";
        break;
    }

    return std::to_string(tile.number) + color;
}

bool SeatLess(Seat left, Seat right)
{
    return static_cast<int>(left) < static_cast<int>(right);
}
}
