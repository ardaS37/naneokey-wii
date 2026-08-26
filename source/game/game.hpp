#pragma once

#include "game/rules.hpp"

#include <random>
#include <string>
#include <vector>

namespace nane
{
class BotEngine;

class OfflineGame
{
public:
    OfflineGame();
    ~OfflineGame();

    void StartNewGame();
    void UpdateBots();

    const GameState& State() const;
    const RuleValidator& Validator() const;

    bool IsHumanTurn() const;
    std::size_t HandCursor() const;
    std::size_t TableMeldCursor() const;
    std::size_t TableTileCursor() const;
    void SetHandCursor(std::size_t index);
    void SetTableMeldCursor(std::size_t index);
    void SetSingleSelectedHand(std::size_t index);

    void MoveHandCursor(int delta);
    void MoveMeldCursor(int delta);
    void MoveMeldTileCursor(int delta);
    void ToggleHandSelection();
    void ClearSelection();

    bool CreateMeldFromSelection();
    bool AddSelectedTileToFocusedMeld();
    bool TakeFocusedTileFromTable();
    bool CommitHumanTurn();
    bool DrawOrPass();

    const std::vector<int>& SelectedHandIndices() const;
    std::string BuildDebugStatus() const;

    bool BeginTurn(Seat seat);
    bool CreateMeldFromHand(Seat seat, const std::vector<int>& tileIds);
    bool AddTileToMeld(Seat seat, int tileId, std::size_t meldIndex);
    bool RemoveTileFromMeld(Seat seat, std::size_t meldIndex, std::size_t tileIndex);
    bool CommitTurn(Seat seat, std::string& error);
    bool DrawTile(Seat seat, std::string& message);
    bool PassTurn(Seat seat, std::string& message);
    void UndoTurn(Seat seat);

private:
    GameState state_;
    RuleValidator validator_;
    BotEngine* bot_;
    std::mt19937 rng_;
    std::vector<int> selectedHandIndices_;
    std::size_t handCursor_ = 0;
    std::size_t tableMeldCursor_ = 0;
    std::size_t tableTileCursor_ = 0;

    void StartTurnIfNeeded(Seat seat);
    void AdvanceTurn();
    PlayerState& PlayerForSeat(Seat seat);
    const PlayerState& PlayerForSeat(Seat seat) const;
    std::vector<int> GatherAllTileIds(const std::vector<Meld>& table, const std::vector<Tile>& hand) const;
    bool HasSameTilesBeforeAndAfter() const;
    bool IsPureOpeningMeld(const Meld& meld) const;
    bool HasAnyPlayerOpenedBefore(Seat seat) const;
    Tile DrawFromDeck();
    std::vector<Tile> CreateShuffledDeck();
    void ClampCursors();
    void SetStatus(const std::string& text);
};
}
