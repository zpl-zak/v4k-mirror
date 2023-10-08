// ----------------------------------------------------------------------------
// compression api

static struct zcompressor {
    // id of compressor
    unsigned enumerator;
    // name of compressor
    const char name1, *name4, *name;
    // returns worst case compression estimation for selected flags
    unsigned (*bounds)(unsigned bytes, unsigned flags);
    // returns number of bytes written. 0 if error.
    unsigned (*encode)(const void *in, unsigned inlen, void *out, unsigned outcap, unsigned flags);
    // returns number of excess bytes that will be overwritten when decoding.
    unsigned (*excess)(unsigned flags);
    // returns number of bytes written. 0 if error.
    unsigned (*decode)(const void *in, unsigned inlen, void *out, unsigned outcap);
} zlist[] = {
    { COMPRESS_RAW,     '0', "raw",  "raw",     raw_bounds, raw_encode, raw_excess, raw_decode },
    { COMPRESS_PPP,     'p', "ppp",  "ppp",     ppp_bounds, ppp_encode, ppp_excess, ppp_decode },
    { COMPRESS_ULZ,     'u', "ulz",  "ulz",     ulz_bounds, ulz_encode, ulz_excess, ulz_decode },
    { COMPRESS_LZ4,     '4', "lz4x", "lz4x",    lz4x_bounds, lz4x_encode, lz4x_excess, lz4x_decode },
    { COMPRESS_CRUSH,   'c', "crsh", "crush",   crush_bounds, crush_encode, crush_excess, crush_decode },
    { COMPRESS_DEFLATE, 'd', "defl", "deflate", deflate_bounds, deflate_encode, deflate_excess, deflate_decode },
    { COMPRESS_LZP1,    '1', "lzp1", "lzp1",    lzp1_bounds, lzp1_encode, lzp1_excess, lzp1_decode },
    { COMPRESS_LZMA,    'm', "lzma", "lzma",    lzma_bounds, lzma_encode, lzma_excess, lzma_decode },
    { COMPRESS_BALZ,    'b', "balz", "balz",    balz_bounds, balz_encode, balz_excess, balz_decode },
    { COMPRESS_LZW3,    'w', "lzw3", "lzrw3a",  lzrw3a_bounds, lzrw3a_encode, lzrw3a_excess, lzrw3a_decode },
    { COMPRESS_LZSS,    's', "lzss", "lzss",    lzss_bounds, lzss_encode, lzss_excess, lzss_decode },
    { COMPRESS_BCM,     'B', "bcm",  "bcm",     bcm_bounds, bcm_encode, bcm_excess, bcm_decode },
    { COMPRESS_ZLIB,    'z', "zlib", "zlib",    deflate_bounds, deflatez_encode, deflate_excess, deflatez_decode },
};

enum { COMPRESS_NUM = 14 };

static char *znameof(unsigned flags) {
    static __thread char buf[16];
    snprintf(buf, 16, "%4s.%c", zlist[(flags>>4)&0x0F].name4, "0123456789ABCDEF"[flags&0xF]);
    return buf;
}
unsigned zencode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].encode(in, inlen, (uint8_t*)out, outlen, flags & 0x0F);
}
unsigned zdecode(void *out, unsigned outlen, const void *in, unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].decode((uint8_t*)in, inlen, out, outlen);
}
unsigned zbounds(unsigned inlen, unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].bounds(inlen, flags & 0x0F);
}
unsigned zexcess(unsigned flags) {
    return zlist[(flags >> 4) % COMPRESS_NUM].excess(flags & 0x0F);
}

// ----------------------------------------------------------------------------
// cobs en/decoding
//
// Based on code by Jacques Fortier.
// "Redistribution and use in source and binary forms are permitted, with or without modification."
//
// Consistent Overhead Byte Stuffing is an encoding that removes all 0 bytes from arbitrary binary data.
// The encoded data consists only of bytes with values from 0x01 to 0xFF. This is useful for preparing data for
// transmission over a serial link (RS-232 or RS-485 for example), as the 0 byte can be used to unambiguously indicate
// packet boundaries. COBS also has the advantage of adding very little overhead (at least 1 byte, plus up to an
// additional byte per 254 bytes of data). For messages smaller than 254 bytes, the overhead is constant.
//
// This implementation is designed to be both efficient and robust.
// The decoder is designed to detect malformed input data and report an error upon detection.

unsigned cobs_bounds( unsigned len ) {
    return len + ceil(len / 254.0) + 1;
}
unsigned cobs_encode(const void *in, unsigned inlen, void *out, unsigned outlen) {
    const uint8_t *src = (const uint8_t *)in;
    uint8_t *dst = (uint8_t*)out;
    size_t srclen = inlen;

    uint8_t code = 1;
    size_t read_index = 0, write_index = 1, code_index = 0;

    while( read_index < srclen ) {
        if( src[ read_index ] == 0) {
            dst[ code_index ] = code;
            code = 1;
            code_index = write_index++;
            read_index++;
        } else {
            dst[ write_index++ ] = src[ read_index++ ];
            code++;
            if( code == 0xFF ) {
                dst[ code_index ] = code;
                code = 1;
                code_index = write_index++;
            }
        }
    }

    dst[ code_index ] = code;
    return write_index;
}
unsigned cobs_decode(const void *in, unsigned inlen, void *out, unsigned outlen) {
    const uint8_t *src = (const uint8_t *)in;
    uint8_t *dst = (uint8_t*)out;
    size_t srclen = inlen;

    uint8_t code, i;
    size_t read_index = 0, write_index = 0;

    while( read_index < srclen ) {
        code = src[ read_index ];

        if( ((read_index + code) > srclen) && (code != 1) ) {
            return 0;
        }

        read_index++;

        for( i = 1; i < code; i++ ) {
            dst[ write_index++ ] = src[ read_index++ ];
        }
        if( (code != 0xFF) && (read_index != srclen) ) {
            dst[ write_index++ ] = '\0';
        }
    }

    return write_index;
}

#if 0
static
void cobs_test( const char *buffer, int buflen ) {
    char enc[4096];
    int enclen = cobs_encode( buffer, buflen, enc, 4096 );

    char dec[4096];
    int declen = cobs_decode( enc, enclen, dec, 4096 );

    test( enclen >= buflen );
    test( declen == buflen );
    test( memcmp(dec, buffer, buflen) == 0 );

    printf("%d->%d->%d (+%d extra bytes)\n", declen, enclen, declen, enclen - declen);
}
AUTORUN {
    const char *null = 0;
    cobs_test( null, 0 );

    const char empty[] = "";
    cobs_test( empty, sizeof(empty) );

    const char text[] = "hello world\n";
    cobs_test( text, sizeof(text) );

    const char bintext[] = "hello\0\0\0world\n";
    cobs_test( bintext, sizeof(bintext) );

    const char blank[512] = {0};
    cobs_test( blank, sizeof(blank) );

    char longbintext[1024];
    for( int i = 0; i < 1024; ++i ) longbintext[i] = (unsigned char)i;
    cobs_test( longbintext, sizeof(longbintext) );

    assert(~puts("Ok"));
}
#endif

// ----------------------------------------------------------------------------
// float un/packing: 8 (micro), 16 (half), 32 (float), 64 (double) types
// - rlyeh, public domain. original code by Beej.us (PD).
//
// [src] http://beej.us/guide/bgnet/output/html/multipage/advanced.html#serialization
// Modified to encode NaN and Infinity as well.
//
// [1] http://www.mrob.com/pub/math/floatformats.html#minifloat
// [2] microfloat: [0.002 to 240] range.
// [3] half float: can approximate any 16-bit unsigned integer or its reciprocal to 3 decimal places.

uint64_t pack754(long double f, unsigned bits, unsigned expbits) {
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0) return 0; // get this special case out of the way
//< @r-lyeh beware! works for 32/64 only
    else if (f ==  INFINITY) return 0x7f800000ULL << (bits - 32); // 0111 1111 1000
    else if (f == -INFINITY) return 0xff800000ULL << (bits - 32);
    else if (f != f)         return 0x7fc00000ULL << (bits - 32); // 0111 1111 1100 NaN
//< @r-lyeh

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double unpack754(uint64_t i, unsigned bits, unsigned expbits) {
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0) return 0.0;
//< @r-lyeh beware! works for 32 only
    else if (i == 0x7fc00000ULL) return  NAN;      //  NaN
    else if (i == 0x7f800000ULL) return  INFINITY; // +Inf
    else if (i == 0xff800000ULL) return -INFINITY; // -Inf
//< @r-lyeh

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}

// ----------------------------------------------------------------------------
// msgpack v5, schema based struct/buffer bitpacking
// - rlyeh, public domain.
//
// [ref] https://github.com/msgpack/msgpack/blob/master/spec.md
//
// @todo: finish msgunpack()
// @todo: alt api v3
// return number of bytes written; 0 if not space enough.
// int msgpack( uint8_t *buf, const char *fmt, ... );         // if !buf, bulk size; else pack.
// return number of processed bytes; 0 if parse error.
// int msgunpack( const uint8_t *buf, const char *fmt, ... ); // if !buf, test message; else unpack.

struct writer {
    uint8_t *w; // Write pointer into buffer
    size_t len; // Written bytes up to date
    size_t cap; // Buffer capacity
};

struct reader {
    FILE *fp;
    const void *membuf;
    size_t memsize, offset;
    struct variant v; // tmp
};

static __thread struct writer out;
static __thread struct reader in;

static void wrbe(uint64_t n, uint8_t *b) {
#ifndef BIG_ENDIAN
    n = ntoh64(n);
#endif
    memcpy(b, &n, sizeof(uint64_t));
}
static int wr(int len, uint8_t opcode, uint64_t value) {
    uint8_t b[8];
    assert((out.len + (len+1) < out.cap) && "buffer overflow!");
    *out.w++ = (opcode);
    /**/ if(len == 1) *out.w++ = (uint8_t)(value);
    else if(len == 2) wrbe(value, b), memcpy(out.w, &b[6], 2), out.w += 2;
    else if(len == 4) wrbe(value, b), memcpy(out.w, &b[4], 4), out.w += 4;
    else if(len == 8) wrbe(value, b), memcpy(out.w, &b[0], 8), out.w += 8;
    out.len += len+1;
    return len+1;
}
static bool rd(void *buf, size_t len, size_t swap) { // return false any error and/or eof
    bool ret;
    if( in.fp ) {
        assert( !ferror(in.fp) && "invalid file handle (reader)" );
        ret = len == fread((char*)buf, 1, len, in.fp);
    } else {
        assert( in.membuf && "invalid memory buffer (reader)");
        assert( (in.offset + len <= in.memsize) && "memory overflow! (reader)");
        ret = !!memcpy(buf, (char*)in.membuf + in.offset, len);
    }
#ifndef BIG_ENDIAN
    /**/ if( swap && len == 2 ) *((uint16_t*)buf) = ntoh16(*((uint16_t*)buf));
    else if( swap && len == 4 ) *((uint32_t*)buf) = ntoh32(*((uint32_t*)buf));
    else if( swap && len == 8 ) *((uint64_t*)buf) = ntoh64(*((uint64_t*)buf));
#endif
    return in.offset += len, ret;
}
static bool rdbuf(char **buf, size_t len) { // return false on error or out of memory
    char *ptr = REALLOC(*buf, len+1);
    if( ptr && rd(ptr, len, 0) ) {
        (*buf = ptr)[len] = 0;
    } else {
        FREE(ptr), ptr = 0;
    }
    return !!ptr;
}

int msgpack_new(uint8_t *w, size_t l) {
    out.w = w;
    out.len = 0;
    out.cap = l;
    return w != 0 && l != 0;
}
int msgpack_nil() {
    return wr(0, 0xC0, 0);
}
int msgpack_chr(bool c) {
    return wr(0, c ? 0xC3 : 0xC2, 0);
}
int msgpack_uns(uint64_t n) {
    /**/ if (n < 0x80)           return wr(0, n, 0);
    else if (n < 0x100)          return wr(1, 0xCC, n);
    else if (n < 0x10000)        return wr(2, 0xCD, n);
    else if (n < 0x100000000)    return wr(4, 0xCE, n);
    else                         return wr(8, 0xCF, n);
}
int msgpack_int(int64_t n) {
    /**/ if (n >= 0)             return msgpack_uns(n);
    else if (n >= -32)           return wr(0, n, 0); //wr(0, 0xE0 | n, 0);
    else if (n >= -128)          return wr(1, 0xD0, n + 0xff + 1);
    else if (n >= -32768)        return wr(2, 0xD1, n + 0xffff + 1);
    else if (n >= -2147483648LL) return wr(4, 0xD2, n + 0xffffffffull + 1);
    else                         return wr(8, 0xD3, n + 0xffffffffffffffffull + 1);
}
int msgpack_flt(double g) {
    float f = (float)g;
    double h = f;
    /**/ if(g == h) return wr(4, 0xCA, pack754_32(f));
    else            return wr(8, 0xCB, pack754_64(g));
}
int msgpack_str(const char *s) {
    size_t n = strlen(s), c = n;

    /**/ if (n < 0x20)     c += wr(0, 0xA0 | n, 0);
    else if (n < 0x100)    c += wr(1, 0xD9, n);
    else if (n < 0x10000)  c += wr(2, 0xDA, n);
    else                   c += wr(4, 0xDB, n);

    memcpy(out.w, s, n);
    out.w += n;
    out.len += n;
    return c;
}
int msgpack_bin(const char *s, size_t n) {
    size_t c = n;
    /**/ if (n < 0x100)    c += wr(1, 0xC4, n);
    else if (n < 0x10000)  c += wr(2, 0xC5, n);
    else                   c += wr(4, 0xC6, n);

    memcpy(out.w, s, n);
    out.w += n;
    out.len += n;
    return c;
}
int msgpack_arr(uint32_t numitems) {
    uint32_t n = numitems;
    /**/ if (n < 0x10)    return wr(0, 0x90 | n, 0);
    else if (n < 0x10000) return wr(2, 0xDC, n);
    else                  return wr(4, 0xDD, n);
}
int msgpack_map(uint32_t numpairs) {
    uint32_t n = numpairs;
    /**/ if (n < 0x10)    return wr(0, 0x80 | n, 0);
    else if (n < 0x10000) return wr(2, 0xDE, n);
    else                  return wr(4, 0xDF, n);
}
int msgpack_ext(uint8_t key, void *val, size_t n) {
    uint32_t c = n;
    /**/ if (n == 1)        c += wr(1, 0xD4, key);
    else if (n == 2)        c += wr(1, 0xD5, key);
    else if (n == 4)        c += wr(1, 0xD6, key);
    else if (n == 8)        c += wr(1, 0xD7, key);
    else if (n == 16)       c += wr(1, 0xD8, key);
    else if (n < 0x100)     c += wr(1, 0xC7, n), c += wr(0, key, 0);
    else if (n < 0x10000)   c += wr(2, 0xC8, n), c += wr(0, key, 0);
    else                    c += wr(4, 0xC9, n), c += wr(0, key, 0);

    memcpy(out.w, val, n);
    out.w += n;
    out.len += n;
    return c;
}

bool msgunpack_new( const void *opaque_or_FILE, size_t bytes ) {
    return !!((memset(&in, 0, sizeof(in)), in.memsize = bytes) ? (in.membuf = opaque_or_FILE) : (in.fp = (FILE*)opaque_or_FILE));
}
bool msgunpack_eof() {
    return in.fp ? !!feof(in.fp) : (in.offset > in.memsize);
}
bool msgunpack_err() {
    return in.fp ? !!ferror(in.fp) : !in.memsize;
}
bool msgunpack_var(struct variant *w) {
    uint8_t tag;
    struct variant v = {0};
    if( rd(&tag, 1, 0) )
    switch(tag) {
        default:
        /**/ if((tag & 0x80) == 0x00) { v.type = UNS; v.sz = 1; v.uns =         tag; }
        else if((tag & 0xe0) == 0xe0) { v.type = SIG; v.sz = 1; v.sig = (int8_t)tag; }
        else if((tag & 0xe0) == 0xa0) { v.type = rdbuf(&v.str, v.sz = tag & 0x1f) ? STR : ERR; }
        else if((tag & 0xf0) == 0x90) { v.type = ARR; v.sz = tag & 0x0f; }
        else if((tag & 0xf0) == 0x80) { v.type = MAP; v.sz = tag & 0x0f; }

        break; case 0xc0: v.type = NIL; v.sz = 0;
        break; case 0xc2: v.type = BOL; v.sz = 1; v.chr = 0;
        break; case 0xc3: v.type = BOL; v.sz = 1; v.chr = 1;

        break; case 0xcc: v.type = rd(&v.uns, v.sz = 1, 0) ? UNS : ERR;
        break; case 0xcd: v.type = rd(&v.uns, v.sz = 2, 1) ? UNS : ERR;
        break; case 0xce: v.type = rd(&v.uns, v.sz = 4, 1) ? UNS : ERR;
        break; case 0xcf: v.type = rd(&v.uns, v.sz = 8, 1) ? UNS : ERR;

        break; case 0xd0: v.type = rd(&v.uns, v.sz = 1, 0) ? (v.sig -= 0xff + 1, SIG) : ERR;
        break; case 0xd1: v.type = rd(&v.uns, v.sz = 2, 1) ? (v.sig -= 0xffff + 1, SIG) : ERR;
        break; case 0xd2: v.type = rd(&v.uns, v.sz = 4, 1) ? (v.sig -= 0xffffffffull + 1, SIG) : ERR;
        break; case 0xd3: v.type = rd(&v.uns, v.sz = 8, 1) ? (v.sig -= 0xffffffffffffffffull + 1, SIG) : ERR;

        break; case 0xca: v.type = rd(&v.u32, v.sz = 4, 1) ? (v.flt = unpack754_32(v.u32), FLT) : ERR;
        break; case 0xcb: v.type = rd(&v.uns, v.sz = 8, 1) ? (v.flt = unpack754_64(v.uns), FLT) : ERR;

        break; case 0xd9: v.type = rd(&v.sz, 1, 0) && rdbuf(&v.str, v.sz) ? STR : ERR;
        break; case 0xda: v.type = rd(&v.sz, 2, 1) && rdbuf(&v.str, v.sz) ? STR : ERR;
        break; case 0xdb: v.type = rd(&v.sz, 4, 1) && rdbuf(&v.str, v.sz) ? STR : ERR;

        break; case 0xc4: v.type = rd(&v.sz, 1, 0) && rdbuf(&v.str, v.sz) ? BIN : ERR;
        break; case 0xc5: v.type = rd(&v.sz, 2, 1) && rdbuf(&v.str, v.sz) ? BIN : ERR;
        break; case 0xc6: v.type = rd(&v.sz, 4, 1) && rdbuf(&v.str, v.sz) ? BIN : ERR;

        break; case 0xdc: v.type = rd(&v.sz, 2, 1) ? ARR : ERR;
        break; case 0xdd: v.type = rd(&v.sz, 4, 1) ? ARR : ERR;

        break; case 0xde: v.type = rd(&v.sz, 2, 1) ? MAP : ERR;
        break; case 0xdf: v.type = rd(&v.sz, 4, 1) ? MAP : ERR;

        break; case 0xd4: v.type = rd(&v.ext, 1, 0) && rd(&v.uns, 1, 0) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;
        break; case 0xd5: v.type = rd(&v.ext, 1, 0) && rd(&v.uns, 2, 1) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;
        break; case 0xd6: v.type = rd(&v.ext, 1, 0) && rd(&v.uns, 4, 1) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;
        break; case 0xd7: v.type = rd(&v.ext, 1, 0) && rd(&v.uns, 8, 1) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;
        break; case 0xd8: v.type = rd(&v.ext, 1, 0) && rd(&v.uns,16, 1) && rdbuf(&v.str, v.sz = v.uns) ? EXT : ERR;

        break; case 0xc7: v.type = rd(&v.sz, 1, 0) && rd(&v.ext, 1, 0) && rdbuf(&v.str,v.sz) ? EXT : ERR;
        break; case 0xc8: v.type = rd(&v.sz, 2, 1) && rd(&v.ext, 1, 1) && rdbuf(&v.str,v.sz) ? EXT : ERR;
        break; case 0xc9: v.type = rd(&v.sz, 4, 1) && rd(&v.ext, 1, 1) && rdbuf(&v.str,v.sz) ? EXT : ERR;
    }
    return *w = v, v.type != ERR;
}
bool msgunpack_nil() {
    return msgunpack_var(&in.v) && (in.v.type == NIL);
}
bool msgunpack_chr(bool *chr) {
    return msgunpack_var(&in.v) && (*chr = in.v.chr, in.v.type == BOL);
}
bool msgunpack_uns(uint64_t *uns) {
    return msgunpack_var(&in.v) && (*uns = in.v.uns, in.v.type == UNS);
}
bool msgunpack_int(int64_t *sig) {
    return msgunpack_var(&in.v) && (*sig = in.v.sig, in.v.type == SIG);
}
bool msgunpack_flt(float *flt) {
    return msgunpack_var(&in.v) && (*flt = in.v.flt, in.v.type == FLT);
}
bool msgunpack_dbl(double *dbl) {
    return msgunpack_var(&in.v) && (*dbl = in.v.flt, in.v.type == FLT);
}
bool msgunpack_bin(void **bin, uint64_t *len) {
    return msgunpack_var(&in.v) && (*bin = in.v.bin, *len = in.v.sz, in.v.type == BIN);
}
bool msgunpack_str(char **str) {
    return msgunpack_var(&in.v) && (str ? *str = in.v.str, in.v.type == STR : in.v.type == STR);
}
bool msgunpack_ext(uint8_t *key, void **val, uint64_t *len) {
    return msgunpack_var(&in.v) && (*key = in.v.ext, *val = in.v.bin, *len = in.v.sz, in.v.type == EXT);
}
bool msgunpack_arr(uint64_t *len) {
    return msgunpack_var(&in.v) && (*len = in.v.sz, in.v.type == ARR);
}
bool msgunpack_map(uint64_t *len) {
    return msgunpack_var(&in.v) && (*len = in.v.sz, in.v.type == MAP);
}


int msgpack(const char *fmt, ... ) {
    int count = 0;
    va_list vl;
    va_start(vl, fmt);
    while( *fmt ) {
        char f = *fmt++;
        switch( f ) {
            break; case '{': { int i = va_arg(vl, int64_t); count += msgpack_map( i ); }
            break; case '[': { int i = va_arg(vl, int64_t); count += msgpack_arr( i ); }
            break; case 'b': { bool v = !!va_arg(vl, int64_t); count += msgpack_chr(v); }
            break; case 'e': { uint8_t k = va_arg(vl, uint64_t); void *v = va_arg(vl, void*); size_t l = va_arg(vl, uint64_t); count += msgpack_ext( k, v, l ); }
            break; case 'n': { count += msgpack_nil(); }
            break; case 'p': { void *p = va_arg(vl, void*); size_t l = va_arg(vl, uint64_t); count += msgpack_bin( p, l ); }
            break; case 's': { const char *v = va_arg(vl, const char *); count += msgpack_str(v); }
            break; case 'u': { uint64_t v = va_arg(vl, uint64_t); count += msgpack_uns(v); }
            break; case 'd': case 'i': { int64_t v = va_arg(vl, int64_t); count += msgpack_int(v); }
            break; case 'f': case 'g': { double v = va_arg(vl, double); count += msgpack_flt(v); }
            default: /*count = 0;*/ break;
        }
    }
    va_end(vl);
    return count;
}
bool msgunpack(const char *fmt, ... ) {
    int count = 0;
    va_list vl;
    va_start(vl, fmt);
    while( *fmt ) {
        char f = *fmt++;
        switch( f ) {
            break; case '{': { int64_t *i = va_arg(vl, int64_t*); count += msgunpack_map( i ); }
            break; case '[': { int64_t *i = va_arg(vl, int64_t*); count += msgunpack_arr( i ); }
            break; case 'f': { float *v = va_arg(vl, float*); count += msgunpack_flt(v); }
            break; case 'g': { double *v = va_arg(vl, double*); count += msgunpack_dbl(v); }
            break; case 's': { char **v = va_arg(vl, char **); count += msgunpack_str(v); }
//          break; case 'b': { bool *v = !!va_arg(vl, bool*); count += msgunpack_chr(v); }
//          break; case 'e': { uint8_t k = va_arg(vl, uint64_t); void *v = va_arg(vl, void*); size_t l = va_arg(vl, uint64_t); count += msgunpack_ext( k, v, l ); }
//          break; case 'n': { count += msgunpack_nil(); }
//          break; case 'p': { void *p = va_arg(vl, void*); size_t l = va_arg(vl, uint64_t); count += msgunpack_bin( p, l ); }
//          break; case 'u': { uint64_t v = va_arg(vl, uint64_t); count += msgunpack_uns(v); }
//          break; case 'd': case 'i': { int64_t v = va_arg(vl, int64_t); count += msgunpack_int(v); }
            default: /*count = 0;*/ break;
        }
    }
    va_end(vl);
    return count;
}

#if 0
AUTORUN {
#   define unit(title)
#   define data(data) msgunpack_new(data, sizeof(data) -1 )
#   define TEST(expr) test(msgunpack_var(&obj) && !!(expr))

    int test_len;
    const char *test_data = 0;
    struct variant obj = {0};

    /*
     * Test vectors are derived from
     * `https://github.com/ludocode/mpack/blob/v0.8.2/test/test-write.c`.
     */

    unit("(minposfixint)");
    data("\x00");
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 0);

    unit("(maxposfixint)");
    data("\x7f");
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 127);

    unit("(maxnegfixint)");
    data("\xe0");
    TEST(obj.type == SIG && obj.sz == 1 && obj.uns == -32);

    unit("(minnegfixint)");
    data("\xff");
    TEST(obj.type == SIG && obj.sz == 1 && obj.uns == -1);

    unit("(uint8)");
    data("\xcc\0");
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 0);

    unit("(uint16)");
    data("\xcd\0\0");
    TEST(obj.type == UNS && obj.sz == 2 && obj.uns == 0);

    unit("(uint32)");
    data("\xce\0\0\0\0");
    TEST(obj.type == UNS && obj.sz == 4 && obj.uns == 0);

    unit("(uint64)");
    data("\xcf\0\0\0\0\0\0\0\0");
    TEST(obj.type == UNS && obj.sz == 8 && obj.uns == 0);

    unit("(float32)");
    data("\xca\0\0\0\0");
    TEST(obj.type == FLT && obj.sz == 4 && obj.uns == 0);

    unit("(float64)");
    data("\xcb\0\0\0\0\0\0\0\0");
    TEST(obj.type == FLT && obj.sz == 8 && obj.uns == 0);

    unit("(string)");
    data("\xa5Hello");
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));

    unit("(str8)");
    data("\xd9\x05Hello");
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));

    unit("(str16)");
    data("\xda\0\x05Hello");
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));

    unit("(str32)");
    data("\xdb\0\0\0\x05Hello");
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));

    unit("(array)");
    data("\x91\x01");
    TEST(obj.type == ARR && obj.sz == 1);

    unit("(array8)");
    data("\x91\x01");
    TEST(obj.type == ARR && obj.sz == 1);

    unit("(array16)");
    data("\xdc\0\x01\x01");
    TEST(obj.type == ARR && obj.sz == 1);

    unit("(map8)");
    data("\x81\x01\x01");
    TEST(obj.type == MAP && obj.sz == 1);
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 1);
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 1);

    unit("(map32)");
    data("\xdf\0\0\0\x01\xa5Hello\x01");
    TEST(obj.type == MAP && obj.sz == 1);
    TEST(obj.type == STR && obj.sz == 5 && !strcmp(obj.str, "Hello"));
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 1);

    unit("(+fixnum)");
    data("\x00"); TEST(obj.type == UNS && obj.uns == 0);
    data("\x01"); TEST(obj.type == UNS && obj.uns == 1);
    data("\x02"); TEST(obj.type == UNS && obj.uns == 2);
    data("\x0f"); TEST(obj.type == UNS && obj.uns == 0x0f);
    data("\x10"); TEST(obj.type == UNS && obj.uns == 0x10);
    data("\x7f"); TEST(obj.type == UNS && obj.uns == 0x7f);

    unit("(-fixnum)");
    data("\xff"); TEST(obj.type == SIG && obj.sig == -1);
    data("\xfe"); TEST(obj.type == SIG && obj.sig == -2);
    data("\xf0"); TEST(obj.type == SIG && obj.sig == -16);
    data("\xe0"); TEST(obj.type == SIG && obj.sig == -32);

    unit("(+int)");
    data("\xcc\x80"); TEST(obj.type == UNS && obj.uns == 0x80);
    data("\xcc\xff"); TEST(obj.type == UNS && obj.uns == 0xff);
    data("\xcd\x01\x00"); TEST(obj.type == UNS && obj.uns == 0x100);
    data("\xcd\xff\xff"); TEST(obj.type == UNS && obj.uns == 0xffff);
    data("\xce\x00\x01\x00\x00"); TEST(obj.type == UNS && obj.uns == 0x10000);
    data("\xce\xff\xff\xff\xff"); TEST(obj.type == UNS && obj.uns == 0xffffffffull);
    data("\xcf\x00\x00\x00\x01\x00\x00\x00\x00"); TEST(obj.type == UNS && obj.uns == 0x100000000ull);
    data("\xcf\xff\xff\xff\xff\xff\xff\xff\xff"); TEST(obj.type == UNS && obj.uns == 0xffffffffffffffffull);

    unit("(-int)");
    data("\xd0\xdf"); TEST(obj.type == SIG && obj.sig == -33);
    data("\xd0\x80"); TEST(obj.type == SIG && obj.sig == -128);
    data("\xd1\xff\x7f"); TEST(obj.type == SIG && obj.sig == -129);
    data("\xd1\x80\x00"); TEST(obj.type == SIG && obj.sig == -32768);
    data("\xd2\xff\xff\x7f\xff"); TEST(obj.type == SIG && obj.sig == -32769);
    data("\xd2\x80\x00\x00\x00"); TEST(obj.type == SIG && obj.sig == -2147483648ll);
    data("\xd3\xff\xff\xff\xff\x7f\xff\xff\xff"); TEST(obj.type == SIG && obj.sig == -2147483649ll);
    data("\xd3\x80\x00\x00\x00\x00\x00\x00\x00"); TEST(obj.type == SIG && obj.sig == INT64_MIN);

    unit("(misc)");
    data("\xc0"); TEST(obj.type == NIL && obj.chr == 0);
    data("\xc2"); TEST(obj.type == BOL && obj.chr == 0);
    data("\xc3"); TEST(obj.type == BOL && obj.chr == 1);

    data("\x90"); TEST(obj.type == ARR && obj.sz == 0);

    data("\x91\xc0");
    TEST(obj.type==ARR && obj.sz==1);
    TEST(obj.type==NIL);

    data("\x9f\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e");
    TEST(obj.type==ARR && obj.sz==15);
    for(int i = 0; i < 15; ++i) {
        TEST(obj.type==UNS && obj.sig==i);
    }

    data("\xdc\x00\x10\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c"
         "\x0d\x0e\x0f");
    TEST(obj.type==ARR && obj.sz==16);
    for(unsigned i = 0; i < 16; ++i) {
        TEST(obj.type == UNS && obj.uns == i);
    }

    data("\x80");
    TEST(obj.type == MAP && obj.sz == 0);

    data("\x81\xc0\xc0");
    TEST(obj.type == MAP && obj.sz == 1);
    TEST(obj.type == NIL);
    TEST(obj.type == NIL);

    data("\x82\x00\x00\x01\x01");
    TEST(obj.type == MAP && obj.sz == 2);
    TEST(obj.type == UNS && obj.sig == 0);
    TEST(obj.type == UNS && obj.sig == 0);
    TEST(obj.type == UNS && obj.sig == 1);
    TEST(obj.type == UNS && obj.sig == 1);

    data("\x8f\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e"
         "\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d");
    TEST(obj.type == MAP && obj.sz == 15);
    for(unsigned i = 0; i < 15; ++i) {
        TEST(obj.type == UNS && obj.uns == i*2+0);
        TEST(obj.type == UNS && obj.uns == i*2+1);
    }

    data("\xde\x00\x10\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c"
         "\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c"
         "\x1d\x1e\x1f");
    TEST(obj.type == MAP && obj.sz == 16);
    for(unsigned i = 0; i < 16; ++i) {
        TEST(obj.type == UNS && obj.uns == i*2+0);
        TEST(obj.type == UNS && obj.uns == i*2+1);
    }

    data("\x91\x90");
    test( obj.type == ARR && obj.sz == 1 );
    test( obj.type == ARR && obj.sz == 0 );

    data("\x93\x90\x91\x00\x92\x01\x02");
    test( obj.type == ARR && obj.sz == 3 );
        test( obj.type == ARR && obj.sz == 0 );
        test( obj.type == ARR && obj.sz == 1 );
            test( obj.type == UNS && obj.uns == 0 );
        test( obj.type == ARR && obj.sz == 2 );
            test( obj.type == UNS && obj.uns == 1 );
            test( obj.type == UNS && obj.uns == 2 );

    data("\x95\x90\x91\xc0\x92\x90\x91\xc0\x9f\x00\x01\x02\x03\x04\x05\x06"
         "\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\xdc\x00\x10\x00\x01\x02\x03\x04"
         "\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f");
    test( obj.type == ARR && obj.sz == 5 );
        test( obj.type == ARR && obj.sz == 0 );
        test( obj.type == ARR && obj.sz == 1 );
            test( obj.type == NIL );
        test( obj.type == ARR && obj.sz == 2 );
            test( obj.type == ARR && obj.sz == 0 );
            test( obj.type == ARR && obj.sz == 1 );
                test( obj.type == NIL );
        test( obj.type == ARR && obj.sz == 15 );
            for( unsigned i = 0; i < 15; ++i ) {
                test( obj.type == UNS && obj.uns == i );
            }
        test( obj.type == ARR && obj.sz == 16 );
            for( unsigned i = 0; i < 15; ++i ) {
                test( obj.type == UNS && obj.uns == i );
            }

    data("\x85\x00\x80\x01\x81\x00\xc0\x02\x82\x00\x80\x01\x81\xc0\xc0\x03"
         "\x8f\x00\x00\x01\x01\x02\x02\x03\x03\x04\x04\x05\x05\x06\x06\x07"
         "\x07\x08\x08\x09\x09\x0a\x0a\x0b\x0b\x0c\x0c\x0d\x0d\x0e\x0e\x04"
         "\xde\x00\x10\x00\x00\x01\x01\x02\x02\x03\x03\x04\x04\x05\x05\x06"
         "\x06\x07\x07\x08\x08\x09\x09\x0a\x0a\x0b\x0b\x0c\x0c\x0d\x0d\x0e"
         "\x0e\x0f\x0f");
    TEST(obj.type == MAP && obj.sz == 5);
        TEST(obj.type == UNS && obj.uns == 0);
        TEST(obj.type == MAP && obj.sz == 0);
        TEST(obj.type == UNS && obj.uns == 1);
        TEST(obj.type == MAP && obj.sz == 1);
            TEST(obj.type == UNS && obj.uns == 0);
            TEST(obj.type == NIL);
        TEST(obj.type == UNS && obj.uns == 2);
        TEST(obj.type == MAP && obj.sz == 2);
            TEST(obj.type == UNS && obj.uns == 0);
            TEST(obj.type == MAP && obj.sz == 0);
            TEST(obj.type == UNS && obj.uns == 1);
            TEST(obj.type == MAP && obj.sz == 1);
                TEST(obj.type == NIL);
                TEST(obj.type == NIL);
        TEST(obj.type == UNS && obj.uns == 3);
        TEST(obj.type == MAP && obj.sz == 15);
            for( unsigned i = 0; i < 15; ++i ) {
                TEST(obj.type == UNS && obj.uns == i);
                TEST(obj.type == UNS && obj.uns == i);
            }
        TEST(obj.type == UNS && obj.uns == 4);
        TEST(obj.type == MAP && obj.sz == 16);
            for( unsigned i = 0; i < 16; ++i ) {
                TEST(obj.type == UNS && obj.uns == i);
                TEST(obj.type == UNS && obj.uns == i);
            }

    data("\x85\xd0\xd1\x91\xc0\x90\x81\xc0\x00\xc0\x82\xc0\x90\x04\x05\xa5"
         "\x68\x65\x6c\x6c\x6f\x93\xa7\x62\x6f\x6e\x6a\x6f\x75\x72\xc0\xff"
         "\x91\x5c\xcd\x01\x5e");
    TEST(obj.type == MAP && obj.sz == 5);
        TEST(obj.type == SIG && obj.sig == -47);
        TEST(obj.type == ARR && obj.sz == 1);
            TEST(obj.type == NIL);
        TEST(obj.type == ARR && obj.sz == 0);
        TEST(obj.type == MAP && obj.sz == 1);
            TEST(obj.type == NIL);
            TEST(obj.type == UNS && obj.uns == 0);
        TEST(obj.type == NIL);
        TEST(obj.type == MAP && obj.sz == 2);
            TEST(obj.type == NIL);
            TEST(obj.type == ARR && obj.sz == 0);
            TEST(obj.type == UNS && obj.uns == 4);
            TEST(obj.type == UNS && obj.uns == 5);
        TEST(obj.type == STR && !strcmp(obj.str, "hello"));
        TEST(obj.type == ARR && obj.sz == 3);
            TEST(obj.type == STR && !strcmp(obj.str, "bonjour"));
            TEST(obj.type == NIL);
            TEST(obj.type == SIG && obj.sig == -1);
        TEST(obj.type == ARR && obj.sz == 1);
            TEST(obj.type == UNS && obj.uns == 92);
        TEST(obj.type == UNS && obj.uns == 350);

    data("\x82\xa7" "compact" "\xc3\xa6" "schema" "\x00");
    TEST(obj.type == MAP && obj.sz == 2);
    TEST(obj.type == STR && obj.sz == 7 && !strcmp(obj.str, "compact"));
    TEST(obj.type == BOL && obj.chr == 1);
    TEST(obj.type == STR && obj.sz == 6 && !strcmp(obj.str, "schema"));
    TEST(obj.type == UNS && obj.sz == 1 && obj.uns == 0);

#   undef TEST
#   undef data
#   undef unit
}

bool vardump( struct variant *w ) {
    static int tabs = 0;
    struct variant v = *w;
    printf("%.*s", tabs, "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
    switch( v.type ) {
    default: case ERR:
         if( !msgunpack_eof() ) printf("ERROR: unknown tag type (%02X)\n", (int)v.type);
         return false;
    break; case NIL: printf("(%s)\n", "null");
    break; case BOL: printf("bool: %d\n", v.chr);
    break; case SIG: printf("int: %lld\n", v.sig);
    break; case UNS: printf("uint: %llu\n", v.uns);
    break; case FLT: printf("float: %g\n", v.flt);
    break; case STR: printf("string: '%s'\n", v.str);
    break; case BIN: { for( size_t n = 0; n < v.sz; n++ ) printf("%s%02x(%c)", n > 0 ? " ":"binary: ", v.str[n], v.str[n] >= 32 ? v.str[n] : '.'); puts(""); }
    break; case EXT: { printf("ext: [%02X (%d)] ", v.ext, v.ext); for( size_t n = 0; n < v.sz; n++ ) printf("%s%02x(%c)", n > 0 ? " ":"", v.str[n], v.str[n] >= 32 ? v.str[n] : '.'); puts(""); }
    break; case ARR: {
        ++tabs; puts("[");
        for( size_t n = v.sz; n-- > 0; ) {
            if( !msgunpack_var(&v) || !vardump(&v) ) return false;
        }
        --tabs; puts("]");
    }
    break; case MAP: {
        ++tabs; puts("{");
        for( size_t n = v.sz; n-- > 0; ) {
            if( !msgunpack_var(&v) || !vardump(&v) ) return false;
            if( !msgunpack_var(&v) || !vardump(&v) ) return false;
        }
        --tabs; puts("}");
    }}
    return true;
}

void testdump( const char *fname ) {
    FILE *fp = fopen(fname, "rb");
    if( !fp ) {
        fputs("Cannot read input stream", stderr);
    } else {
        if( msgunpack_new(fp, 0) ) {
            struct variant v;
            while( msgunpack_var(&v) ) {
                vardump(&v);
            }
            if( msgunpack_err() ) {
                fputs("Error while unpacking", stderr);
            }
        }
        fclose(fp);
    }
}

void testwrite(const char *outfile) {
    char buf[256];
    msgpack_new(buf, 256);
    int len = msgpack("ddufs [dddddddd-dddddddd {sisi bne"/*bp0*/,
        -123LL, 123LL, 123456ULL, 3.14159f, "hello world",
        16ULL,
         -31LL, -32LL, -127LL, -128LL, -255LL, -256LL, -511LL, -512LL,  // ,121, 3, "hi",
         +31LL, +32LL, +127LL, +128LL, +255LL, +256LL, +511LL, +512LL,  // ,121, 3, "hi",
        2ULL,
          "hello", -123LL,
          "world", -456LL,
        1ULL,
        0xeeULL, "this is an EXT type", sizeof("this is an EXT type")-1
    );
    hexdump(buf, len);

    FILE *fp = fopen(outfile, "wb");
    if( fp ) {
        fwrite( buf, len, 1, fp );
        fclose(fp);
    }
}

AUTORUN {
    testwrite("out.mp");
    testdump("out.mp");
}
#endif
