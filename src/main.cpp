#include <termgui.hh>
#include <config.hh>
#include <fileio.hh>
#include <iostream>
#include <inhandler.hh>
#include <vector>
#include <map>
#include <signal.h>
#include <commands.hh>
#include <fcntl.h>
#include <inreader.hh>

const std::map<char, std::string> commands = {
    {'v', "Launches TinyTEd in verbose mode"},
    {'h', "Lists launch options for TinyTEd"},
};

/**
 * @brief Processes command-line input arguments.
 *
 * @param gui The TerminalGUI object for displaying UI elements.
 * @param config The configuration object holding editor state.
 * @param argc Argument count from the command line.
 * @param argv Argument vector from the command line.
 */
void processInput(TerminalGUI &gui, Config &config, int argc, char *argv[])
{
    const std::vector<std::string> args(argv + 1, argv + argc);
    for (const std::string &arg : args)
    {
        if (arg.at(0) == '-')
        {
            // Handle CLI options
            const std::vector<char> options(arg.begin() + 1, arg.end());
            for (char option : options)
            {
                switch (option)
                {
                case 'v':
                    gui.splashScreen();
                    break;
                case 'h':
                    std::cout << "TinyTEd Help Page:\r\n";
                    for (const auto &[key, description] : commands)
                    {
                        std::cout << '\t' << key << " - " << description << "\r\n";
                    }
                    gui.reset();
                    config.term.exitRaw();
                    exit(0);
                default:
                    std::cerr << "Unknown argument: " << option << std::endl;
                    exit(1);
                }
            }
        }
        else
        {
            // Assume argument is a filename
            if (!FileIO::openFile(config, arg))
            {
                std::cerr << "Failed to open file: " << arg << std::endl;
                exit(1);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    Config config;
    TerminalGUI terminalGUI(config);

    config.term.enterRaw();
    config.term.getWindowSize();

    terminalGUI.reset();
    processInput(terminalGUI, config, argc, argv);
    config.status.setStatusMsg("HELP: Ctrl-Q = quit");

    // // Non blocking keystroke read
    // int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    // fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    while (true)
    {
        // Read from sockfd
        if (config.conn.connected)
        {
            while (config.recv() > 0) {
                config.status.setStatusMsg("recv at " + std::to_string(config.mod.x) + ", " + std::to_string(config.mod.x));
                size_t copyX = config.cursor.cx;
                size_t copyY = config.cursor.cy;
                size_t copyRX = config.cursor.rx;


                // Adjust cursor
                config.cursor.cy = config.mod.y;
                config.cursor.cx = config.mod.x;
                config.cursor.rx = config.mod.rx;
                terminalGUI.updateCursor(config.cursor);

                // Process the input
                InputHandler::processKey(config);

                // Reset cursor
                config.cursor.cx = copyX;
                config.cursor.cy = copyY;
                config.cursor.rx = copyRX;
                terminalGUI.updateCursor(config.cursor);
            }
        }

        // draw
        terminalGUI.draw();

        // Process Input -> skip until actually read a key
        config.mod.c = InputReader::readKey();
        if (config.mod.c < 0) {
            continue;
        } 
        config.mod.x = config.cursor.cx;
        config.mod.y = config.cursor.cy;
        config.mod.rx = config.cursor.rx;
        switch (InputHandler::processKey(config))
        {
        case InputHandler::procval::FAILURE:
            // Handle processing error
            goto exit;
            break;

        case InputHandler::procval::PROMPTSAVE:
        {
            std::string path;
            if (config.fileData.path.empty())
            {
                path = InputHandler::promptUser(terminalGUI, config, "Save as: ", std::nullopt);

                // Check if saving was canceled
                if (path.empty())
                {
                    config.status.setStatusMsg("Save aborted");
                }
                else
                {
                    config.fileData.path = path;
                    if (FileIO::saveFile(config))
                    {
                        config.status.setStatusMsg("File saved");
                    }
                    else
                    {
                        config.status.setStatusMsg("Failed to save file");
                    }
                }
            }
            else
            {
                if (FileIO::saveFile(config))
                {
                    config.status.setStatusMsg("File saved");
                }
                else
                {
                    config.status.setStatusMsg("Failed to save file");
                }
            }
            break;
        }

        case InputHandler::procval::PROMPTSEARCH:
            Commands::Search::run(terminalGUI, config);
            break;

        case InputHandler::procval::PROMPTSERVER:
            Commands::LaunchServer::run(terminalGUI, config);
            if (fcntl(config.conn.sockfd, F_SETFL, O_NONBLOCK) < 0) {
                std::cerr << "fcntl error" << std::endl;
            }
            break;
        case InputHandler::procval::PROMPTCONNECT:
            Commands::ConnectServer::run(terminalGUI, config);
            if (fcntl(config.conn.sockfd, F_SETFL, O_NONBLOCK) < 0) {
                std::cerr << "fcntl error" << std::endl;
            }
            break;

        case InputHandler::procval::SHUTDOWN:
            goto exit;
            break;

        case InputHandler::procval::PROMPTMOD:
            if (config.conn.connected)
            {
                send(config.conn.sockfd, &config.mod, sizeof(config.mod), 0);
            }
            break;

        default:
            break;
        }
    }

exit:
    if (config.conn.connected)
    {
        config.conn.connected = false;
        close(config.conn.sockfd);
    }

    terminalGUI.reset();
    config.term.exitRaw();
    return 0;
}
