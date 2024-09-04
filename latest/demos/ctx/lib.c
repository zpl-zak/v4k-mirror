#define API IMPORT
#include "v4k.h" // Minimal C sample
__declspec(dllexport) void test() {
    ui_notify("dll test", "hello");
}
