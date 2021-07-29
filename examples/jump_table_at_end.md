# jump_table_at_end.6502

This is a simple example showing how to align a jump table or parameter
block to appear just _before_ a page boundary, so you can have something
like this:
```
.wibble
     7BF7   4C 00 7B   JMP real_wibble
.blah
     7BFA   4C 07 7B   JMP real_blah
.sayXY
     7BFD   4C 0B 7B   JMP real_sayXY
```
This is enormously useful when you have a BASIC program which is `CALL`ing
machine code routines; if you follow this example then you should never
have to change the entry points used by your BASIC program, no matter how
the machine code grows.  The only changes you will need to make will be to
add any new entry points along with the BASIC that is going to make use of
them; and to alter `HIMEM` as your machine code grows  (otherwise, the
BASIC `PROC` stack will stomp on your code).

The trick is this:  We create a label pointing to the first free location
after the code that has been assembled so far.  Then we ALIGN &100 to
advance the origin to the next page boundary.  We use CLEAR to tell BeebAsm
that the area from where the aforementioned label points to the current
origin is safe to use; and lastly, we adjust the origin backwards by the
exact size of the code or data we are going to insert.

This is the actual code from the example, with its nine-byte jump table:
```
._real_end              \  This is the first location after end of code

ALIGN &100              \  Now P% is on a page boundary

GUARD P%                \  Prevent assembling any more code after here

CLEAR _real_end, P%     \  Mark the area from the real end of code as far
                        \   as the new origin, as available for use again

ORG P% - 9              \  Wind the origin back 3 bytes for each entry in
                        \   the jump table, to &xxF7
                        \  (YOU WILL NEED TO EDIT THIS TO SUIT YOUR CODE!)
```
The value 9 will need to be edited to match the size of the fixed part of
the code in your own use case.  You can just decrease the origin, if the
code grows too big.  
