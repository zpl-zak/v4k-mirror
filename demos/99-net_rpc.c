// core for remote procedure calls
// - rlyeh, public domain.
//
// format:
// - query:  id method [args.....] ('subject' is alias for arg[0]; and 'object' can be any arg[1..])
// - answer: id error  [values...]
//
// todo:
// - [ ] promote rpc_function to (int argc, void **args) ?
#define V4K_IMPLEMENTATION
#include "engine/joint/v4k.h"

#ifndef RPC_H
#define RPC_H

void  rpc_insert(const char *signature, void * function );

char *rpc(const char *cmdline); // for debugging purposes
char* rpc_quick(unsigned query_number, const char* cmdline);
char *rpc_full(unsigned id, const char* method, unsigned num_args, char *args[]);

#endif

// -----------------------------------------------------------------------------

#define RPC_C
#ifdef RPC_C
#pragma once

typedef void* (*rpc_function)();

typedef struct rpc_call {
    char *method;
    rpc_function function;
    uint64_t function_hash;
} rpc_call;

#define RPC_SIGNATURE_i_iii UINT64_C(0x9830e90d3327e74b) // HASH_STR("int(int,int,int)")
#define RPC_SIGNATURE_i_ii  UINT64_C(0xa7fcab437d38c750) // HASH_STR("int(int,int)")
#define RPC_SIGNATURE_s_s   UINT64_C(0xc4db3d7818162463) // HASH_STR("char*(char*)")
#define RPC_SIGNATURE_s_v   UINT64_C(0x8857a7c1cd20bd7b) // HASH_STR("char*(void)")

rpc_call rpc_new_call(const char *signature, rpc_function function) {
    if( signature && function ) {
        array(char*)tokens = strsplit(signature, "(,)"); array_pop(tokens);
        if( array_count(tokens) >= 1 ) {
            char *method = strrchr(tokens[0], ' ')+1;
            char *rettype = va("%.*s", (int)(method - tokens[0] - 1), tokens[0]);
            int num_args = array_count(tokens) - 1;
            uint64_t hash = hash_str(va("%s(%s)", rettype, num_args ? strjoin(num_args, &tokens[1], ",") : "void" ));
            method = va("%s%d", method, num_args );
#ifdef _DEBUG
            printf("%p %p %s %s(", function, (void*)hash, rettype, method); for(int i = 1, end = array_count(tokens); i < end; ++i) printf("%s%s", tokens[i], i == (end-1)? "":", "); puts(");");
#endif
            return (rpc_call) { strdup(method), function, hash }; // LEAK
        }
    }
    return (rpc_call) {0};
}

static map(char*, rpc_call) rpc_calls = 0;

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

char *rpc_full(unsigned id, const char* method, unsigned num_args, char *args[]) {
#ifdef _DEBUG
    printf("id:%x method:%s args:", id, method );
    for( int i = 0; i < num_args; ++i ) printf("%s", args[i]); puts("");
#endif

    method = va("%s%d", method, num_args);
    rpc_call *found = map_find(rpc_calls, (char*)method);
    if( found ) {
        switch(found->function_hash) {
            default:
            case RPC_SIGNATURE_i_iii: return va("%d \"%d %s\" %d", id, errno, strerror(errno), (int)(uintptr_t)found->function(atoi(args[0]), atoi(args[1]), atoi(args[2])) );
            case RPC_SIGNATURE_i_ii:  return va("%d \"%d %s\" %d", id, errno, strerror(errno), (int)(uintptr_t)found->function(atoi(args[0]), atoi(args[1])) );
            case RPC_SIGNATURE_s_s:   return va("%d \"%d %s\" %s", id, errno, strerror(errno), (char*)found->function(args[0]) );
            case RPC_SIGNATURE_s_v:   return va("%d \"%d %s\" %s", id, errno, strerror(errno), (char*)found->function() );
        }
    }
    return va("%d \"0\" ?", id);
}

char* rpc_quick(unsigned query_number, const char* cmdline) {
    array(char*) args = os_argparse(cmdline, false);
    int num_args = array_count(args);
    char *ret = num_args ? rpc_full(query_number, args[0], num_args - 1, &args[1]) : rpc_full(query_number, "", 0, NULL);
    array_free(args);
    return ret;
}

char *rpc(const char *cmdline) { // for debugging purposes
    char *rc = rpc_quick(0, cmdline);
    puts(rc);
    return rc;
}

// -----------------------------------------------------------------------------

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
    rpc("add 1 2");              // -> 3
    rpc("add 100 3 -3");         // -> 100
    rpc("echo \"hello world\""); // -> hello world
}
#endif // RPC_C
