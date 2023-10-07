// -----------------------------------------------------------------------------
// semantic versioning in a single byte (octal)
// - rlyeh, public domain.
    //
// - single octal byte that represents semantic versioning (major.minor.patch).
// - allowed range [0000..0377] ( <-> [0..255] decimal )
// - comparison checks only major.minor tuple as per convention.

int semver( int major, int minor, int patch ) {
    return SEMVER(major, minor, patch);
}
int semvercmp( int v1, int v2 ) {
    return SEMVERCMP(v1, v2);
}

#if 0
AUTORUN {
    for( int i= 0; i <= 255; ++i) printf(SEMVERFMT ",", i);
    puts("");

    printf(SEMVERFMT "\n", semver(3,7,7));
    printf(SEMVERFMT "\n", semver(2,7,7));
    printf(SEMVERFMT "\n", semver(1,7,7));
    printf(SEMVERFMT "\n", semver(0,7,7));

    printf(SEMVERFMT "\n", semver(3,7,1));
    printf(SEMVERFMT "\n", semver(2,5,3));
    printf(SEMVERFMT "\n", semver(1,3,5));
    printf(SEMVERFMT "\n", semver(0,1,7));

    assert( semvercmp( 0357, 0300 )  > 0 );
    assert( semvercmp( 0277, 0300 )  < 0 );
    assert( semvercmp( 0277, 0200 )  > 0 );
    assert( semvercmp( 0277, 0100 )  < 0 );
    assert( semvercmp( 0076, 0070 ) == 0 );
    assert( semvercmp( 0076, 0077 ) == 0 );
    assert( semvercmp( 0176, 0170 ) == 0 );
    assert( semvercmp( 0176, 0177 ) == 0 );
    assert( semvercmp( 0276, 0270 ) == 0 );
    assert( semvercmp( 0276, 0277 ) == 0 );
    assert( semvercmp( 0376, 0370 ) == 0 );
    assert( semvercmp( 0376, 0377 ) == 0 );
}
#endif
