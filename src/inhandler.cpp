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

        int c;
        while ((c = InputReader::readKey()) < 0);
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

int InputHandler::processKey(Config &cfg, bool breakAny)
{
    static int quitStroke = 1;
    int c = cfg.mod.c;

    if (breakAny)
        return procval::SUCCESS;

    switch (cfg.mod.c)
    {
    case '\r': // Newline
        cfg.fileData.insertNewLine(cfg.cursor);
        return procval::PROMPTMOD;
    case K_CTRL('q'):
    {
        if (cfg.fileData.modified && quitStroke > 0)
        {
            cfg.status.setStatusMsg("UNSAVED CHANGES | Press quit again to discard changes");
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
            cfg.cursor.cy = cfg.cursor.rOffset;
        }
        else if (c == PAGE_DOWN)
        {
            cfg.cursor.cy = cfg.cursor.rOffset + cfg.term.sRow - 1;
            if (cfg.cursor.cy > cfg.fileData.size())
            {
                cfg.cursor.cy = cfg.fileData.size();
            }
        }

        int times = cfg.term.sRow;
        while (times--)
        {
            moveCursor(cfg.cursor, cfg.fileData, c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
        }
        break;
    }
    case HOME:
        cfg.cursor.cx = 0;
        break;
    case END:
        if (cfg.cursor.cy < cfg.fileData.size())
        {
            cfg.cursor.cx = cfg.fileData.at(cfg.cursor.cy)->sRaw.size();
        }
        break;
    case BACKSPACE:
    case K_CTRL('h'):
    case DEL:
        if (c == DEL)
            moveCursor(cfg.cursor, cfg.fileData, ARROW_RIGHT); // Move cursor to the right for DEL
        cfg.fileData.deleteChar(cfg.cursor);
        return procval::PROMPTMOD;
    case ARROW_UP:
    case ARROW_LEFT:
    case ARROW_DOWN:
    case ARROW_RIGHT:
        moveCursor(cfg.cursor, cfg.fileData, c);
        break;
    case K_CTRL('l'):
    case '\x1b':
        // No action needed for these keys
        break;
    default:
        // Handle other keys by inserting them
        if (c >= 0 && c < 128)
        {
            cfg.fileData.insertChar(cfg.cursor, static_cast<char>(c));
        }
        return procval::PROMPTMOD;
    }

    return procval::SUCCESS;
}
