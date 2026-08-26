#include "ui/text_ui.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace nane
{
namespace
{
constexpr int kBoardX = 20;
constexpr int kBoardY = 82;
constexpr int kBoardW = 600;
constexpr int kBoardH = 140;
constexpr int kBoardTileW = 34;
constexpr int kBoardTileH = 44;
constexpr int kBoardGap = 2;
constexpr int kBoardStartX = 30;
constexpr int kBoardStartY = 96;
constexpr int kBoardRowGap = 10;

constexpr int kHandX = 20;
constexpr int kHandY = 274;
constexpr int kHandTileW = 34;
constexpr int kHandTileH = 44;
constexpr int kHandGapX = 4;
constexpr int kHandGapY = 8;
constexpr int kHandStartX = 30;
constexpr int kHandStartY = 298;
constexpr int kHandCols = 10;
}

TextUi::TextUi()
    : focus_(FocusArea::Hand)
    , hoveredAction_(SoftAction::None)
{
}

void TextUi::HandleInput(OfflineGame& game, const UiInput& input)
{
    pointer_ = input.pointer;
    hoveredAction_ = HitTestPointer(pointer_);
    if (input.click && HandleGameAreaClick(game, pointer_))
    {
        return;
    }

    if (input.click && hoveredAction_ != SoftAction::None)
    {
        ExecuteAction(game, hoveredAction_);
        return;
    }

    const UiButton button = input.button;
    if (button == UiButton::None)
    {
        return;
    }

    if (button == UiButton::Home)
    {
        game.StartNewGame();
        focus_ = FocusArea::Hand;
        return;
    }

    if (game.State().isGameOver)
    {
        if (button == UiButton::A || button == UiButton::Plus)
        {
            game.StartNewGame();
            focus_ = FocusArea::Hand;
        }
        return;
    }

    if (!game.IsHumanTurn())
    {
        return;
    }

    switch (button)
    {
    case UiButton::Up:
    case UiButton::Down:
        focus_ = focus_ == FocusArea::Hand ? FocusArea::Table : FocusArea::Hand;
        break;
    case UiButton::Left:
        if (focus_ == FocusArea::Hand)
        {
            game.MoveHandCursor(-1);
        }
        else
        {
            game.MoveMeldCursor(-1);
        }
        break;
    case UiButton::Right:
        if (focus_ == FocusArea::Hand)
        {
            game.MoveHandCursor(1);
        }
        else
        {
            game.MoveMeldCursor(1);
        }
        break;
    case UiButton::A:
        if (focus_ == FocusArea::Hand)
        {
            game.ToggleHandSelection();
        }
        else
        {
            game.AddSelectedTileToFocusedMeld();
        }
        break;
    case UiButton::B:
        if (focus_ == FocusArea::Hand)
        {
            game.ClearSelection();
        }
        else
        {
            game.TakeFocusedTileFromTable();
        }
        break;
    case UiButton::One:
        game.CreateMeldFromSelection();
        break;
    case UiButton::Plus:
        game.CommitHumanTurn();
        break;
    case UiButton::Minus:
        game.DrawOrPass();
        break;
    default:
        break;
    }
}

std::string TextUi::Render(const OfflineGame& game) const
{
    std::ostringstream stream;
    stream << RenderHeaderBar(game) << "\n";
    stream << RenderPlayers(game) << "\n";
    stream << RenderTable(game) << "\n";
    stream << RenderHand(game) << "\n";
    stream << RenderSoftButtons() << "\n";
    stream << RenderFooter(game);
    return stream.str();
}

std::string TextUi::RenderHeaderBar(const OfflineGame& game) const
{
    std::vector<std::string> lines;
    std::ostringstream line;
    line << "SIRA " << SeatName(game.State().currentTurn)
         << "  DESTE " << game.State().deck.size()
         << "  MASA " << game.State().table.size();
    if (game.State().isGameOver)
    {
        line << "  KAZANAN " << game.State().winnerName;
    }
    lines.push_back(line.str());
    return Box("UST", lines, 78);
}

std::string TextUi::RenderTitle(const OfflineGame& game) const
{
    std::ostringstream stream;
    stream << "NANE OKEY WII OFFLINE\n";
    stream << "Sira: " << SeatName(game.State().currentTurn)
           << " | Deste: " << game.State().deck.size()
           << " | Masa per: " << game.State().table.size();
    if (game.State().isGameOver)
    {
        stream << " | Kazanan: " << game.State().winnerName;
    }
    stream << "\n";
    stream << game.BuildDebugStatus() << "\n";
    return stream.str();
}

std::string TextUi::RenderPlayers(const OfflineGame& game) const
{
    std::vector<std::string> lines;
    for (const PlayerState& player : game.State().players)
    {
        std::ostringstream line;
        line << SeatName(player.seat) << " "
             << player.name
             << "  EL " << player.hand.size();
        if (player.seat == game.State().currentTurn)
        {
            line << "  *";
        }
        lines.push_back(line.str());
    }
    return Box("OYUNCU", lines, 78);
}

std::string TextUi::RenderTable(const OfflineGame& game) const
{
    std::vector<std::string> lines;
    if (focus_ == FocusArea::Table)
    {
        lines.push_back("ODAK MASA");
    }
    else
    {
        lines.push_back("MASA");
    }

    const std::vector<Meld>& table = game.State().turn.active ? game.State().turn.table : game.State().table;
    if (table.empty())
    {
        lines.push_back("Bos masa");
        return Box("MASA", lines, 78);
    }

    for (std::size_t meldIndex = 0; meldIndex < table.size(); ++meldIndex)
    {
        std::ostringstream line;
        line << (meldIndex == game.TableMeldCursor() ? ">" : " ") << " ";
        line << "P" << (meldIndex + 1) << " ";
        for (std::size_t tileIndex = 0; tileIndex < table[meldIndex].tiles.size(); ++tileIndex)
        {
            const bool focusedMeld = focus_ == FocusArea::Table && meldIndex == game.TableMeldCursor();
            if (focusedMeld && tileIndex == 0)
            {
                line << "[";
            }
            else
            {
                line << " ";
            }

            line << TileLabel(table[meldIndex].tiles[tileIndex]);

            if (focusedMeld && tileIndex + 1 == table[meldIndex].tiles.size())
            {
                line << "]";
            }
            else
            {
                line << " ";
            }
        }
        lines.push_back(line.str());
        if (lines.size() >= 4)
        {
            break;
        }
    }

    return Box("MASA", lines, 78);
}

std::string TextUi::RenderHand(const OfflineGame& game) const
{
    std::vector<std::string> lines;
    if (focus_ == FocusArea::Hand)
    {
        lines.push_back("ODAK EL");
    }
    else
    {
        lines.push_back("EL");
    }

    const std::vector<Tile>& hand = game.State().turn.active ? game.State().turn.hand : game.State().players[0].hand;
    if (hand.empty())
    {
        lines.push_back("Bos el");
        return Box("EL", lines, 78);
    }

    std::ostringstream currentRow;
    for (std::size_t i = 0; i < hand.size(); ++i)
    {
        const bool selected = std::find(
            game.SelectedHandIndices().begin(),
            game.SelectedHandIndices().end(),
            static_cast<int>(i)) != game.SelectedHandIndices().end();
        const bool focused = focus_ == FocusArea::Hand && i == game.HandCursor();

        if (focused)
        {
            currentRow << ">";
        }
        else
        {
            currentRow << " ";
        }

        if (selected)
        {
            currentRow << "[";
        }
        else
        {
            currentRow << " ";
        }

        currentRow << std::setw(3) << TileLabel(hand[i]);

        if (selected)
        {
            currentRow << "]";
        }
        else
        {
            currentRow << " ";
        }

        if ((i + 1) % 6 == 0)
        {
            lines.push_back(currentRow.str());
            currentRow.str("");
            currentRow.clear();
            if (lines.size() >= 4)
            {
                break;
            }
        }
    }

    if (!currentRow.str().empty())
    {
        lines.push_back(currentRow.str());
    }

    return Box("EL", lines, 78);
}

std::string TextUi::RenderFooter(const OfflineGame& game) const
{
    std::vector<std::string> lines;
    lines.push_back("DURUM " + WrapLine(game.State().status, 40));
    lines.push_back("A SEC  B AL  + ONAY  - CEK");
    if (pointer_.visible)
    {
        std::ostringstream pointerLine;
        pointerLine << "IR " << pointer_.x << "," << pointer_.y;
        lines.push_back(pointerLine.str());
    }
    return Box("ALT", lines, 78);
}

std::string TextUi::RenderSoftButtons() const
{
    const struct ButtonDef
    {
        SoftAction action;
        const char* label;
    } buttons[] = {
        {SoftAction::HandPrev, "EL <"},
        {SoftAction::HandNext, "EL >"},
        {SoftAction::ToggleTile, "SEC"},
        {SoftAction::NewMeld, "YENI PER"},
        {SoftAction::MeldPrev, "PER <"},
        {SoftAction::MeldNext, "PER >"},
        {SoftAction::AddTile, "EKLE"},
        {SoftAction::TakeTile, "AL"},
        {SoftAction::CommitTurn, "ONAY"},
        {SoftAction::DrawOrPass, "CEK/PAS"},
        {SoftAction::NewGame, "YENI OYUN"}
    };

    std::vector<std::string> lines;
    std::ostringstream row;
    for (std::size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); ++i)
    {
        const bool hovered = buttons[i].action == hoveredAction_;
        row << (hovered ? ">" : " ");
        row << "[" << buttons[i].label << "]";
        row << (hovered ? "<" : " ");
        row << " ";
        if ((i + 1) % 4 == 0)
        {
            lines.push_back(row.str());
            row.str("");
            row.clear();
        }
    }

    if (!row.str().empty())
    {
        lines.push_back(row.str());
    }

    return Box("BUTON", lines, 78);
}

bool TextUi::HandleGameAreaClick(OfflineGame& game, const PointerState& pointer)
{
    if (!pointer.visible || !game.IsHumanTurn())
    {
        return false;
    }

    const std::vector<Tile>& hand = game.State().turn.active ? game.State().turn.hand : game.State().players[0].hand;
    for (std::size_t i = 0; i < hand.size(); ++i)
    {
        const int row = static_cast<int>(i) / kHandCols;
        const int col = static_cast<int>(i) % kHandCols;
        const int x = kHandStartX + col * (kHandTileW + kHandGapX);
        const int y = kHandStartY + row * (kHandTileH + kHandGapY);
        if (pointer.x >= x && pointer.x <= x + kHandTileW &&
            pointer.y >= y && pointer.y <= y + kHandTileH)
        {
            focus_ = FocusArea::Hand;
            game.SetHandCursor(i);
            game.ToggleHandSelection();
            return true;
        }
    }

    const std::vector<Meld>& table = game.State().turn.active ? game.State().turn.table : game.State().table;
    int cursorX = kBoardStartX;
    int cursorY = kBoardStartY;
    const int rightLimit = kBoardX + kBoardW - 18;
    for (std::size_t meldIndex = 0; meldIndex < table.size(); ++meldIndex)
    {
        const int meldWidth = static_cast<int>(table[meldIndex].tiles.size()) * (kBoardTileW + kBoardGap);
        if (cursorX + meldWidth > rightLimit)
        {
            cursorX = kBoardStartX;
            cursorY += kBoardTileH + kBoardRowGap;
        }

        if (pointer.x >= cursorX && pointer.x <= cursorX + meldWidth &&
            pointer.y >= cursorY && pointer.y <= cursorY + kBoardTileH)
        {
            focus_ = FocusArea::Table;
            game.SetTableMeldCursor(meldIndex);
            if (game.SelectedHandIndices().size() == 1)
            {
                game.AddSelectedTileToFocusedMeld();
            }
            return true;
        }

        cursorX += meldWidth + 14;
    }

    if (pointer.x >= kBoardX && pointer.x <= kBoardX + kBoardW &&
        pointer.y >= kBoardY && pointer.y <= kBoardY + kBoardH)
    {
        if (game.SelectedHandIndices().size() >= 3)
        {
            focus_ = FocusArea::Table;
            game.CreateMeldFromSelection();
            return true;
        }
    }

    return false;
}

TextUi::SoftAction TextUi::HitTestPointer(const PointerState& pointer) const
{
    if (!pointer.visible)
    {
        return SoftAction::None;
    }

    const struct HitButton
    {
        int x;
        int y;
        int w;
        int h;
        SoftAction action;
    } buttons[] = {
        {476, 94, 56, 24, SoftAction::HandPrev},
        {544, 94, 56, 24, SoftAction::HandNext},
        {476, 126, 56, 24, SoftAction::ToggleTile},
        {544, 126, 56, 24, SoftAction::NewMeld},
        {476, 158, 56, 24, SoftAction::MeldPrev},
        {544, 158, 56, 24, SoftAction::MeldNext},
        {476, 190, 56, 24, SoftAction::AddTile},
        {544, 190, 56, 24, SoftAction::TakeTile},
        {476, 222, 56, 24, SoftAction::CommitTurn},
        {544, 222, 56, 24, SoftAction::DrawOrPass},
        {510, 254, 56, 24, SoftAction::NewGame}
    };

    for (const HitButton& button : buttons)
    {
        if (pointer.x >= button.x && pointer.x <= button.x + button.w &&
            pointer.y >= button.y && pointer.y <= button.y + button.h)
        {
            return button.action;
        }
    }

    return SoftAction::None;
}

void TextUi::ExecuteAction(OfflineGame& game, SoftAction action)
{
    switch (action)
    {
    case SoftAction::HandPrev:
        focus_ = FocusArea::Hand;
        game.MoveHandCursor(-1);
        break;
    case SoftAction::HandNext:
        focus_ = FocusArea::Hand;
        game.MoveHandCursor(1);
        break;
    case SoftAction::ToggleTile:
        focus_ = FocusArea::Hand;
        game.ToggleHandSelection();
        break;
    case SoftAction::NewMeld:
        focus_ = FocusArea::Hand;
        game.CreateMeldFromSelection();
        break;
    case SoftAction::MeldPrev:
        focus_ = FocusArea::Table;
        game.MoveMeldCursor(-1);
        break;
    case SoftAction::MeldNext:
        focus_ = FocusArea::Table;
        game.MoveMeldCursor(1);
        break;
    case SoftAction::AddTile:
        focus_ = FocusArea::Table;
        game.AddSelectedTileToFocusedMeld();
        break;
    case SoftAction::TakeTile:
        focus_ = FocusArea::Table;
        game.TakeFocusedTileFromTable();
        break;
    case SoftAction::CommitTurn:
        game.CommitHumanTurn();
        break;
    case SoftAction::DrawOrPass:
        game.DrawOrPass();
        break;
    case SoftAction::NewGame:
        game.StartNewGame();
        focus_ = FocusArea::Hand;
        break;
    default:
        break;
    }
}

std::string TextUi::Box(const std::string& title, const std::vector<std::string>& lines, std::size_t width)
{
    std::ostringstream stream;
    std::string border(width > 2 ? width - 2 : 0, '=');
    stream << "+" << border << "+\n";

    std::string header = " " + title + " ";
    if (header.size() > width - 2)
    {
        header = header.substr(0, width - 2);
    }
    stream << "|" << std::left << std::setw(static_cast<int>(width - 2)) << header << "|\n";
    stream << "+" << border << "+\n";

    for (const std::string& line : lines)
    {
        std::string clipped = line;
        if (clipped.size() > width - 4)
        {
            clipped = clipped.substr(0, width - 4);
        }
        stream << "| " << std::left << std::setw(static_cast<int>(width - 4)) << clipped << " |\n";
    }

    stream << "+" << border << "+\n";
    return stream.str();
}

std::string TextUi::WrapLine(const std::string& text, std::size_t width)
{
    if (text.size() <= width)
    {
        return text;
    }

    return text.substr(0, width);
}
}
