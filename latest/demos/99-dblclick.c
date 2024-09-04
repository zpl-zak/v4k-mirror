#include "v4k.h" // Minimal C sample
int main() {
    window_create(30.0, WINDOW_SQUARE); // 75% size, no extra flags
    float t=0.0;
    while( window_swap() && !input(KEY_ESC) ) { // game loop
        if (input_repeat(KEY_SPACE, 750)) {
            printf("250ms repeat after %.02f\n", t);
            t=0;
        }

        t += window_delta();
    }
}