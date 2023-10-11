// C reflection: enums, functions, structs, members and anotations.
// - rlyeh, public domain
//
// @todo: nested structs? pointers in members?
// @todo: declare TYPEDEF(vec3, float[3]), TYPEDEF(mat4, vec4[4]/*float[16]*/)

#define ifdef_objapi(T,...) __VA_ARGS__

typedef struct reflected_t {
    unsigned id, objtype;
    unsigned sz;
    const char *name;
    const char *info;
    void *addr;
    unsigned parent;
    const char *type;
} reflected_t;

// inscribe api

#define ENUM(V, value_annotations) \
    enum_inscribe(#V,intern(#V),V, value_annotations)

#define FUNCTION(F, function_annotations) \
    function_inscribe(#F,intern(#F),(void*)F, function_annotations)

#define STRUCT(T, type, member, member_annotations) \
    struct_inscribe(#T,intern(#T),sizeof(T),ifdef(objapi,OBJTYPE_##T,0),NULL), \
    type_inscribe(#type,intern(#type),sizeof(((T){0}).member),member_annotations), \
    member_inscribe(intern(#T), #member,intern(#member),(uintptr_t)&((T*)0)->member, member_annotations, #type )

// find api

API unsigned           enum_find(const char *E);
API void *             function_find(const char *F);

API reflected_t        member_find(const char *T, const char *M); /// find specific member
API void *             member_findptr(void *obj, const char *T, const char *M);
API array(reflected_t) members_find(const char *T);

// iterate members in a struct

#define each_member(T,R) \
    (array(reflected_t)*found_ = map_find(members, intern(T)); found_; found_ = 0) \
        for(int it_ = 0, end_ = array_count(*found_); it_ != end_; ++it_ ) \
            for(reflected_t *R = (*found_)+it_; R; R = 0 )

// private api, still exposed

API void               type_inscribe(const char *TY,unsigned TYid,unsigned TYsz,const char *infos);
API void               enum_inscribe(const char *E,unsigned Eid,unsigned Eval,const char *infos);
API void               struct_inscribe(const char *T,unsigned Tid,unsigned Tsz,unsigned OBJTYPEid, const char *infos);
API void               member_inscribe(unsigned Tid, const char *M,unsigned Mid,unsigned Msz, const char *infos, const char *type);
API void               function_inscribe(const char *F,unsigned Fid,void *func,const char *infos);

API void               reflected_printf(reflected_t *r);
API void               reflected_printf_all();
