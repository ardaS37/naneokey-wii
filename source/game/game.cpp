#include "game/game.hpp"
#include "game/bot.hpp"

#include <algorithm>
#include <sstream>

namespace nane
{
namespace
{
std::vector<Seat> SeatOrder()
{
    return {Seat::South, Seat::West, Seat::North, Seat::East};
}
}

OfflineGame::OfflineGame()
    : bot_(new BotEngine(validator_))
    , rng_(static_cast<std::mt19937::result_type>(std::random_device{}()))
{
    StartNewGame();
}

OfflineGame::~OfflineGame()
{
    delete bot_;
}

void OfflineGame::StartNewGame()
{
    state_ = GameState{};
    selectedHandIndices_.clear();
    handCursor_ = 0;
    tableMeldCursor_ = 0;
    tableTileCursor_ = 0;

    state_.players = {
        PlayerState{Seat::South, "Nurhan", false, false, {}},
        PlayerState{Seat::West, "Sarp", true, false, {}},
        PlayerState{Seat::North, "Sukru", true, false, {}},
        PlayerState{Seat::East, "Kemal", true, false, {}}
    };

    state_.deck = CreateShuffledDeck();
    for (PlayerState& player : state_.players)
    {
        for (int i = 0; i < 15; ++i)
        {
            player.hand.push_back(DrawFromDeck());
        }
    }

    state_.currentTurn = Seat::South;
    SetStatus("Yeni Wii oyunu basladi. Sira Nurhan oyuncusunda.");
    StartTurnIfNeeded(Seat::South);
}

void OfflineGame::UpdateBots()
{
    while (!state_.isGameOver && !IsHumanTurn())
    {
        bot_->PlayTurn(*this, state_.currentTurn);
    }

    if (!state_.isGameOver && IsHumanTurn())
    {
        StartTurnIfNeeded(Seat::South);
    }
}

const GameState& OfflineGame::State() const
{
    return state_;
}

const RuleValidator& OfflineGame::Validator() const
{
    return validator_;
}

bool OfflineGame::IsHumanTurn() const
{
    return !state_.isGameOver && state_.currentTurn == Seat::South;
}

std::size_t OfflineGame::HandCursor() const
{
    return handCursor_;
}

std::size_t OfflineGame::TableMeldCursor() const
{
    return tableMeldCursor_;
}

std::size_t OfflineGame::TableTileCursor() const
{
    return tableTileCursor_;
}

void OfflineGame::SetHandCursor(std::size_t index)
{
    StartTurnIfNeeded(Seat::South);
    handCursor_ = index;
    ClampCursors();
}

void OfflineGame::SetTableMeldCursor(std::size_t index)
{
    StartTurnIfNeeded(Seat::South);
    tableMeldCursor_ = index;
    ClampCursors();
}

void OfflineGame::SetSingleSelectedHand(std::size_t index)
{
    StartTurnIfNeeded(Seat::South);
    selectedHandIndices_.clear();
    if (index < state_.turn.hand.size())
    {
        selectedHandIndices_.push_back(static_cast<int>(index));
        handCursor_ = index;
    }
    ClampCursors();
}

void OfflineGame::MoveHandCursor(int delta)
{
    StartTurnIfNeeded(Seat::South);
    if (state_.turn.hand.empty())
    {
        handCursor_ = 0;
        return;
    }

    const int count = static_cast<int>(state_.turn.hand.size());
    int next = static_cast<int>(handCursor_) + delta;
    if (next < 0)
    {
        next = 0;
    }
    if (next >= count)
    {
        next = count - 1;
    }

    handCursor_ = static_cast<std::size_t>(next);
}

void OfflineGame::MoveMeldCursor(int delta)
{
    StartTurnIfNeeded(Seat::South);
    if (state_.turn.table.empty())
    {
        tableMeldCursor_ = 0;
        tableTileCursor_ = 0;
        return;
    }

    const int count = static_cast<int>(state_.turn.table.size());
    int next = static_cast<int>(tableMeldCursor_) + delta;
    if (next < 0)
    {
        next = 0;
    }
    if (next >= count)
    {
        next = count - 1;
    }

    tableMeldCursor_ = static_cast<std::size_t>(next);
    ClampCursors();
}

void OfflineGame::MoveMeldTileCursor(int delta)
{
    StartTurnIfNeeded(Seat::South);
    if (state_.turn.table.empty())
    {
        tableTileCursor_ = 0;
        return;
    }

    const std::vector<Tile>& tiles = state_.turn.table[tableMeldCursor_].tiles;
    if (tiles.empty())
    {
        tableTileCursor_ = 0;
        return;
    }

    const int count = static_cast<int>(tiles.size());
    int next = static_cast<int>(tableTileCursor_) + delta;
    if (next < 0)
    {
        next = 0;
    }
    if (next >= count)
    {
        next = count - 1;
    }

    tableTileCursor_ = static_cast<std::size_t>(next);
}

void OfflineGame::ToggleHandSelection()
{
    StartTurnIfNeeded(Seat::South);
    if (state_.turn.hand.empty() || handCursor_ >= state_.turn.hand.size())
    {
        return;
    }

    const int index = static_cast<int>(handCursor_);
    auto found = std::find(selectedHandIndices_.begin(), selectedHandIndices_.end(), index);
    if (found == selectedHandIndices_.end())
    {
        selectedHandIndices_.push_back(index);
        std::sort(selectedHandIndices_.begin(), selectedHandIndices_.end());
    }
    else
    {
        selectedHandIndices_.erase(found);
    }
}

void OfflineGame::ClearSelection()
{
    selectedHandIndices_.clear();
}

bool OfflineGame::CreateMeldFromSelection()
{
    if (!IsHumanTurn())
    {
        return false;
    }

    StartTurnIfNeeded(Seat::South);
    if (selectedHandIndices_.size() < 3)
    {
        SetStatus("Yeni per icin elde en az 3 tas secmelisin.");
        return false;
    }

    std::vector<int> tileIds;
    for (int index : selectedHandIndices_)
    {
        if (index < 0 || static_cast<std::size_t>(index) >= state_.turn.hand.size())
        {
            SetStatus("Secim guncel degil, tekrar sec.");
            return false;
        }

        tileIds.push_back(state_.turn.hand[static_cast<std::size_t>(index)].id);
    }

    if (!CreateMeldFromHand(Seat::South, tileIds))
    {
        SetStatus("Secili taslarla gecerli bir per kurulamadi.");
        return false;
    }

    selectedHandIndices_.clear();
    ClampCursors();
    SetStatus("Secili taslar masaya yeni per olarak acildi.");
    return true;
}

bool OfflineGame::AddSelectedTileToFocusedMeld()
{
    if (!IsHumanTurn())
    {
        return false;
    }

    StartTurnIfNeeded(Seat::South);
    if (selectedHandIndices_.size() != 1)
    {
        SetStatus("Masaya eklemek icin elde tam bir tas secili olmali.");
        return false;
    }
    if (state_.turn.table.empty())
    {
        SetStatus("Masada hedef bir per yok.");
        return false;
    }

    const int handIndex = selectedHandIndices_.front();
    if (handIndex < 0 || static_cast<std::size_t>(handIndex) >= state_.turn.hand.size())
    {
        SetStatus("Secili tas gecerli degil.");
        return false;
    }

    const int tileId = state_.turn.hand[static_cast<std::size_t>(handIndex)].id;
    if (!AddTileToMeld(Seat::South, tileId, tableMeldCursor_))
    {
        SetStatus("Bu tas secili pere eklenemiyor.");
        return false;
    }

    selectedHandIndices_.clear();
    ClampCursors();
    SetStatus("Tas secili pere eklendi.");
    return true;
}

bool OfflineGame::TakeFocusedTileFromTable()
{
    if (!IsHumanTurn())
    {
        return false;
    }

    StartTurnIfNeeded(Seat::South);
    if (state_.turn.table.empty())
    {
        SetStatus("Masada alinacak per yok.");
        return false;
    }

    const Meld& meld = state_.turn.table[tableMeldCursor_];
    if (meld.tiles.empty())
    {
        SetStatus("Secili perde tas yok.");
        return false;
    }

    if (!RemoveTileFromMeld(Seat::South, tableMeldCursor_, meld.tiles.size() - 1))
    {
        SetStatus("Masadan tas alinamadi.");
        return false;
    }

    ClampCursors();
    SetStatus("Secili tas masadan ele alindi.");
    return true;
}

bool OfflineGame::CommitHumanTurn()
{
    if (!IsHumanTurn())
    {
        return false;
    }

    std::string error;
    if (!CommitTurn(Seat::South, error))
    {
        SetStatus(error);
        return false;
    }

    selectedHandIndices_.clear();
    UpdateBots();
    return true;
}

bool OfflineGame::DrawOrPass()
{
    if (!IsHumanTurn())
    {
        return false;
    }

    std::string message;
    const bool success = state_.deck.empty() ? PassTurn(Seat::South, message) : DrawTile(Seat::South, message);
    if (!success)
    {
        SetStatus(message);
        return false;
    }

    selectedHandIndices_.clear();
    UpdateBots();
    return true;
}

const std::vector<int>& OfflineGame::SelectedHandIndices() const
{
    return selectedHandIndices_;
}

std::string OfflineGame::BuildDebugStatus() const
{
    std::ostringstream stream;
    stream << "Deck:" << state_.deck.size() << " Table:" << state_.table.size();
    return stream.str();
}

bool OfflineGame::BeginTurn(Seat seat)
{
    if (state_.currentTurn != seat || state_.isGameOver)
    {
        return false;
    }

    if (state_.turn.active)
    {
        return true;
    }

    PlayerState& player = PlayerForSeat(seat);
    state_.turn.active = true;
    state_.turn.table = state_.table;
    state_.turn.hand = player.hand;
    state_.turn.originalHandIds.clear();
    state_.turn.originalTableIds.clear();
    for (const Tile& tile : player.hand)
    {
        state_.turn.originalHandIds.push_back(tile.id);
    }
    for (const Meld& meld : state_.table)
    {
        for (const Tile& tile : meld.tiles)
        {
            state_.turn.originalTableIds.push_back(tile.id);
        }
    }

    ClampCursors();
    return true;
}

bool OfflineGame::CreateMeldFromHand(Seat seat, const std::vector<int>& tileIds)
{
    if (!BeginTurn(seat))
    {
        return false;
    }

    if (tileIds.size() < 3)
    {
        return false;
    }

    std::vector<Tile> selectedTiles;
    for (int tileId : tileIds)
    {
        auto found = std::find_if(
            state_.turn.hand.begin(),
            state_.turn.hand.end(),
            [tileId](const Tile& tile) { return tile.id == tileId; });
        if (found == state_.turn.hand.end())
        {
            return false;
        }

        selectedTiles.push_back(*found);
    }

    Meld meld{selectedTiles};
    if (!validator_.IsValidMeld(meld))
    {
        return false;
    }

    for (int tileId : tileIds)
    {
        auto found = std::find_if(
            state_.turn.hand.begin(),
            state_.turn.hand.end(),
            [tileId](const Tile& tile) { return tile.id == tileId; });
        state_.turn.hand.erase(found);
    }

    state_.turn.table.push_back(validator_.NormalizeMeld(meld));
    ClampCursors();
    return true;
}

bool OfflineGame::AddTileToMeld(Seat seat, int tileId, std::size_t meldIndex)
{
    if (!BeginTurn(seat))
    {
        return false;
    }

    if (meldIndex >= state_.turn.table.size())
    {
        return false;
    }

    auto found = std::find_if(
        state_.turn.hand.begin(),
        state_.turn.hand.end(),
        [tileId](const Tile& tile) { return tile.id == tileId; });
    if (found == state_.turn.hand.end())
    {
        return false;
    }

    const Tile tile = *found;
    state_.turn.table[meldIndex].tiles.push_back(tile);
    if (!validator_.IsValidMeld(state_.turn.table[meldIndex]))
    {
        state_.turn.table[meldIndex].tiles.pop_back();
        return false;
    }

    state_.turn.table[meldIndex] = validator_.NormalizeMeld(state_.turn.table[meldIndex]);
    state_.turn.hand.erase(found);
    ClampCursors();
    return true;
}

bool OfflineGame::RemoveTileFromMeld(Seat seat, std::size_t meldIndex, std::size_t tileIndex)
{
    if (!BeginTurn(seat))
    {
        return false;
    }

    if (meldIndex >= state_.turn.table.size())
    {
        return false;
    }

    Meld& meld = state_.turn.table[meldIndex];
    if (tileIndex >= meld.tiles.size())
    {
        return false;
    }

    state_.turn.hand.push_back(meld.tiles[tileIndex]);
    meld.tiles.erase(meld.tiles.begin() + static_cast<std::ptrdiff_t>(tileIndex));
    if (meld.tiles.empty())
    {
        state_.turn.table.erase(state_.turn.table.begin() + static_cast<std::ptrdiff_t>(meldIndex));
    }

    ClampCursors();
    return true;
}

bool OfflineGame::CommitTurn(Seat seat, std::string& error)
{
    if (!BeginTurn(seat))
    {
        error = "Sira bu oyuncuda degil.";
        return false;
    }

    std::vector<Meld> normalizedTable;
    normalizedTable.reserve(state_.turn.table.size());
    for (const Meld& meld : state_.turn.table)
    {
        normalizedTable.push_back(validator_.NormalizeMeld(meld));
    }

    if (!validator_.AreAllMeldsValid(normalizedTable))
    {
        error = "Masadaki tum perler gecerli olmali.";
        return false;
    }

    if (!HasSameTilesBeforeAndAfter())
    {
        error = "Hamlede tas kaybi veya kopyasi var.";
        return false;
    }

    PlayerState& player = PlayerForSeat(seat);
    const bool handReduced = state_.turn.hand.size() < state_.turn.originalHandIds.size();
    if (!player.hasOpened && !handReduced)
    {
        error = "Acilmak icin elinden en az bir tas koymalisin.";
        return false;
    }

    if (!player.hasOpened)
    {
        int openingCount = 0;
        for (const Meld& meld : normalizedTable)
        {
            if (IsPureOpeningMeld(meld))
            {
                ++openingCount;
            }
        }

        const int minimumOpening = HasAnyPlayerOpenedBefore(seat) ? 1 : 2;
        if (openingCount < minimumOpening)
        {
            error = minimumOpening == 2
                ? "Ilk acilis icin elinden en az iki gecerli per kurmalisin."
                : "Acilmak icin elinden en az bir gecerli per kurmalisin.";
            return false;
        }
    }

    player.hand = state_.turn.hand;
    player.hasOpened = true;
    state_.table = normalizedTable;
    state_.turn = TurnState{};

    if (player.hand.empty())
    {
        state_.isGameOver = true;
        state_.winnerName = player.name;
        SetStatus(player.name + " oyunu kazandi.");
        error.clear();
        return true;
    }

    SetStatus(player.name + " hamlesini onayladi.");
    AdvanceTurn();
    error.clear();
    return true;
}

bool OfflineGame::DrawTile(Seat seat, std::string& message)
{
    if (state_.currentTurn != seat)
    {
        message = "Sira bu oyuncuda degil.";
        return false;
    }

    if (state_.deck.empty())
    {
        message = "Ortada cekilecek tas kalmadi.";
        return false;
    }

    if (state_.turn.active)
    {
        UndoTurn(seat);
    }

    PlayerState& player = PlayerForSeat(seat);
    player.hand.push_back(DrawFromDeck());
    message = player.name + " ortadan tas cekti.";
    SetStatus(message);
    AdvanceTurn();
    return true;
}

bool OfflineGame::PassTurn(Seat seat, std::string& message)
{
    if (state_.currentTurn != seat)
    {
        message = "Sira bu oyuncuda degil.";
        return false;
    }

    if (!state_.deck.empty())
    {
        message = "Ortada tas varken pas gecilemez.";
        return false;
    }

    if (state_.turn.active)
    {
        UndoTurn(seat);
    }

    message = PlayerForSeat(seat).name + " pas gecti.";
    SetStatus(message);
    AdvanceTurn();
    return true;
}

void OfflineGame::UndoTurn(Seat seat)
{
    if (state_.currentTurn != seat)
    {
        return;
    }

    state_.turn = TurnState{};
    selectedHandIndices_.clear();
    ClampCursors();
}

void OfflineGame::StartTurnIfNeeded(Seat seat)
{
    if (state_.currentTurn == seat && !state_.turn.active && !state_.isGameOver)
    {
        BeginTurn(seat);
    }
}

void OfflineGame::AdvanceTurn()
{
    const std::vector<Seat> order = SeatOrder();
    auto found = std::find(order.begin(), order.end(), state_.currentTurn);
    std::size_t index = static_cast<std::size_t>(std::distance(order.begin(), found));
    index = (index + 1) % order.size();
    state_.currentTurn = order[index];
    state_.turn = TurnState{};
    handCursor_ = 0;
    tableMeldCursor_ = 0;
    tableTileCursor_ = 0;
}

PlayerState& OfflineGame::PlayerForSeat(Seat seat)
{
    return state_.players[static_cast<int>(seat)];
}

const PlayerState& OfflineGame::PlayerForSeat(Seat seat) const
{
    return state_.players[static_cast<int>(seat)];
}

std::vector<int> OfflineGame::GatherAllTileIds(const std::vector<Meld>& table, const std::vector<Tile>& hand) const
{
    std::vector<int> ids;
    for (const Meld& meld : table)
    {
        for (const Tile& tile : meld.tiles)
        {
            ids.push_back(tile.id);
        }
    }
    for (const Tile& tile : hand)
    {
        ids.push_back(tile.id);
    }

    std::sort(ids.begin(), ids.end());
    return ids;
}

bool OfflineGame::HasSameTilesBeforeAndAfter() const
{
    std::vector<int> before = state_.turn.originalHandIds;
    before.insert(before.end(), state_.turn.originalTableIds.begin(), state_.turn.originalTableIds.end());
    std::sort(before.begin(), before.end());
    const std::vector<int> after = GatherAllTileIds(state_.turn.table, state_.turn.hand);
    return before == after;
}

bool OfflineGame::IsPureOpeningMeld(const Meld& meld) const
{
    for (const Tile& tile : meld.tiles)
    {
        if (std::find(state_.turn.originalHandIds.begin(), state_.turn.originalHandIds.end(), tile.id) == state_.turn.originalHandIds.end())
        {
            return false;
        }
    }

    return true;
}

bool OfflineGame::HasAnyPlayerOpenedBefore(Seat seat) const
{
    for (const PlayerState& player : state_.players)
    {
        if (player.seat != seat && player.hasOpened)
        {
            return true;
        }
    }

    return false;
}

Tile OfflineGame::DrawFromDeck()
{
    const Tile tile = state_.deck.back();
    state_.deck.pop_back();
    return tile;
}

std::vector<Tile> OfflineGame::CreateShuffledDeck()
{
    std::vector<Tile> deck;
    deck.reserve(104);
    int id = 1;
    for (int copy = 0; copy < 2; ++copy)
    {
        for (int color = 0; color < 4; ++color)
        {
            for (int number = 1; number <= 13; ++number)
            {
                deck.push_back(Tile{id++, static_cast<TileColor>(color), number});
            }
        }
    }

    std::shuffle(deck.begin(), deck.end(), rng_);
    return deck;
}

void OfflineGame::ClampCursors()
{
    if (!state_.turn.hand.empty() && handCursor_ >= state_.turn.hand.size())
    {
        handCursor_ = state_.turn.hand.size() - 1;
    }
    if (state_.turn.hand.empty())
    {
        handCursor_ = 0;
    }

    if (!state_.turn.table.empty() && tableMeldCursor_ >= state_.turn.table.size())
    {
        tableMeldCursor_ = state_.turn.table.size() - 1;
    }
    if (state_.turn.table.empty())
    {
        tableMeldCursor_ = 0;
        tableTileCursor_ = 0;
        return;
    }

    const std::vector<Tile>& tiles = state_.turn.table[tableMeldCursor_].tiles;
    if (!tiles.empty() && tableTileCursor_ >= tiles.size())
    {
        tableTileCursor_ = tiles.size() - 1;
    }
    if (tiles.empty())
    {
        tableTileCursor_ = 0;
    }

    selectedHandIndices_.erase(
        std::remove_if(
            selectedHandIndices_.begin(),
            selectedHandIndices_.end(),
            [&](int value)
            {
                return value < 0 || static_cast<std::size_t>(value) >= state_.turn.hand.size();
            }),
        selectedHandIndices_.end());
}

void OfflineGame::SetStatus(const std::string& text)
{
    state_.status = text;
}
}
