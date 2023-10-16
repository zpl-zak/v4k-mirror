// C reflection: enums, functions, structs, members and anotations.
// - rlyeh, public domain
//
// @todo: nested structs? pointers in members?
// @todo: declare TYPEDEF(vec3, float[3]), TYPEDEF(mat4, vec4[4]/*float[16]*/)

static map(unsigned, reflect_t) reflects;
static map(unsigned, array(reflect_t)) members;

void reflect_init() {
    if(!reflects) map_init_int(reflects);
    }
AUTORUN {
    reflect_init();
}

void type_inscribe(const char *TY,unsigned TYid,unsigned TYsz,const char *infos) {
    reflect_init();
    map_find_or_add(reflects, TYid, ((reflect_t){TYid, 0, TYsz, TY, infos}));
}
void enum_inscribe(const char *E,unsigned Eid,unsigned Eval,const char *infos) {
    reflect_init();
    map_find_or_add(reflects, Eid, ((reflect_t){Eid,0, Eval, E,infos}));
}
unsigned enum_find(const char *E) {
    reflect_init();
    return map_find(reflects, intern(E))->sz;
}
void function_inscribe(const char *F,unsigned Fid,void *func,const char *infos) {
    reflect_init();
    map_find_or_add(reflects, Fid, ((reflect_t){Fid,0, 0, F,infos, func}));
    reflect_t *found = map_find(reflects,Fid);
}
void *function_find(const char *F) {
    reflect_init();
    return map_find(reflects, intern(F))->addr;
}
void struct_inscribe(const char *T,unsigned Tid,unsigned Tsz,unsigned OBJTYPEid, const char *infos) {
    reflect_init();
    map_find_or_add(reflects, Tid, ((reflect_t){Tid, OBJTYPEid, Tsz, T, infos}));
}
void member_inscribe(unsigned Tid, const char *M,unsigned Mid,unsigned Msz, const char *infos, const char *type) {
    reflect_init();
    map_find_or_add(reflects, (Mid<<16)|Tid, ((reflect_t){Mid, 0, Msz, M, infos, NULL, Tid, type }));
    // add member separately as well
    if(!members) map_init_int(members);
    array(reflect_t) *found = map_find_or_add(members, Tid, 0);
    array_push(*found, ((reflect_t){Mid, 0, Msz, M, infos, NULL, Tid, type }));
}
reflect_t member_find(const char *T, const char *M) {
    reflect_init();
    return *map_find(reflects, (intern(M)<<16)|intern(T));
}
void *member_findptr(void *obj, const char *T, const char *M) {
    reflect_init();
    return (char*)obj + member_find(T,M).sz;
}
array(reflect_t) members_find(const char *T) {
    reflect_init();
    return *map_find(members, intern(T));
}


void reflect_dump(const char *mask) {
    for each_map_ptr(reflects, unsigned, k, reflect_t, R) {
        if( strmatchi(R->name, mask))
        printf("name:%s info:'%s' id:%u objtype:%u sz:%u addr:%p parent:%u type:%s\n",
            R->name ? R->name : "", R->info ? R->info : "", R->id, R->objtype, R->sz, R->addr, R->parent, R->type ? R->type : "");
    }
}

void reflect_print_(const reflect_t *R) {
    static __thread int tabs = 0;
    printf("%*.s", 4 * (tabs++), "");
    unsigned symbol_q = intern(R->name);
    {
        array(reflect_t) *RR = map_find(members, symbol_q);
        /**/ if( RR ) {       printf("struct %s: %s%s\n", R->name, R->info ? "// ":"", R->info ? R->info : ""); for each_array_ptr(*RR, reflect_t, it) reflect_print_(it); }
        else if( R->addr )    printf("func %s(); %s%s\n", R->name, R->info ? "// ":"", R->info ? R->info : "");
        else if( !R->parent ) printf("enum %s = %d; %s%s\n", R->name, R->sz, R->info ? "// ":"", R->info ? R->info : "");
        else                  printf("%s %s; %s%s\n", R->type, R->name, R->info ? "// ":"", R->info ? R->info : "");
/*
        ifdef(debug,
            printf("%.*sname:%s info:'%s' id:%u objtype:%u sz:%u addr:%p parent:%u type:%s\n",
                tabs, "", R->name ? R->name : "", R->info ? R->info : "", R->id, R->objtype, R->sz, R->addr, R->parent, R->type ? R->type : "");
        );
*/
    }
    --tabs;
}

void reflect_print(const char *symbol) {
    reflect_t *found = map_find(reflects, intern(symbol));
    if( found ) reflect_print_(found);
}

// -- tests

enum {
    MYVALUE0 = 0,
    MYVALUE1,
    MYVALUE2,
    MYVALUEA = 123,
};

typedef struct MyVec4 {
    float x,y,z,w;
} MyVec4;

enum { OBJTYPE_MyVec4 = 0x01 };

AUTOTEST {
    // register structs, enums and functions

    STRUCT( MyVec4, float, x, "Right" );
    STRUCT( MyVec4, float, y, "Forward" );
    STRUCT( MyVec4, float, z, "Up" );
    STRUCT( MyVec4, float, w, "W" );

    ENUM( MYVALUE0, "bla bla #0" );
    ENUM( MYVALUE1, "bla bla #1" );
    ENUM( MYVALUE2, "bla bla #2" );
    ENUM( MYVALUEA, "bla bla (A)" );

    FUNCTION( puts, "handy function that I use a lot" );
    FUNCTION( printf, "handy function that I use a lot" );

    // verify some reflected infos

    test( function_find("puts") == puts );
    test( function_find("printf") == printf );

    test( enum_find("MYVALUE0") == MYVALUE0 );
    test( enum_find("MYVALUE1") == MYVALUE1 );
    test( enum_find("MYVALUE2") == MYVALUE2 );
    test( enum_find("MYVALUEA") == MYVALUEA );

    // iterate reflected struct
    for each_member("MyVec4", R) {
        // printf("+%s MyVec4.%s // %s\n", R->type, R->name, R->info);
    }

    // reflect_print("puts");
    // reflect_print("MYVALUE0");
    // reflect_print("MyVec4");

    // reflect_dump("*");
}
