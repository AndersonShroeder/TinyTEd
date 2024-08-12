#include <commands.hh>
#include <inhandler.hh>

void Commands::Search::callback(Config &cfg, std::string s, int c)
{
    // Track the index of the previous match and the direction of movement
    static int matchPrev = -1;
    static int matchDir = 1;

    // Handle input commands
    if (c == '\r' || c == '\x1b')
    {                   // Enter or Escape key
        matchPrev = -1; // Reset match state
        matchDir = 1;   // Default direction
        return;
    }
    else if (c == ARROW_RIGHT || c == ARROW_DOWN)
    {
        matchDir = 1; // Move forward
    }
    else if (c == ARROW_LEFT || c == ARROW_UP)
    {
        matchDir = -1; // Move backward
    }
    else
    {
        matchPrev = -1; // Reset on other key presses
        matchDir = 1;
    }

    // Start search from the current match or the beginning
    if (matchPrev == -1)
        matchDir = 1;
    int current = matchPrev;

    // Search through the file data
    for (size_t i = 0; i < cfg.fileData.size(); i++)
    {
        current += matchDir;
        if (current < 0)
            current = cfg.fileData.size() - 1;
        else if (current >= (int)cfg.fileData.size())
            current = 0;

        std::shared_ptr<Row> row = cfg.fileData.at(current);

        size_t match = row->sRender.find(s);
        if (match != std::string::npos)
        { // Found a match
            matchPrev = current;
            cfg.cursor.cy = current;
            cfg.cursor.cx = match + s.size();
            cfg.cursor.rOffset = cfg.term.sRow; // Adjust row offset
            break;
        }
    }
}

void Commands::Search::run(TerminalGUI &gui, Config &cfg)
{
    // Save previous cursor state
    size_t prevCX = cfg.cursor.cx;
    size_t prevCY = cfg.cursor.cy;
    size_t prevCOffset = cfg.cursor.cOffset;
    size_t prevROffset = cfg.cursor.rOffset;

    // Prompt user for search input
    std::string s = InputHandler::promptUser(gui, cfg, "Search: ", Commands::Search::callback);

    // Reset cursor position if no search term was provided
    if (s.empty())
    {
        cfg.cursor.cx = prevCX;
        cfg.cursor.cy = prevCY;
        cfg.cursor.cOffset = prevCOffset;
        cfg.cursor.rOffset = prevROffset;
    }
}
