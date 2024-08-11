#include <termgui.hh>
#include <config.hh>
#include <fileio.hh>
#include <iostream>
#include <inhandler.hh>
#include <vector>
#include <map>
#include <signal.h>

const std::map<char, std::string> commands = {
    {'v', "Launches TinyTEd in verbose mode"},
    {'h', "Lists launch options for TinyTEd"},
};

void processInput(TerminalGUI &gui, Config &config, int argc, char *argv[]) {
    const std::vector<std::string> args(argv + 1, argv + argc);
    for (std::string arg: args) {
        // CLI option
        if (arg.at(0) == '-') {
            // Allow chained arguments
            std::vector<char> chain(arg.begin() + 1, arg.end());
            for (char c: chain) {
                switch (c) {
                case 'v':
                    gui.splashScreen();
                    break;
                case 'h':
                    std::cout << "TinyTEd Help Page:\r\n";
                    for (std::pair<char, std::string> p: commands) {
                        std::cout << '\t' << p.first << " - " << p.second << "\r\n"; 
                    }
                    gui.reset();
                    config.term.exitRaw();
                    exit(0);
                default:
                    std::cout << "Unknown argument: " << c << std::endl;
                    exit(0);
                }
            }
        }

        // Assume file
        else {
            FileIO::openFile(config.fileData, arg);
        }
    }
}

int main(int argc, char *argv[]) {
    Config config;
    TerminalGUI terminalGUI(config);
    config.term.enterRaw();
    config.term.getWindowSize();

    terminalGUI.reset();
    processInput(terminalGUI, config, argc, argv);
    config.status.setStatusMsg("HELP: Ctrl-Q = quit");
    while (true) {
        terminalGUI.draw();
        switch (InputHandler::processKey(config.cursor, config.fileData, config.term)) {
            case InputHandler::procval::FAILURE:
                // ERROR DESCRIPOR HERE
                terminalGUI.reset();
                config.term.exitRaw();
                return 0;
            case InputHandler::procval::PROMPTSAVE:
                if (config.fileData.filename == "") {
                    std::string s = InputHandler::promptUser(terminalGUI, config.status, "Save as: ");
                    
                    // Check if saving was canceled
                    if (s == "") {
                        config.status.setStatusMsg("Save aborted");
                        break;
                    } else {
                        config.fileData.filename = s;
                        FileIO::saveFile(config.fileData);
                        config.status.setStatusMsg("File saved");
                    }
                }
                else {
                    FileIO::saveFile(config.fileData);
                    config.status.setStatusMsg("File saved");
                }
                break;
            case InputHandler::procval::PROMPTSEARCH: {
                std::string s = InputHandler::promptUser(terminalGUI, config.status, "Search: ");
                if (s == "") {
                    break;
                }

                // Begin search
                for (size_t i = 0; i < config.fileData.size(); i++) {
                    std::shared_ptr<Row> row = config.fileData.at(i);

                    size_t match = row->sRender.find(s);
                    if (match != std::string::npos) {
                        config.cursor.cy = i;
                        config.cursor.cx = match;
                        config.cursor.rOffset = config.term.sRow;
                        break;
                    }
                }

                break;
            }    
            case InputHandler::procval::SHUTDOWN:
                terminalGUI.reset();
                config.term.exitRaw();
                return 0;
            default:
                break;
        }
    }

    terminalGUI.reset();
    config.term.exitRaw();


    return 0;
}