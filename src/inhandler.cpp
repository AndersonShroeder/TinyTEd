#include <inhandler.hh>
#include <errmgr.hh>
#include <termgui.hh>
#include <fileio.hh>
#include <unistd.h>
#include <inreader.hh>
#include <cctype>
#include <optional>
#include <commands.hh>

void InputHandler::moveCursor(TTEdCursor &cursor, TTEdFileData &fData, int c)
{
    std::shared_ptr<Row> data = cursor.cy >= fData.size() ? nullptr : fData.at(cursor.cy);

    switch (c)
    {
    case ARROW_UP:
        if (cursor.cy > 0)
            cursor.cy--;
        break;
    case ARROW_LEFT:
        if (cursor.cx > 0)
        {
            cursor.cx--;
        }
        else if (cursor.cy > 0)
        {
            cursor.cy--;
            cursor.cx = fData.at(cursor.cy)->sRaw.size();
        }
        break;
    case ARROW_DOWN:
        if (cursor.cy < fData.size())
            cursor.cy++;
        break;
    case ARROW_RIGHT:
        if (data && cursor.cx < data->sRaw.size())
        {
            cursor.cx++;
        }
        else if (data && cursor.cx == data->sRaw.size())
        {
            cursor.cy++;
            cursor.cx = 0;
        }
        break;
    }

    // Ensure cursor does not exceed the bounds of the current row
    data = cursor.cy >= fData.size() ? nullptr : fData.at(cursor.cy);
    size_t newlen = data ? data->sRaw.size() : 0;
    if (cursor.cx > newlen)
    {
        cursor.cx = newlen;
    }
}

std::string InputHandler::promptUser(TerminalGUI &gui, Config &cfg, const std::string &msg, std::optional<TTEdCommand> cmd)
{
    std::string userInput;
    while (true)
    {
        cfg.status.setStatusMsg(msg + userInput);
        gui.draw();

        int c = InputReader::readKey();
        if (c == DEL || c == K_CTRL('h') || c == BACKSPACE)
        {
            if (!userInput.empty())
                userInput.pop_back();
        }
        else if (c == '\x1b' || c == K_CTRL('q'))
        { // Escape cancels input
            cfg.status.resetStatusMsg();
            if (cmd)
                cmd.value()(cfg, userInput, c);
            return "";
        }
        else if (c == '\r')
        { // Enter submits input
            cfg.status.resetStatusMsg();
            if (cmd)
                cmd.value()(cfg, userInput, c);
            return userInput;
        }
        else if (!std::iscntrl(c) && c < 128)
        { // Accept ASCII printable characters
            userInput += static_cast<char>(c);
        }

        if (cmd)
            cmd.value()(cfg, userInput, c);
    }
}

int InputHandler::processKey(TTEdCursor &cursor, TTEdFileData &fData, TTEdTermData &term, TTEdStatus &stat, bool breakAny)
{
    static int quitStroke = 1;
    int c = InputReader::readKey();

    if (breakAny)
        return procval::SUCCESS;

    switch (c)
    {
    case '\r': // Newline
        fData.insertNewLine(cursor);
        return procval::PROMPTMOD;
    case K_CTRL('q'):
    {
        if (fData.modified && quitStroke > 0)
        {
            stat.setStatusMsg("UNSAVED CHANGES | Press quit again to discard changes");
            quitStroke--;
            break;
        }
        return procval::SHUTDOWN;
    }
    case K_CTRL('s'):
        return procval::PROMPTSAVE;

    case K_CTRL('f'):
        return procval::PROMPTSEARCH;

    case K_CTRL('n'):
        return procval::PROMPTSERVER;
    case K_CTRL('b'):
        return procval::PROMPTCONNECT;

    case PAGE_UP:
    case PAGE_DOWN:
    {
        if (c == PAGE_UP)
        {
            cursor.cy = cursor.rOffset;
        }
        else if (c == PAGE_DOWN)
        {
            cursor.cy = cursor.rOffset + term.sRow - 1;
            if (cursor.cy > fData.size())
            {
                cursor.cy = fData.size();
            }
        }

        int times = term.sRow;
        while (times--)
        {
            moveCursor(cursor, fData, c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
        break;
    }
    case HOME:
        cursor.cx = 0;
        break;
    case END:
        if (cursor.cy < fData.size())
        {
            cursor.cx = fData.at(cursor.cy)->sRaw.size();
        }
        break;
    case BACKSPACE:
    case K_CTRL('h'):
    case DEL:
        if (c == DEL)
            moveCursor(cursor, fData, ARROW_RIGHT); // Move cursor to the right for DEL
        fData.deleteChar(cursor);
        return procval::PROMPTMOD;
    case ARROW_UP:
    case ARROW_LEFT:
    case ARROW_DOWN:
    case ARROW_RIGHT:
        moveCursor(cursor, fData, c);
        break;
    case K_CTRL('l'):
    case '\x1b':
        // No action needed for these keys
        break;
    default:
        // Handle other keys by inserting them
        if (c >= 0 && c < 128)
        {
            fData.insertChar(cursor, static_cast<char>(c));
        }
        return procval::PROMPTMOD;
    }

    return procval::SUCCESS;
}
