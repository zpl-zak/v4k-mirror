// ----------------------------------------------------------------------------
// compression api

enum COMPRESS_FLAGS {
    COMPRESS_RAW     = 0,
    COMPRESS_PPP     = (1<<4),
    COMPRESS_ULZ     = (2<<4),
    COMPRESS_LZ4     = (3<<4),
    COMPRESS_CRUSH   = (4<<4),
    COMPRESS_DEFLATE = (5<<4),
    COMPRESS_LZP1    = (6<<4),
    COMPRESS_LZMA    = (7<<4),
    COMPRESS_BALZ    = (8<<4),
    COMPRESS_LZW3    = (9<<4),
    COMPRESS_LZSS    = (10<<4),
    COMPRESS_BCM     = (11<<4),
    COMPRESS_ZLIB    = (12<<4), // same as deflate with header
};

API unsigned zbounds(unsigned inlen, unsigned flags);
API unsigned zencode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags);
API unsigned zexcess(unsigned flags);
API unsigned zdecode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags);

// ----------------------------------------------------------------------------
// cobs en/decoding

API unsigned cobs_bounds(unsigned len);
API unsigned cobs_encode(const void *in, unsigned inlen, void *out, unsigned outlen);
API unsigned cobs_decode(const void *in, unsigned inlen, void *out, unsigned outlen);

// ----------------------------------------------------------------------------
// float un/packing: 8 (micro), 16 (half), 32 (float), 64 (double) types

#define pack754_8(f)    (  pack754((f),  8,  4))
#define pack754_16(f)   (  pack754((f), 16,  5))
#define pack754_32(f)   (  pack754((f), 32,  8))
#define pack754_64(f)   (  pack754((f), 64, 11))
#define unpack754_8(u)  (unpack754((u),  8,  4))
#define unpack754_16(u) (unpack754((u), 16,  5))
#define unpack754_32(u) (unpack754((u), 32,  8))
#define unpack754_64(u) (unpack754((u), 64, 11))

API    uint64_t pack754(long double f, unsigned bits, unsigned expbits);
API long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

// ----------------------------------------------------------------------------
// msgpack v5, schema based struct/buffer bitpacking

// api v2

API int  msgpack(const char *fmt, ... );                // va arg pack "n,b,u,d/i,s,p,f/g,e,[,{"
API bool msgunpack(const char *fmt, ... );              // va arg pack "n,b,u,d/i,s,p,f/g,e,[,{"

// api v1

API int msgpack_new(uint8_t *w, size_t l);
API int msgpack_nil();                                  // write null
API int msgpack_chr(bool n);                            // write boolean
API int msgpack_uns(uint64_t n);                        // write unsigned integer
API int msgpack_int(int64_t n);                         // write integer
API int msgpack_str(const char *s);                     // write string
API int msgpack_bin(const char *s, size_t n);           // write binary pointer
API int msgpack_flt(double g);                          // write real
API int msgpack_ext(uint8_t key, void *val, size_t n);  // write extension type
API int msgpack_arr(uint32_t n);                        // write array mark for next N items
API int msgpack_map(uint32_t n);                        // write map mark for next N pairs (N keys + N values)
API int msgpack_eof();                                  // write full?
API int msgpack_err();                                  // write error?

API bool msgunpack_new( const void *opaque_or_FILE, size_t bytes );
API bool msgunpack_nil();
API bool msgunpack_chr(bool *chr);
API bool msgunpack_uns(uint64_t *uns);
API bool msgunpack_int(int64_t *sig);
API bool msgunpack_str(char **str);
API bool msgunpack_bin(void **bin, uint64_t *len);
API bool msgunpack_flt(float *flt);
API bool msgunpack_dbl(double *dbl);
API bool msgunpack_ext(uint8_t *key, void **val, uint64_t *len);
API bool msgunpack_arr(uint64_t *len);
API bool msgunpack_map(uint64_t *len);
API bool msgunpack_eof();
API bool msgunpack_err();

// alt unpack api v1

enum {
    ERR,NIL,BOL,UNS,SIG,STR,BIN,FLT,EXT,ARR,MAP
};
typedef struct variant {
    union {
    uint8_t     chr;
    uint64_t    uns;
    int64_t     sig;
    uint8_t    *str;
    void       *bin;
    double      flt;
    uint32_t    u32;
    };
    uint64_t sz;
    uint16_t ext;
    uint16_t type; //[0..10]={err,nil,bol,uns,sig,str,bin,flt,ext,arr,map}
} variant;

API bool msgunpack_var(struct variant *var);
