// object: method dispatch tables

#define ctor(obj) obj_method0(obj, ctor) // ctor[obj_typeid(obj)](obj)
#define dtor(obj) obj_method0(obj, dtor) // dtor[obj_typeid(obj)](obj)

API extern void (*ctor[256])(); ///-
API extern void (*dtor[256])(); ///-

const char *obj_typeof( const void *obj ) {
int obj_typeeq(a,b)

// ---


// ---

void *obj_copy(void **dst, const void *src) {
    if(!*dst) return *dst = obj_clone(src);

    if( obj_typeeq(*dst, src) ) {
        return memcpy(*dst, src, obj_sizeof(src));
    }

    return NULL;
}

void *obj_mutate(void **dst_, const void *src) {
    // mutate a class. ie, convert a given object class into a different one,
    // while preserving the original metas and references as much as possible.
    //
    // @fixme: systems might be tracking objects in the future. the fact that we
    // can reallocate a pointer (and hence, change its address) seems way too dangerous,
    // as the tracking systems could crash when referencing a mutated object.
    // solutions: do not reallocate if sizeof(new_class) > sizeof(old_class) maybe? good enough?
    // also, optimization hint: no need to reallocate if both sizes matches, just copy contents.

    if(!*dst_) return *dst_ = obj_clone(src);

    void *dst = *dst_;
    dtor(dst);

        unsigned src_sz = obj_sizeof(src);
        unsigned src_id = obj_typeid(src);

        void *dst_ptr = *((void**)dst - 1);
        unsigned payload = (OBJPAYLOAD16(dst_ptr) & 255) | src_id << 8;
        FREE( OBJUNBOX(dst_ptr) );
        *((void**)dst - 1) = OBJBOX( STRDUP( OBJUNBOX(*((void**)src - 1)) ), payload);

        void *base = (void*)((void**)dst - 1);
        base = REALLOC(base, src_sz + sizeof(void*));
        *dst_ = (char*)base + sizeof(void*);
        dst = (char*)base + sizeof(void*);
        memcpy(dst, src, src_sz);

    ctor(dst);
    return dst;
}


#ifdef OBJ_DEMO

typedef struct MyObject {
    char* id;
    int x,y;
    float rotation;
    struct MyObject *next;
} MyObject;

void tests1() {
    // Construct two objects
    MyObject *root = obj_new(MyObject, 0);
    MyObject *obj = obj_new(MyObject, "An identifier!", 0x11, 0x22, 3.1415f, root );

    // Dump contents of our objects

    obj_hexdump(root);
    obj_hexdump(obj);

    // Save to mem

    char* buffer = obj_save(obj);
    printf("%d bytes\n", (int)strlen(buffer));

    // Clear

    obj_zero( obj );
    obj_hexdump( obj );

    // Reload

    obj_load( obj, buffer );
    obj_hexdump( obj );

    // Copy tests

    {
        MyObject *clone = obj_clone(obj);
        obj_hexdump(clone);
        obj_del(clone);
    }

    {
        MyObject *copy = 0;
        obj_copy(&copy, obj);
        obj_hexdump(copy);
        obj_del(copy);
    }

    {
        MyObject *copy = obj_new(MyObject, "A different identifier!", 0x33, 0x44, 0.0f, root );
        obj_copy(&copy, obj);
        obj_hexdump(copy);
        obj_del(copy);
    }

    {
        void *copy = obj_malloc(100, "an untyped class" );
        obj_mutate(&copy, obj);
        obj_hexdump(copy);
        obj_copy(&copy, obj);
        obj_hexdump(copy);
        obj_del(copy);
    }

    // Benchmarking call overhead.
    // We're here using dtor as a method to test. Since there is actually no
    // destructor associated to this class, it will be safe to call it extensively (no double frees).
    //
    // results:
    // 427 million calls/s @ old i5-4300/1.90Ghz laptop. compiled with "cl /Ox /Os /MT /DNDEBUG /GL /GF /arch:AVX2"

    #ifndef N
    #define N (INT32_MAX-1)
    #endif

    double t = (puts("benchmarking..."), -clock() / (double)CLOCKS_PER_SEC);
    for( int i = 0; i < N; ++i ) {
        dtor(root);
    }
    t += clock() / (double)CLOCKS_PER_SEC;
    printf("Benchmark: %5.2f objcalls/s %5.2fM objcalls/s\n", N/(t), (N/1000)/(t*1000)); // ((N+N)*5) / (t) );

}

void tests2() {
    REGISTER_BOX
    REGISTER_RECT

    box *b = obj_new(box, 100);
    rect *r = obj_new(rect, 100, 200);

    dump(b);
    dump(r);

    printf("%f\n", area(b));
    printf("%f\n", area(r));

    obj_del(b);
    obj_ref(r); obj_unref(r); //obj_del(r);

    int *untyped = obj_malloc( sizeof(int) );
    int *my_number = obj_malloc( sizeof(int), "a comment about my_number" );
    char *my_text = obj_malloc( 32, "some debug info here" );

    *untyped = 100;
    *my_number = 123;
    sprintf( my_text, "hello world" );

    struct my_bitmap { int w, h, bpp; const char *pixels; };
    struct my_bitmap *my_bitmap = obj_new(struct my_bitmap, 2,2,8, "\1\2\3\4");

    printf( "%p(%s,%u)\n", my_bitmap, obj_typeof(my_bitmap), obj_typeid(my_bitmap) );
    printf( "%d(%s,%d)\n", *untyped, obj_typeof(untyped), obj_typeid(untyped) );
    printf( "%d(%s,%d)\n", *my_number, obj_typeof(my_number), obj_typeid(my_number) );
    printf( "%s(%s,%d)\n", my_text, obj_typeof(my_text), obj_typeid(my_text) );

    obj_printf(my_text, "hello world #1\n");
    obj_printf(my_text, "hello world #2\n");
    puts(obj_output(my_text));

    printf( "%s(%s,%d)\n", my_text, obj_typeof(my_text), obj_typeid(my_text) );

    printf( "equal?:%d\n", obj_typeeq(my_number, untyped) );
    printf( "equal?:%d\n", obj_typeeq(my_number, my_number) );
    printf( "equal?:%d\n", obj_typeeq(my_number, my_text) );
    printf( "equal?:%d\n", obj_typeeq(my_number, my_bitmap) );

    obj_free( untyped );
    obj_free( my_text );
    obj_free( my_bitmap );
    obj_del( my_number ); // should not crash, even if allocated with obj_malloc()
}
