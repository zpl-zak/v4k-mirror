#include "fwk.h" // Minimal C sample

__thread int foo=0; 

int worker(void *ud) {
    printf("thread:%d\n", foo);
    return 0;
}

int main() {
    foo=1;
    printf("main:%d\n", foo);
    thread_destroy(thread(worker, NULL));
    return 0;
}