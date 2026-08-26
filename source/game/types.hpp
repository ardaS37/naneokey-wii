#pragma once

#include <string>
#include <vector>

namespace nane
{
enum class TileColor
{
    Red = 0,
    Blue = 1,
    Yellow = 2,
    Black = 3
};

enum class Seat
{
    South = 0,
    West = 1,
    North = 2,
    East = 3
};

struct Tile
{
    int id = 0;
    TileColor color = TileColor::Red;
    int number = 1;
};

struct Meld
{
    std::vector<Tile> tiles;
};

struct PlayerState
{
    Seat seat = Seat::South;
    std::string name;
    bool isBot = false;
    bool hasOpened = false;
    std::vector<Tile> hand;
};

struct TurnState
{
    bool active = false;
    std::vector<Meld> table;
    std::vector<Tile> hand;
    std::vector<int> originalHandIds;
    std::vector<int> originalTableIds;
};

struct GameState
{
    std::vector<PlayerState> players;
    std::vector<Meld> table;
    std::vector<Tile> deck;
    Seat currentTurn = Seat::South;
    bool isGameOver = false;
    std::string winnerName;
    std::string status;
    TurnState turn;
};

const char* SeatName(Seat seat);
std::string TileLabel(const Tile& tile);
bool SeatLess(Seat left, Seat right);
}

