#pragma once
#define API IMPORT
#include "v4k.h"

struct asset_t;

typedef int (*ed_proc)(struct asset_t *asset);

typedef struct {
    ed_proc init;
    ed_proc tick;
    ed_proc quit;
    char* (*ext)();
} editor_vtable_t;

typedef struct {
    char *name;
    editor_vtable_t f;
} editor_t;

typedef struct {
    char *name;
    editor_t *ed;

    int slot; //<< internal, used by plugin
    bool opened;
    uint64_t last_modified;
} asset_t;
