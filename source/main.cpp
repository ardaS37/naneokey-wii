#include "game/game.hpp"
#include "ui/text_ui.hpp"

#ifdef GEKKO
#include <gccore.h>
#include <grrlib.h>
#include <wiiuse/wpad.h>
#include "frontal_bmf.h"
#endif

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace
{
#ifdef GEKKO
static ir_t pointer;
static GRRLIB_bytemapFont* uiFont = nullptr;
static int fallbackPointerX = 320;
static int fallbackPointerY = 240;
constexpr int kTileW = 34;
constexpr int kTileH = 44;

struct InputState
{
    nane::UiInput ui;
};

InputState PollInput()
{
    InputState state;
    WPAD_ScanPads();
    const u32 down = WPAD_ButtonsDown(0);
    const u32 held = WPAD_ButtonsHeld(0);
    WPAD_IR(0, &pointer);

    if (pointer.valid)
    {
        fallbackPointerX = static_cast<int>(pointer.x);
        fallbackPointerY = static_cast<int>(pointer.y);
        state.ui.pointer.visible = true;
        state.ui.pointer.x = fallbackPointerX;
        state.ui.pointer.y = fallbackPointerY;
    }
    else
    {
        if (held & WPAD_BUTTON_LEFT)  fallbackPointerX -= 6;
        if (held & WPAD_BUTTON_RIGHT) fallbackPointerX += 6;
        if (held & WPAD_BUTTON_UP)    fallbackPointerY -= 6;
        if (held & WPAD_BUTTON_DOWN)  fallbackPointerY += 6;

        fallbackPointerX = std::max(8, std::min(632, fallbackPointerX));
        fallbackPointerY = std::max(8, std::min(472, fallbackPointerY));

        state.ui.pointer.visible = true;
        state.ui.pointer.x = fallbackPointerX;
        state.ui.pointer.y = fallbackPointerY;
    }

    state.ui.click = (down & WPAD_BUTTON_A) != 0;

    if (down & WPAD_BUTTON_LEFT)
    {
        state.ui.button = nane::UiButton::Left;
        return state;
    }
    if (down & WPAD_BUTTON_RIGHT)
    {
        state.ui.button = nane::UiButton::Right;
        return state;
    }
    if (down & WPAD_BUTTON_UP)
    {
        state.ui.button = nane::UiButton::Up;
        return state;
    }
    if (down & WPAD_BUTTON_DOWN)
    {
        state.ui.button = nane::UiButton::Down;
        return state;
    }
    if (down & WPAD_BUTTON_A)
    {
        state.ui.button = nane::UiButton::A;
        return state;
    }
    if (down & WPAD_BUTTON_B)
    {
        state.ui.button = nane::UiButton::B;
        return state;
    }
    if (down & WPAD_BUTTON_1)
    {
        state.ui.button = nane::UiButton::One;
        return state;
    }
    if (down & WPAD_BUTTON_PLUS)
    {
        state.ui.button = nane::UiButton::Plus;
        return state;
    }
    if (down & WPAD_BUTTON_MINUS)
    {
        state.ui.button = nane::UiButton::Minus;
        return state;
    }
    if (down & WPAD_BUTTON_HOME)
    {
        state.ui.button = nane::UiButton::Home;
        return state;
    }

    return state;
}

void InitVideo()
{
    GRRLIB_Init();
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_0, 640, 480);
    WPAD_SetIdleTimeout(120);
    uiFont = GRRLIB_LoadBMF(frontal_bmf);
}

u32 TileFill(const nane::Tile& tile)
{
    switch (tile.color)
    {
    case nane::TileColor::Red:
        return RGBA(190, 48, 48, 255);
    case nane::TileColor::Blue:
        return RGBA(52, 88, 190, 255);
    case nane::TileColor::Yellow:
        return RGBA(206, 166, 34, 255);
    case nane::TileColor::Black:
        return RGBA(30, 126, 58, 255);
    }

    return RGBA(90, 90, 90, 255);
}

u32 TileText(const nane::Tile& tile)
{
    return tile.color == nane::TileColor::Yellow ? RGBA(35, 22, 5, 255) : RGBA(255, 255, 255, 255);
}

void DrawText(int x, int y, const std::string& text)
{
    if (uiFont != nullptr)
    {
        GRRLIB_PrintBMF(static_cast<f32>(x), static_cast<f32>(y), uiFont, "%s", text.c_str());
    }
}

void DrawPanel(int x, int y, int w, int h, u32 fill, u32 border)
{
    GRRLIB_Rectangle(static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(w), static_cast<f32>(h), fill, true);
    GRRLIB_Rectangle(static_cast<f32>(x), static_cast<f32>(y), static_cast<f32>(w), static_cast<f32>(h), border, false);
}

void DrawTileBox(int x, int y, const nane::Tile& tile, bool selected, bool focused)
{
    const u32 fill = TileFill(tile);
    const u32 border = selected ? RGBA(255, 235, 120, 255) : RGBA(240, 240, 240, 220);
    DrawPanel(x, y, kTileW, kTileH, fill, border);
    if (focused)
    {
        GRRLIB_Rectangle(static_cast<f32>(x - 1), static_cast<f32>(y - 1), static_cast<f32>(kTileW + 2), static_cast<f32>(kTileH + 2), RGBA(255, 255, 255, 255), false);
    }

    DrawText(x + 4, y + 8, nane::TileLabel(tile));
}

void DrawPlayers(const nane::OfflineGame& game)
{
    const int boxW = 145;
    const int boxY = 42;
    for (int i = 0; i < 4; ++i)
    {
        const nane::PlayerState& player = game.State().players[static_cast<std::size_t>(i)];
        const int x = 20 + i * (boxW + 8);
        const bool active = player.seat == game.State().currentTurn;
        DrawPanel(x, boxY, boxW, 28, active ? RGBA(34, 94, 70, 255) : RGBA(20, 66, 50, 255), RGBA(82, 140, 112, 255));
        DrawText(x + 6, boxY + 6, player.name.substr(0, 5) + " " + std::to_string(player.hand.size()));
    }
}

void DrawBoard(const nane::OfflineGame& game)
{
    DrawPanel(20, 82, 600, 140, RGBA(16, 56, 44, 240), RGBA(70, 120, 96, 255));

    const std::vector<nane::Meld>& table = game.State().turn.active ? game.State().turn.table : game.State().table;
    int cursorX = 30;
    int cursorY = 96;
    const int rightLimit = 602;
    for (std::size_t meldIndex = 0; meldIndex < table.size(); ++meldIndex)
    {
        const int meldWidth = static_cast<int>(table[meldIndex].tiles.size()) * 36;
        if (cursorX + meldWidth > rightLimit)
        {
            cursorX = 30;
            cursorY += 54;
        }

        for (std::size_t tileIndex = 0; tileIndex < table[meldIndex].tiles.size(); ++tileIndex)
        {
            const int x = cursorX + static_cast<int>(tileIndex) * 36;
            const bool focused = meldIndex == game.TableMeldCursor();
            DrawTileBox(x, cursorY, table[meldIndex].tiles[tileIndex], false, focused);
        }

        cursorX += meldWidth + 14;
    }

    if (table.empty())
    {
        DrawText(226, 136, "PER AC");
    }
}

void DrawHand(const nane::OfflineGame& game)
{
    DrawPanel(20, 292, 600, 116, RGBA(26, 72, 58, 240), RGBA(90, 142, 110, 255));

    const std::vector<nane::Tile>& hand = game.State().turn.active ? game.State().turn.hand : game.State().players[0].hand;
    for (std::size_t i = 0; i < hand.size(); ++i)
    {
        const int row = static_cast<int>(i) / 10;
        const int col = static_cast<int>(i) % 10;
        const int x = 30 + col * 38;
        const int y = 316 + row * 52;
        const bool selected = std::find(
            game.SelectedHandIndices().begin(),
            game.SelectedHandIndices().end(),
            static_cast<int>(i)) != game.SelectedHandIndices().end();
        const bool focused = i == game.HandCursor();
        DrawTileBox(x, y, hand[i], selected, focused);
    }
}

void DrawButtons()
{
    DrawPanel(464, 82, 156, 196, RGBA(22, 60, 48, 240), RGBA(90, 142, 110, 255));
    const struct ButtonInfo { int x; int y; const char* label; } buttons[] = {
        {476, 94, "E-"}, {544, 94, "E+"},
        {476, 126, "SEC"}, {544, 126, "PER"},
        {476, 158, "P-"}, {544, 158, "P+"},
        {476, 190, "EK"}, {544, 190, "AL"},
        {476, 222, "OK"}, {544, 222, "CEK"},
        {510, 254, "YN"}
    };

    for (const ButtonInfo& button : buttons)
    {
        DrawPanel(button.x, button.y, 56, 24, RGBA(40, 100, 78, 255), RGBA(180, 220, 190, 255));
        DrawText(button.x + 8, button.y + 4, button.label);
    }
}

void DrawStatus(const nane::OfflineGame& game)
{
    DrawPanel(20, 10, 600, 24, RGBA(18, 66, 52, 255), RGBA(80, 132, 108, 255));
    DrawText(28, 14, std::string(nane::SeatName(game.State().currentTurn)).substr(0, 3) + " D" + std::to_string(game.State().deck.size()));
    DrawText(396, 14, game.State().status.substr(0, 12));
}

void PresentFrame(const nane::OfflineGame& game)
{
    GRRLIB_FillScreen(RGBA(18, 56, 42, 255));
    DrawStatus(game);
    DrawPlayers(game);
    DrawBoard(game);
    DrawButtons();
    DrawHand(game);

    GRRLIB_Line(fallbackPointerX - 8, fallbackPointerY, fallbackPointerX + 8, fallbackPointerY, RGBA(255, 255, 255, 255));
    GRRLIB_Line(fallbackPointerX, fallbackPointerY - 8, fallbackPointerX, fallbackPointerY + 8, RGBA(255, 255, 255, 255));
    GRRLIB_Circle(fallbackPointerX, fallbackPointerY, 10, RGBA(255, 230, 120, 220), 0);
    GRRLIB_Render();
}
#else
struct InputState
{
    nane::UiInput ui;
};

InputState PollInput()
{
    return InputState{};
}

void InitVideo()
{
}

void PresentFrame(const nane::OfflineGame&)
{
}
#endif
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    InitVideo();

    nane::OfflineGame game;
    nane::TextUi ui;

    while (true)
    {
        game.UpdateBots();
        const InputState input = PollInput();
        ui.HandleInput(game, input.ui);
        PresentFrame(game);
    }

    return 0;
}
