// C reflection: enums, functions, structs, members and anotations.
// - rlyeh, public domain
//
// @todo: nested structs? pointers in members?
// @todo: declare TYPEDEF(vec3, float[3]), TYPEDEF(mat4, vec4[4]/*float[16]*/)

static map(unsigned, reflected_t) reflects;
static map(unsigned, array(reflected_t)) members;

void reflected_printf(reflected_t *r) {
    printf("name:%s info:'%s' id:%u objtype:%u sz:%u addr:%p parent:%u type:%s",
        r->name ? r->name : "", r->info ? r->info : "", r->id, r->objtype, r->sz, r->addr, r->parent, r->type ? r->type : "");
}
void reflected_printf_all() {
    for each_map_ptr(reflects, unsigned, k, reflected_t, p) {
        reflected_printf(p);
        puts("");
    }
}

void type_inscribe(const char *TY,unsigned TYid,unsigned TYsz,const char *infos) {
    if(!reflects) map_init_int(reflects);
    map_find_or_add(reflects, TYid, ((reflected_t){TYid, 0, TYsz, TY, infos}));
}
void enum_inscribe(const char *E,unsigned Eid,unsigned Eval,const char *infos) {
    if(!reflects) map_init_int(reflects);
    map_find_or_add(reflects, Eid, ((reflected_t){Eid,0, Eval, E,infos}));
}
unsigned enum_find(const char *E) {
    return map_find(reflects, intern(E))->sz;
}
void function_inscribe(const char *F,unsigned Fid,void *func,const char *infos) {
    if(!reflects) map_init_int(reflects);
    map_find_or_add(reflects, Fid, ((reflected_t){Fid,0, 0, F,infos, func}));
    reflected_t *found = map_find(reflects,Fid);
}
void *function_find(const char *F) {
    return map_find(reflects, intern(F))->addr;
}
void struct_inscribe(const char *T,unsigned Tid,unsigned Tsz,unsigned OBJTYPEid, const char *infos) {
    if(!reflects) map_init_int(reflects);
    map_find_or_add(reflects, Tid, ((reflected_t){Tid, OBJTYPEid, Tsz, T, infos}));
}
void member_inscribe(unsigned Tid, const char *M,unsigned Mid,unsigned Msz, const char *infos, const char *type) {
    if(!reflects) map_init_int(reflects);
    map_find_or_add(reflects, (Mid<<16)|Tid, ((reflected_t){Mid, 0, Msz, M, infos, NULL, Tid, type }));
    // add member separately as well
    if(!members) map_init_int(members);
    array(reflected_t) *found = map_find_or_add(members, Tid, 0);
    array_push(*found, ((reflected_t){Mid, 0, Msz, M, infos, NULL, Tid, type }));
}
reflected_t member_find(const char *T, const char *M) {
    return *map_find(reflects, (intern(M)<<16)|intern(T));
}
void *member_findptr(void *obj, const char *T, const char *M) {
    return (char*)obj + member_find(T,M).sz;
}
array(reflected_t) members_find(const char *T) {
    return *map_find(members, intern(T));
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

ifdef(objapi, enum { OBJTYPE_MyVec4 = 0x100 });

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

    // reflected_printf_all();
}
