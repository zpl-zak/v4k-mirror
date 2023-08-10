#include "v4k.h"

typedef void* (*rpc_function)();

typedef struct rpc_call {
    char *method;
    rpc_function function;
    uint64_t function_hash;
} rpc_call;

#define RPC_SIGNATURE_i_iii UINT64_C(0x78409099752fa48a) // printf("%llx\n, HASH_STR("int(int,int,int)"));
#define RPC_SIGNATURE_i_ii  UINT64_C(0x258290edf43985a5) // printf("%llx\n, HASH_STR("int(int,int)"));
#define RPC_SIGNATURE_s_s   UINT64_C(0x97deedd17d9afb12) // printf("%llx\n, HASH_STR("char*(char*)"));
#define RPC_SIGNATURE_s_v   UINT64_C(0x09c16a1242049b80) // printf("%llx\n, HASH_STR("char*(void)"));

static
rpc_call rpc_new_call(const char *signature, rpc_function function) {
    if( signature && function ) {
        array(char*)tokens = strsplit(signature, "(,)");
        if( array_count(tokens) >= 1 ) {
            char *method = strrchr(tokens[0], ' ')+1;
            char *rettype = va("%.*s", (int)(method - tokens[0] - 1), tokens[0]);
            int num_args = array_count(tokens) - 1;
            char* hash_sig = va("%s(%s)", rettype, num_args ? (array_pop_front(tokens), strjoin(tokens, ",")) : "void");
            uint64_t hash = hash_str(hash_sig);
            method = va("%s%d", method, num_args );
#if RPC_DEBUG
            printf("%p %p %s `%s` %s(", function, (void*)hash, rettype, hash_sig, method); for(int i = 0, end = array_count(tokens); i < end; ++i) printf("%s%s", tokens[i], i == (end-1)? "":", "); puts(");");
#endif
            return (rpc_call) { strdup(method), function, hash }; // LEAK
        }
    }
    return (rpc_call) {0};
}

static map(char*, rpc_call) rpc_calls = 0;

static
void rpc_insert(const char *signature, void *function ) {
    rpc_call call = rpc_new_call(signature, function);
    if( call.method ) {
        if( !rpc_calls ) map_init(rpc_calls, less_str, hash_str);
        if( map_find(rpc_calls, call.method)) {
            map_erase(rpc_calls, call.method);
        }
        map_insert(rpc_calls, call.method, call);
    }
}

static
char *rpc_full(unsigned id, const char* method, unsigned num_args, char *args[]) {
#if RPC_DEBUG
    printf("id:%x method:%s args:", id, method );
    for( int i = 0; i < num_args; ++i ) printf("%s,", args[i]); puts("");
#endif

    method = va("%s%d", method, num_args);
    rpc_call *found = map_find(rpc_calls, (char*)method);
    if( found ) {
        switch(found->function_hash) {
            case RPC_SIGNATURE_i_iii: return va("%d %d", id, (int)(uintptr_t)found->function(atoi(args[0]), atoi(args[1]), atoi(args[2])) );
            case RPC_SIGNATURE_i_ii:  return va("%d %d", id, (int)(uintptr_t)found->function(atoi(args[0]), atoi(args[1])) );
            case RPC_SIGNATURE_s_s:   return va("%d %s", id, (char*)found->function(args[0]) );
            case RPC_SIGNATURE_s_v:   return va("%d %s", id, (char*)found->function() );
            default: break;
        }
    }
    return va("%d -1", id);
}

static
array(char*) rpc_parse_args( const char *cmdline, bool quote_whitespaces ) { // parse cmdline arguments. must array_free() after use
    // - supports quotes: "abc" "abc def" "abc \"def\"" "abc \"def\"""ghi" etc.
    // - #comments removed
    array(char*) args = 0; // LEAK
    for( int i = 0; cmdline[i]; ) {
        char buf[256] = {0}, *ptr = buf;
        while(cmdline[i] && isspace(cmdline[i])) ++i;
        bool quoted = cmdline[i] == '\"';
        if( quoted ) {
            while(cmdline[++i]) {
                char ch = cmdline[i];
                /**/ if (ch == '\\' && cmdline[i + 1] == '\"') *ptr++ = '\"', ++i;
                else if (ch == '\"' && cmdline[i + 1] == '\"') ++i;
                else if (ch == '\"' && (!cmdline[i + 1] || isspace(cmdline[i + 1]))) {
                    ++i; break;
                }
                else *ptr++ = ch;
            }
        } else {
            while(cmdline[i] && !isspace(cmdline[i])) *ptr++ = cmdline[i++];
        }
        if (buf[0] && buf[0] != '#') { // exclude empty args + comments
            if( quote_whitespaces && quoted )
            array_push(args, va("\"%s\"",buf));
            else
            array_push(args, va("%s",buf));
        }
    }
    return args;
}

static
char* rpc(unsigned id, const char* cmdline) {
    array(char*) args = rpc_parse_args(cmdline, false);
    int num_args = array_count(args);
    char *ret = num_args ? rpc_full(id, args[0], num_args - 1, &args[1]) : rpc_full(id, "", 0, NULL);
    array_free(args);
    return ret;
}

static void enet_quit(void) {
    do_once {
        // enet_deinitialize();
    }
}
static void enet_init() {
    do_once {
        if( enet_initialize() != 0 ) {
            PANIC("cannot initialize enet");
        }
        atexit( enet_quit );
    }
}

// -----------------------------------------------------------------------------
// demo

int rpc_add2(int num1, int num2) {
    return num1+num2;
}
int rpc_add3(int num1, int num2, int num3) {
    return num1+num2+num3;
}
char *rpc_echo(char *text) {
    return text;
}

int main() {
    rpc_insert("int add(int,int)",     rpc_add2);
    rpc_insert("int add(int,int,int)", rpc_add3);
    rpc_insert("char* echo(char*)",    rpc_echo);
    puts(rpc(0,"add 1 2"));              // -> 3
    puts(rpc(1,"add 100 3 -3"));         // -> 100
    puts(rpc(2,"echo \"hello world\"")); // -> hello world
}
