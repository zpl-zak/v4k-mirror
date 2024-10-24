#include "v4k.h" // Minimal C sample



void game(unsigned frame, float dt, double t) {

}

int main() {
    window_create(75.0, 0); // 75% size, no extra flags

    window_swap();
    while( window_swap() ) {
        editor_frame(game);
    }
}