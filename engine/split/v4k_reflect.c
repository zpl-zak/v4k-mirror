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

static
const char* symbol(const char *s) {
    if( strbeg(s, "const ") ) s += 6;
    if( strbeg(s, "union ") ) s += 6;
    if( strbeg(s, "struct ") ) s += 7;
    if(!strstr(s, " *") ) return s;
    char *copy = va("%s", s);
    do strswap(copy," *","*"); while( strstr(copy, " *") ); // char  * -> char*
    return (const char *)copy;
}

void type_inscribe(const char *TY,unsigned TYsz,const char *infos) {
    reflect_init();
    unsigned TYid = intern(TY = symbol(TY));
    map_find_or_add(reflects, TYid, ((reflect_t){TYid, 0, TYsz, TY, infos}));
}
void enum_inscribe(const char *E,unsigned Eval,const char *infos) {
    reflect_init();
    unsigned Eid = intern(E = symbol(E));
    map_find_or_add(reflects, Eid, ((reflect_t){Eid,0, Eval, E,infos}));
}
unsigned enum_find(const char *E) {
    reflect_init();
    E = symbol(E);
    return map_find(reflects, intern(E))->sz;
}
void function_inscribe(const char *F,void *func,const char *infos) {
    reflect_init();
    unsigned Fid = intern(F = symbol(F));
    map_find_or_add(reflects, Fid, ((reflect_t){Fid,0, 0, F,infos, func}));
    reflect_t *found = map_find(reflects,Fid);
}
void *function_find(const char *F) {
    reflect_init();
    F = symbol(F);
    return map_find(reflects, intern(F))->addr;
}
void struct_inscribe(const char *T,unsigned Tsz,unsigned OBJTYPEid, const char *infos) {
    reflect_init();
    unsigned Tid = intern(T = symbol(T));
    map_find_or_add(reflects, Tid, ((reflect_t){Tid, OBJTYPEid, Tsz, T, infos}));
}
void member_inscribe(const char *T, const char *M,unsigned Msz, const char *infos, const char *type, unsigned bytes) {
    reflect_init();
    unsigned Tid = intern(T = symbol(T));
    unsigned Mid = intern(M = symbol(M));
    type = symbol(type);
    map_find_or_add(reflects, (Mid<<16)|Tid, ((reflect_t){Mid, 0, Msz, M, infos, NULL, Tid, type }));
    // add member separately as well
    if(!members) map_init_int(members);
    array(reflect_t) *found = map_find_or_add(members, Tid, 0);
    array_push(*found, ((reflect_t){Mid, 0, Msz, M, infos, NULL, Tid, type, bytes }));
}
reflect_t member_find(const char *T, const char *M) {
    reflect_init();
    T = symbol(T);
    M = symbol(M);
    return *map_find(reflects, (intern(M)<<16)|intern(T));
}
void *member_findptr(void *obj, const char *T, const char *M) {
    reflect_init();
    T = symbol(T);
    M = symbol(M);
    return (char*)obj + member_find(T,M).sz;
}
array(reflect_t)* members_find(const char *T) {
    reflect_init();
    T = symbol(T);
    return map_find(members, intern(T));
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

// type0 is reserved (no type)
// type1 reserved for objs
// type2 reserved for entities
// @todo: type3 and 4 likely reserved for components and systems??
// enum { OBJTYPE_vec3 = 0x03 };

AUTOTEST {
    // register structs, enums and functions. with and without comments+tags

    STRUCT( vec3, float, x );
    STRUCT( vec3, float, y );
    STRUCT( vec3, float, z, "Up" );

    ENUM( IMAGE_RGB );
    ENUM( TEXTURE_RGB, "3-channel Red+Green+Blue texture flag" );
    ENUM( TEXTURE_RGBA, "4-channel Red+Green+Blue+Alpha texture flag" );

    FUNCTION( puts );
    FUNCTION( printf, "function that prints formatted text to stdout" );

    // verify some reflected infos

    test( function_find("puts") == puts );
    test( function_find("printf") == printf );

    test( enum_find("TEXTURE_RGB") == TEXTURE_RGB );
    test( enum_find("TEXTURE_RGBA") == TEXTURE_RGBA );

    // iterate reflected struct
    for each_member("vec3", R) {
        //printf("+%s vec3.%s (+%x) // %s\n", R->type, R->name, R->member_offset, R->info);
    }

    // reflect_print("puts");
    //reflect_print("TEXTURE_RGBA");
    //reflect_print("vec3");

    // reflect_dump("*");
}
