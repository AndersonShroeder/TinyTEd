#include <termgui.hh>
#include <config.hh>
#include <fileio.hh>
#include <iostream>
#include <inhandler.hh>
#include <vector>
#include <map>
#include <signal.h>
#include <commands.hh>

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

    while (true)
    {
        terminalGUI.draw();
        switch (InputHandler::processKey(config.cursor, config.fileData, config.term, config.status))
        {
        case InputHandler::procval::FAILURE:
            // Handle processing error
            terminalGUI.reset();
            config.term.exitRaw();
            return 1;

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

        case InputHandler::procval::SHUTDOWN:
            goto exit;
            break;

        default:
            break;
        }
    }

exit:
    terminalGUI.reset();
    config.term.exitRaw();
    return 0;
}
