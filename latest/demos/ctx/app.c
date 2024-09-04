#define API IMPORT
#include "v4k.h" // Minimal C sample
int main() {
    window_create(75.0, 0); // 75% size, no extra flags

    void (*test)(void) = dll("lib.dll", "test");
    test();

    while( window_swap() && !input(KEY_ESC) ) { // game loop

    }
}