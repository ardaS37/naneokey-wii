#include "game/bot.hpp"
#include "game/game.hpp"

#include <algorithm>
#include <functional>
#include <set>

namespace nane
{
BotEngine::BotEngine(const RuleValidator& validator)
    : validator_(validator)
{
}

bool BotEngine::PlayTurn(OfflineGame& game, Seat seat)
{
    game.BeginTurn(seat);

    const PlayerState& player = game.State().players[static_cast<int>(seat)];
    const std::vector<Tile>& hand = game.State().turn.hand;
    const int minimumOpeningMelds = game.State().table.empty() ? 2 : 1;

    if (!player.hasOpened)
    {
        const std::vector<Meld> opening = FindOpening(hand, minimumOpeningMelds);
        if (static_cast<int>(opening.size()) >= minimumOpeningMelds)
        {
            for (const Meld& meld : opening)
            {
                std::vector<int> tileIds;
                for (const Tile& tile : meld.tiles)
                {
                    tileIds.push_back(tile.id);
                }

                game.CreateMeldFromHand(seat, tileIds);
            }

            std::string error;
            return game.CommitTurn(seat, error);
        }
    }

    for (std::size_t meldIndex = 0; meldIndex < game.State().turn.table.size(); ++meldIndex)
    {
        const std::vector<Tile> turnHand = game.State().turn.hand;
        for (const Tile& tile : turnHand)
        {
            if (game.AddTileToMeld(seat, tile.id, meldIndex))
            {
                std::string error;
                if (game.CommitTurn(seat, error))
                {
                    return true;
                }

                game.UndoTurn(seat);
                game.BeginTurn(seat);
            }
        }
    }

    const std::vector<Meld> candidates = GenerateCandidateMelds(game.State().turn.hand);
    for (const Meld& meld : candidates)
    {
        std::vector<int> tileIds;
        for (const Tile& tile : meld.tiles)
        {
            tileIds.push_back(tile.id);
        }

        if (!game.CreateMeldFromHand(seat, tileIds))
        {
            continue;
        }

        std::string error;
        if (game.CommitTurn(seat, error))
        {
            return true;
        }

        game.UndoTurn(seat);
        game.BeginTurn(seat);
    }

    std::string message;
    if (!game.DrawTile(seat, message))
    {
        return game.PassTurn(seat, message);
    }

    return true;
}

std::vector<Meld> BotEngine::GenerateCandidateMelds(const std::vector<Tile>& tiles) const
{
    std::vector<Meld> result;
    for (int number = 1; number <= 13; ++number)
    {
        std::vector<Tile> bucket;
        std::set<int> colors;
        for (const Tile& tile : tiles)
        {
            if (tile.number == number && colors.insert(static_cast<int>(tile.color)).second)
            {
                bucket.push_back(tile);
            }
        }

        if (bucket.size() >= 3)
        {
            result.push_back(Meld{std::vector<Tile>{bucket.begin(), bucket.begin() + 3}});
            if (bucket.size() == 4)
            {
                result.push_back(Meld{bucket});
            }
        }
    }

    for (int colorIndex = 0; colorIndex < 4; ++colorIndex)
    {
        std::vector<Tile> colorTiles;
        for (const Tile& tile : tiles)
        {
            if (static_cast<int>(tile.color) == colorIndex)
            {
                colorTiles.push_back(tile);
            }
        }

        std::sort(
            colorTiles.begin(),
            colorTiles.end(),
            [](const Tile& left, const Tile& right)
            {
                const int leftNumber = left.number == 1 ? 14 : left.number;
                const int rightNumber = right.number == 1 ? 14 : right.number;
                if (leftNumber != rightNumber)
                {
                    return leftNumber < rightNumber;
                }

                return left.id < right.id;
            });

        for (std::size_t start = 0; start < colorTiles.size(); ++start)
        {
            std::vector<Tile> run;
            run.push_back(colorTiles[start]);
            for (std::size_t index = start + 1; index < colorTiles.size(); ++index)
            {
                const int previous = run.back().number == 1 ? 14 : run.back().number;
                const int current = colorTiles[index].number == 1 ? 14 : colorTiles[index].number;
                if (current == previous)
                {
                    continue;
                }

                if (current != previous + 1)
                {
                    break;
                }

                run.push_back(colorTiles[index]);
                if (run.size() >= 3)
                {
                    Meld meld{run};
                    if (validator_.IsValidMeld(meld))
                    {
                        result.push_back(meld);
                    }
                }
            }
        }
    }

    std::vector<Meld> unique;
    std::set<std::string> seen;
    for (const Meld& meld : result)
    {
        const Meld normalized = validator_.NormalizeMeld(meld);
        std::string key;
        for (const Tile& tile : normalized.tiles)
        {
            key += std::to_string(tile.id);
            key.push_back(',');
        }

        if (seen.insert(key).second)
        {
            unique.push_back(normalized);
        }
    }

    return unique;
}

std::vector<Meld> BotEngine::FindOpening(const std::vector<Tile>& hand, int minimumMelds) const
{
    const std::vector<Meld> candidates = GenerateCandidateMelds(hand);
    std::vector<Meld> chosen;
    std::set<int> used;

    std::function<bool(std::size_t)> search = [&](std::size_t index)
    {
        if (static_cast<int>(chosen.size()) >= minimumMelds)
        {
            return true;
        }

        for (std::size_t i = index; i < candidates.size(); ++i)
        {
            bool collides = false;
            for (const Tile& tile : candidates[i].tiles)
            {
                if (used.count(tile.id) != 0)
                {
                    collides = true;
                    break;
                }
            }

            if (collides)
            {
                continue;
            }

            for (const Tile& tile : candidates[i].tiles)
            {
                used.insert(tile.id);
            }

            chosen.push_back(candidates[i]);
            if (search(i + 1))
            {
                return true;
            }

            chosen.pop_back();
            for (const Tile& tile : candidates[i].tiles)
            {
                used.erase(tile.id);
            }
        }

        return false;
    };

    if (search(0))
    {
        return chosen;
    }

    return {};
}
}
