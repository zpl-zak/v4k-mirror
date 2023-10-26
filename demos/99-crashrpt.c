#include "v4k.h"

void handle_crash() {
	alert("crash!");
}

int main() {
	trap_install();
	atexit(handle_crash);
	app_crash();
	return 0;
}