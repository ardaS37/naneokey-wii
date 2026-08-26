#pragma once

#include "game/types.hpp"

#include <vector>

namespace nane
{
class RuleValidator
{
public:
    bool IsValidMeld(const Meld& meld) const;
    bool IsValidSet(const std::vector<Tile>& tiles) const;
    bool IsValidRun(const std::vector<Tile>& tiles) const;
    bool AreAllMeldsValid(const std::vector<Meld>& melds) const;
    Meld NormalizeMeld(const Meld& meld) const;

private:
    bool TryOrderRun(const std::vector<Tile>& tiles, bool aceHighOnly, std::vector<Tile>& ordered) const;
    static int SortNumber(const Tile& tile, bool aceHigh);
};
}

