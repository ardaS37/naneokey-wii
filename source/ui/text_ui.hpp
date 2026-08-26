#pragma once

#include "game/game.hpp"

#include <string>
#include <vector>

namespace nane
{
enum class FocusArea
{
    Hand,
    Table
};

enum class UiButton
{
    None,
    Left,
    Right,
    Up,
    Down,
    A,
    B,
    One,
    Plus,
    Minus,
    Home
};

struct PointerState
{
    bool visible = false;
    int x = 0;
    int y = 0;
};

struct UiInput
{
    UiButton button = UiButton::None;
    PointerState pointer;
    bool click = false;
};

class TextUi
{
public:
    TextUi();

    void HandleInput(OfflineGame& game, const UiInput& input);
    std::string Render(const OfflineGame& game) const;

private:
    enum class SoftAction
    {
        None,
        HandPrev,
        HandNext,
        ToggleTile,
        NewMeld,
        MeldPrev,
        MeldNext,
        AddTile,
        TakeTile,
        CommitTurn,
        DrawOrPass,
        NewGame
    };

    FocusArea focus_;
    PointerState pointer_;
    SoftAction hoveredAction_;

    std::string RenderHeaderBar(const OfflineGame& game) const;
    std::string RenderTitle(const OfflineGame& game) const;
    std::string RenderTable(const OfflineGame& game) const;
    std::string RenderHand(const OfflineGame& game) const;
    std::string RenderPlayers(const OfflineGame& game) const;
    std::string RenderFooter(const OfflineGame& game) const;
    std::string RenderSoftButtons() const;
    SoftAction HitTestPointer(const PointerState& pointer) const;
    bool HandleGameAreaClick(OfflineGame& game, const PointerState& pointer);
    void ExecuteAction(OfflineGame& game, SoftAction action);
    static std::string Box(const std::string& title, const std::vector<std::string>& lines, std::size_t width);
    static std::string WrapLine(const std::string& text, std::size_t width);
};
}
