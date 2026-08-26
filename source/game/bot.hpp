#pragma once

#include "game/rules.hpp"

namespace nane
{
class OfflineGame;

class BotEngine
{
public:
    explicit BotEngine(const RuleValidator& validator);

    bool PlayTurn(OfflineGame& game, Seat seat);

private:
    const RuleValidator& validator_;

    std::vector<Meld> GenerateCandidateMelds(const std::vector<Tile>& tiles) const;
    std::vector<Meld> FindOpening(const std::vector<Tile>& hand, int minimumMelds) const;
};
}

