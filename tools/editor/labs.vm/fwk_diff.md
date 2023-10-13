given a string A, we want it to be B.

A: hello world and thanks for the fish.
B: hello cruel o_o world and thanks for the fish!

however, the instructions to reconstruct B must be as small as possible, to minimize transmission costs. this is why we dont send B entirely.

different algorithms as follow:

## ALGORITHM 1

- identify the first mismatching character (S).

         v-- 6(S)
A: hello world and thanks for the fish.
B: hello cruel o_o world and thanks for the fish!

- identify the last mismatch character (E).

                                                v-- 0(E)
A:           hello world and thanks for the fish.
B: hello cruel o_o world and thanks for the fish!

- we construct the patch now with 3 numbers:
  - number of bytes to copy from beginning(A) till S(A)
  - number of bytes to copy from S(B) till E(B). plus the substring that is get copied.
  - number of bytes to copy from E(A) till end of string.

6 40 "cruel o_o world and thanks for the fish!" 0

- total patch size is 3 control bytes + 40 (string) = 43 bytes

## ALGORITHM 2

- We delta every character in both strings, from beginning to end, and from end to beginning.

A: hello world and thanks for the fish.0000000000
B: hello cruel o_o world and thanks for the fish!
C: 000000XXXXX0XXX0XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX (-)

A: 0000000000hello world and thanks for the fish.
B: hello cruel o_o world and thanks for the fish!
C: XXXXXXXXXXXXXX0000000000000000000000000000000X (-)

- Pick the option with most zeros (2nd). Aka, select the option with less Xs.

do { Encode every XXXX island in C as a positive operation indicating how many bytes to copy from.
     Run-length the number of zeros into a negative operation.
   } repeat till lane is exhausted.

  XXXXXXXXXXXXXXX: +14 "hello cruel o_"
                 0000000000000000000000000000000: -29
                                                X: +1 "!"

 - Patch size is 3 control bytes + 14 (string) + 1 (string) = 18 bytes

## ALGORITHM 3

- start source (A) and target (B) strings. start with A:
- select how many bytes to copy (positive), how many to skip (negative), or switch A<-->B lanes (zero).
- repeat previous step over and over until both lanes do reach their respective string ends.

A: hello world and thanks for the fish.
B: hello cruel o_o world and thanks for the fish!

A: +6 0     >> hello
B: -6 +10 0 >> hello cruel o_o
A: +24 -1 0 >> hello cruel o_o world and thanks for the fish
B: -29 +1   >> hello cruel o_o world and thanks for the fish!
(10 bytes)

what if we always specify a dual +/- operation per lane?
lanes changed automatically after every tuple. in this case a 0 shall indicate a no op.

A: +6 0   >> hello
B: -6 +10 >> hello cruel o_o
A: +24 -1 >> hello cruel o_o world and thanks for the fish
B: -29 +1 >> hello cruel o_o world and thanks for the fish!
(8 bytes)

- we construct the patch now. a substring excerpt must come after any positive operations on lane B.

+6 0 -6 +10 "cruel o_o " +24 -1 -29 +1 "!"

- total patch size is 8 control bytes + 10 (string) + 1 (string) = 19 bytes

