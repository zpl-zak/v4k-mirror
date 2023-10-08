// -----------------------------------------------------------------------------
// factory of handle ids, based on code by randy gaul (PD/Zlib licensed)
// - rlyeh, public domain
//
// [src] http://www.randygaul.net/wp-content/uploads/2021/04/handle_table.cpp
// [ref] http://bitsquid.blogspot.com.es/2011/09/managing-decoupling-part-4-id-lookup.html
// [ref] http://glampert.com/2016/05-04/dissecting-idhashindex/
// [ref] https://github.com/nlguillemot/dof/blob/master/viewer/packed_freelist.h
// [ref] https://gist.github.com/pervognsen/ffd89e45b5750e9ce4c6c8589fc7f253

#if is(ems)
#define id_t id_t2
#endif

typedef union id_t {
    uint64_t h;
    struct {
#if (ID_INDEX_BITS+ID_COUNT_BITS) != 64
        uint64_t padding : 64-ID_INDEX_BITS-ID_COUNT_BITS;
#endif
        uint64_t index : ID_INDEX_BITS;
        uint64_t count : ID_COUNT_BITS;
    };
} id_t;

typedef struct id_factory id_factory;
id_factory id_factory_create(uint64_t capacity /*= 256*/);
id_t        id_factory_insert(id_factory *f, uint64_t data);
uint64_t     id_factory_getvalue(id_factory *f, id_t handle);
void         id_factory_setvalue(id_factory *f, id_t handle, uint64_t data);
void        id_factory_erase(id_factory *f, id_t handle);
bool        id_factory_isvalid(id_factory *f, id_t handle);
void       id_factory_destroy(id_factory *f);

// ---

typedef struct id_factory {
    uint64_t freelist;
    uint64_t capacity;
    uint64_t canary;
    union entry* entries;
} id_factory;

typedef union entry {
    struct {
        uint64_t data  : ID_DATA_BITS;
        uint64_t count : ID_COUNT_BITS;
    };
    uint64_t h;
} entry;

id_factory id_factory_create(uint64_t capacity) {
    if(!capacity) capacity = 1ULL << ID_INDEX_BITS;

    id_factory f = {0};
    f.entries = CALLOC(1, sizeof(entry) * capacity);
    f.capacity = capacity;

    for (int i = 0; i < capacity - 1; ++i) {
        f.entries[i].data = i + 1;
        f.entries[i].count = 0;
    }
    f.entries[capacity - 1].h = 0;
    f.entries[capacity - 1].data = ~0;
    f.entries[capacity - 1].count = ~0;
    f.canary = f.entries[capacity - 1].data;

    return f;
}

void id_factory_destroy(id_factory *f) {
    FREE(f->entries);
}

id_t id_factory_insert(id_factory *f, uint64_t data) {
    // pop element off the free list
    assert(f->freelist != f->canary && "max alive capacity reached");
    uint64_t index = f->freelist;
    f->freelist = f->entries[f->freelist].data;

    // create new id_t
    f->entries[index].data = data;
    id_t handle = {0};
    handle.index = index;
    handle.count = f->entries[index].count;
    return handle;
}

void id_factory_erase(id_factory *f, id_t handle) {
    // push id_t onto the freelist
    uint64_t index = handle.index;
    f->entries[index].data = f->freelist;
    f->freelist = index;

    // increment the count. this signifies a change in lifetime (this particular id_t is now dead)
    // the next time this particular index is used in alloc, a new `count` will be used to uniquely identify
    f->entries[index].count++;
}

uint64_t id_factory_getvalue(id_factory *f, id_t handle) {
    uint64_t index = handle.index;
    uint64_t count = handle.count;
    assert(f->entries[index].count == count);
    return f->entries[index].data;
}

void id_factory_setvalue(id_factory *f, id_t handle, uint64_t data) {
    uint64_t index = handle.index;
    uint64_t count = handle.count;
    assert(f->entries[index].count == count);
    f->entries[index].data = data;
}

bool id_factory_isvalid(id_factory *f, id_t handle) {
    uint64_t index = handle.index;
    uint64_t count = handle.count;
    if (index >= f->capacity) return false;
    return f->entries[index].count == count;
}

#if 0
// monitor history of a single entity by running `id_factory | find " 123."`
AUTORUN {
    trap_install();

    id_factory f = id_factory_create(optioni("--NUM",256));

    array(id_t) ids = 0;
    for( int i = 0 ; ; ++i, i &= ((1ULL << ID_INDEX_BITS) - 1) ) { // infinite wrap
        printf("count_ids(%d) ", array_count(ids));
        bool insert = randf() < 0.49; // biased against deletion
        if( insert ) {
            id_t h = id_factory_insert(&f, i);
            array_push(ids, h);
            printf("add %llu.%llu\n", h.index, h.count);
        } else {
            int count = array_count(ids);
            if( count ) {
                int chosen = randi(0,count);
                printf("del %d.\n", chosen);
                id_t h = ids[chosen];
                id_factory_erase(&f, h);
                array_erase(ids, chosen);
            }
        }
    }
}
#endif

// ----------------------------------------------------------------------
// public api

static id_factory fid; // @fixme: threadsafe

uintptr_t id_make(void *ptr) {
    do_once fid = id_factory_create(0), id_factory_insert(&fid, 0); // init and reserve id(0)
    id_t newid = id_factory_insert(&fid, (uint64_t)(uintptr_t)ptr ); // 48-bit effective addr
    return newid.h;
}

void *id_handle(uintptr_t id) {
    return (void *)(uintptr_t)id_factory_getvalue(&fid, ((id_t){ (uint64_t)id }) );
}

void id_dispose(uintptr_t id) {
    id_factory_erase(&fid, ((id_t){ (uint64_t)id }) );
}

bool id_valid(uintptr_t id) {
    return id_factory_isvalid(&fid, ((id_t){ (uint64_t)id }) );
}
