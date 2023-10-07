map(char*, bt_func) binds;

void bt_addfun(const char *name, int(*func)()){
    do_once map_init_str(binds);
    map_find_or_add_allocated_key(binds, STRDUP(name), func);
}
bt_func bt_findfun(const char *name) {
    bt_func *found = map_find(binds, (char*)name);
    return found ? *found : 0;
}
char * bt_funcname(bt_func fn) {
    for each_map(binds,char*,k,bt_func,f) {
        if( f == fn ) return k;
    }
    return 0;
}

bt_t bt(const char *ini_file, unsigned flags) {
    bt_t z = {0}, root = z;
    array(char*) m = strsplit(vfs_read(ini_file), "\r\n");

    bt_t *self = &root;
    self->type = cc4(r,o,o,t);
    //self->parent = self;

    for( int i = 0; i < array_count(m); ++i ) {
        // parse ini
        int level = strspn(m[i], " \t");
        char *k = m[i] + level;
        if( k[0] == '[' ) {
            if( strcmp(k+1, "bt]") ) return z; // we only support [bt]
            continue;
        }
        int sep = strcspn(k, " =:");
        char *v = k + sep; if(sep) *v++ = '\0'; else v = k + strlen(k); // v = (char*)"";
        array(char*) args = *v ? strsplit(v, " ") : NULL;

        // insert node in tree
        bt_t *out = self;
        while( level-- ) {
            out = array_back(out->children);
        }
        array_push(out->children, ((bt_t){0}));
        //array_back(out->children)->parent = out;
        out = array_back(out->children);

        // config node
        out->type = *(uint64_t*)va("%-8s", k);
        if( array_count(args) ) out->argf = atof(args[0]);
        if( !strcmp(k, "run") ) out->action = bt_findfun(v);
    }

    return root;
}

int bt_run(bt_t *b) {
    int rc = 0;

    /**/ if( b->type == cc3(          r,u,n) ) { return b->action ? b->action() : 0; }
    else if( b->type == cc3(          n,o,t) ) { return !bt_run(b->children + 0); }
    else if( b->type == cc5(      s,l,e,e,p) ) { return sleep_ss(b->argf), bt_run(b->children + 0); }
    else if( b->type == cc5(      d,e,f,e,r) ) { rc = bt_run(b->children + 0); return sleep_ss(b->argf), rc; }
    else if( b->type == cc4(        l,o,o,p) ) { int rc; for(int i = 0; i < b->argf; ++i) rc = bt_run(b->children + 0); return rc; }
    else if( b->type == cc4(        o,n,c,e) ) { return b->argf ? 0 : (b->argf = 1), bt_run(b->children + 0); }
    else if( b->type == cc5(      c,o,u,n,t) ) { return b->argf <= 0 ? 0 : --b->argf, bt_run(b->children + 0); }
    else if( b->type == cc4(        p,a,s,s) ) { return bt_run(b->children + 0), 1; }
    else if( b->type == cc4(        f,a,i,l) ) { return bt_run(b->children + 0), 0; }
    else if( b->type == cc6(    r,e,s,u,l,t) ) { return bt_run(b->children + 0), !!b->argf; }
    else if( b->type == cc3(          a,l,l) ) { for( int i = 0; i < array_count(b->children); ++i ) if(!bt_run(b->children+i)) return 0; return 1; }
    else if( b->type == cc3(          a,n,y) ) { for( int i = 0; i < array_count(b->children); ++i ) if( bt_run(b->children+i)) return 1; return 0; }
    else if( b->type == cc4(        r,o,o,t) ) { for( int i = 0; i < array_count(b->children); ++i ) rc|=bt_run(b->children+i); return rc; }
    else if( b->type == cc8(r,o,o,t,f,a,i,l) ) { rc = 1; for( int i = 0; i < array_count(b->children); ++i ) rc&=~bt_run(b->children+i); return rc; }

    return 0;
}

int ui_bt(bt_t *b) {
    if( b ) {
        char *info = bt_funcname(b->action);
        if(!info) info = va("%d", array_count(b->children));

        if( ui_collapse(va("%s (%s)", cc8str(b->type), info), va("bt%p",b)) ) {
            for( int i = 0; i < array_count(b->children); ++i) {
                ui_bt(b->children + i);
            }
            ui_collapse_end();
        }
    }
    return 0;
}
