#ifndef MT2_INPUT_H
#define MT2_INPUT_H

// Read one key without waiting for Enter (Windows: conio; POSIX: raw tty).
// Returns unsigned byte 0-255, or -1 on EOF/error, or -2 if key was ignored (e.g. arrow prefix).
int read_key_immediate();

#endif
