#include <statmgr.hh>
#include <termgui.hh>
#include <editor.hh>
#include <config.hh>
#include <fileio.hh>
#include <iostream>

int main(int argc, char *argv[]) {
    Config config;
    StateMgr::enterRaw(config);
    Editor editor(config);
    TerminalGUI terminalGUI(config);

    terminalGUI.initGUI();
    // terminalGUI.splashScreen(editor);
    if (argc >= 2) {
        FileIO::openFile(config, argv[1]);
    }
    else {
        std::cout << "NEED A FILE LOL\r\n";
        StateMgr::exitRaw(config);
        exit(0);
    }

    terminalGUI.setStatusMsg("HELP: Ctrl-Q = quit");
    
    while (true) {
        terminalGUI.draw();
        if (editor.processKey() < 0) {
            break;
        }
    }

    terminalGUI.reset();
    StateMgr::exitRaw(config);

    return 0;
}