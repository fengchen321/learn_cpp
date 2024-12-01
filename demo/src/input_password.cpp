#include "input_password.h"
#include <cstring>
#include <unistd.h>
#include <termios.h>

struct StdinRawify {
    struct termios old_term;
    bool saved;

    StdinRawify() {
        saved = false;
        if (isatty(STDIN_FILENO)) { // check if stdin is a terminal
            struct termios term;
            tcgetattr(STDIN_FILENO, &term); // save current settings
            memcpy(&old_term, &term, sizeof(term));
            saved = true;
            term.c_lflag &= ~ICANON;  // disable canonical mode
            term.c_lflag &= ~ECHO;    // disable echo
            tcsetattr(STDIN_FILENO, TCSANOW, &term);
        }
    }

    ~StdinRawify() {
        if (saved) {
            tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
        }
    }
};

std::string input_password(const char *prompt, size_t max_size) {
    if (prompt) {
        fprintf(stderr, "%s", prompt);
    }
    std::string ret;
    StdinRawify stdinRawifier;
    while (true) {
        int c = getchar();
        if (c == EOF)
            break;
        if (c == '\n' || c == '\r') {
            fputc('\n', stderr);
            break;
        } else if (c == '\b' || c == '\x7f') {
            if (ret.size() > 0) {
                ret.pop_back();
                fprintf(stderr, "\b \b");
            }
        } else {
            if (ret.size() < max_size) {
                ret.push_back(c);
                fputc('*', stderr);
            }
        }
    }
    return ret;
}