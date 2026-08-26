#include "game/rules.hpp"

#include <algorithm>
#include <set>

namespace nane
{
bool RuleValidator::IsValidMeld(const Meld& meld) const
{
    return meld.tiles.size() >= 3 && (IsValidSet(meld.tiles) || IsValidRun(meld.tiles));
}

bool RuleValidator::IsValidSet(const std::vector<Tile>& tiles) const
{
    if (tiles.size() < 3 || tiles.size() > 4)
    {
        return false;
    }

    const int number = tiles.front().number;
    std::set<int> colors;
    for (const Tile& tile : tiles)
    {
        if (tile.number != number)
        {
            return false;
        }

        colors.insert(static_cast<int>(tile.color));
    }

    return colors.size() == tiles.size();
}

bool RuleValidator::IsValidRun(const std::vector<Tile>& tiles) const
{
    std::vector<Tile> ordered;
    if (TryOrderRun(tiles, false, ordered))
    {
        return true;
    }

    return TryOrderRun(tiles, true, ordered);
}

bool RuleValidator::AreAllMeldsValid(const std::vector<Meld>& melds) const
{
    for (const Meld& meld : melds)
    {
        if (!IsValidMeld(meld))
        {
            return false;
        }
    }

    return true;
}

Meld RuleValidator::NormalizeMeld(const Meld& meld) const
{
    Meld normalized = meld;
    std::vector<Tile> ordered;
    if (TryOrderRun(meld.tiles, false, ordered) || TryOrderRun(meld.tiles, true, ordered))
    {
        normalized.tiles = ordered;
        return normalized;
    }

    std::sort(
        normalized.tiles.begin(),
        normalized.tiles.end(),
        [](const Tile& left, const Tile& right)
        {
            if (left.color != right.color)
            {
                return static_cast<int>(left.color) < static_cast<int>(right.color);
            }

            if (left.number != right.number)
            {
                return left.number < right.number;
            }

            return left.id < right.id;
        });
    return normalized;
}

bool RuleValidator::TryOrderRun(const std::vector<Tile>& tiles, bool aceHighOnly, std::vector<Tile>& ordered) const
{
    if (tiles.size() < 3)
    {
        return false;
    }

    const TileColor color = tiles.front().color;
    int aceCount = 0;
    for (const Tile& tile : tiles)
    {
        if (tile.color != color)
        {
            return false;
        }

        if (tile.number == 1)
        {
            ++aceCount;
        }
    }

    if (aceCount > 1)
    {
        return false;
    }

    ordered = tiles;
    std::sort(
        ordered.begin(),
        ordered.end(),
        [aceHighOnly](const Tile& left, const Tile& right)
        {
            const int leftKey = SortNumber(left, aceHighOnly);
            const int rightKey = SortNumber(right, aceHighOnly);
            if (leftKey != rightKey)
            {
                return leftKey < rightKey;
            }

            return left.id < right.id;
        });

    if (!aceHighOnly)
    {
        for (const Tile& tile : ordered)
        {
            if (tile.number == 1)
            {
                return false;
            }
        }
    }

    for (std::size_t i = 1; i < ordered.size(); ++i)
    {
        const int previous = SortNumber(ordered[i - 1], aceHighOnly);
        const int current = SortNumber(ordered[i], aceHighOnly);
        if (current != previous + 1)
        {
            return false;
        }
    }

    if (aceHighOnly)
    {
        const bool endsWithAce = ordered.back().number == 1;
        const int firstNumber = ordered.front().number;
        return endsWithAce && firstNumber >= 11;
    }

    return true;
}

int RuleValidator::SortNumber(const Tile& tile, bool aceHigh)
{
    return aceHigh && tile.number == 1 ? 14 : tile.number;
}
}
