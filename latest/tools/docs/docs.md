|Version:       | {{VERSION}} |
|:--------------|:------------|
|Revision:      | {{CHANGELIST}} |
|Branch:        | {{BRANCH}} |
<!--| Documentation last modified | { {LAST_MODIFIED} } |-->

# [V·4·K {{VERSION}}](https://v4k.dev)
## a b o u t

- Envision, Prototype, Perfect.
- visit the code repo at [v4k-mirror](https://github.com/zpl-zak/v4k-mirror) now!
- you can try out the [live demo](https://demo.v4k.dev), too!
- and for anything else, reach us at [v4.games](https://v4.games/).

<br/>
<details><summary>Code style</summary>
```C linenumbers
/// ## Markdown comments when documenting (3 slashes)
// C++ comments allowed /*C comments too*/
// Order matters: includes -> defines -> enums -> structs -> functions
#define puts(x) my_printf("%s", x)   // lowercase defines allowed for syntax sugars
#define VERSION "1.0.0"              // uppercase defines otherwise
enum { ZERO = 0 };                   // uppercase enums. also, one-line brackets allowed
void assert_positive(int my_int) { // lowercase snake_case everywhere
    int *x = &my_int;                // no spacing between pointers and variables
    if (*x < ZERO) {                 // no outer padding space after if,do,while,for,switch
        puts( "Negative" );          // inner padding space around operators and parenthesis
    }                                // 4-spaces indents, 1TBS brackets
}                                    // when in doubt, dont worry & mimic style from codebase
```
</details>

<!--
!!! Note
    Ready to browse documentation? This is a very common note.

!!! Tip
    Then we have these informational notes. Tips mostly.

!!! WARNING
    And warning notes. You should read them definitely.

!!! ERROR: Watch out
    Really **important notes**. Beware of these.
-->

## r e a d m e
