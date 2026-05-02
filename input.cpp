#include "input.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <cerrno>
#include <cstdio>
#include <termios.h>
#include <unistd.h>
#endif

int read_key_immediate() {
#ifdef _WIN32
    int c = _getch();
    if (c == 0 || c == 224) {
        (void)_getch();
        return -2;
    }
    return c;
#else
    struct termios oldt {};
    struct termios newt {};
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) return -1;
    newt = oldt;
    newt.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0) return -1;
    (void)tcflush(STDIN_FILENO, TCIFLUSH);
    unsigned char ch = 0;
    ssize_t n = read(STDIN_FILENO, &ch, 1);
    (void)tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    if (n != 1) return -1;
    return static_cast<int>(ch);
#endif
}
