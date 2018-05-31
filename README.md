# BeebAsm
**Version V1.10-pre**

A portable 6502 assembler with BBC Micro style syntax

Copyright (C) Rich Talbot-Watkins 2007-2012
<richtw1@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.   

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.   

You should have received a copy of the GNU General Public License
along with this program, as COPYING.txt.  If not, see   
http://www.gnu.org/licenses


## CONTENTS

1. [ABOUT BEEBASM](#1.-ABOUT-BEEBASM)
2. [BEEBASM 'PHILOSOPHY'](#2.-BEEBASM-'PHILOSOPHY')
3. [EXAMPLE](#3.-EXAMPLE)
4. [COMMAND LINE OPTIONS](#4.-COMMAND-LINE-OPTIONS)
5. [SOURCE FILE SYNTAX](#5.-SOURCE-FILE-SYNTAX)
6. [ASSEMBLER DIRECTIVES](#6.-ASSEMBLER-DIRECTIVES)
7. [TIPS AND TRICKS](#7.-TIPS-AND-TRICKS)
8. [DEMO](#8.-DEMO)
9. [VERSION HISTORY](#9.-VERSION-HISTORY)
10. [REPORTING BUGS](#10.-REPORTING-BUGS)



 ## 1. ABOUT BEEBASM

BeebAsm is a 6502 assembler designed specially for developing assembler programs for the BBC Micro.  It uses syntax reminiscent of BBC BASIC's built-in assembler, and is able to output its object code directly into emulator-ready DFS disc images.

Many of the luxuries which come from assembling within the BBC BASIC environment on a real BBC Micro are also available here, including FOR...NEXT loops, conditional assembly (IF...ELSE...ENDIF), and all of BASIC's numerical functions, including SIN, COS and SQR - very useful for building lookup tables directly within a source file.

BeebAsm is distributed with source code, and should be easily portable to any platform you wish.  To build under Windows, you will need to install MinGW (http://www.mingw.org), and the most basic subset of Cygwin (http://www.cygwin.org) which provides Windows versions of the common Unix commands.  Ensure the executables from these two packages are in your
Windows path, and BeebAsm should compile without problems.  Just navigate to the directory containing 'Makefile', and enter 'make code'.




## 2. BEEBASM 'PHILOSOPHY'

BeebAsm is not like most modern assemblers, in that it doesn't just accept a source file, and output the corresponding object code file - after all, what use is a raw 6502 executable file on a PC, outside of an emulated BBC Micro environment?

Although BeebAsm *is* able to do this, this isn't the way it was intended to be used.  Instead, BeebAsm can be pointed at a BBC Micro DFS disc image (.ssd or .dsd file), and can save blocks of assembled object code directly onto the 'disc', as many or as few as you wish.  It is up to the source code to specify which blocks of assembled code to save, and with which name, just as if you were assembling from within BBC BASIC itself.

## 3. EXAMPLE

Rather than trying to explain anything about BeebAsm now, let's leap straight into an example, as it can probably illustrate more about how BeebAsm should be used than a thousand lines of text.

Take the following highly contrived source file: `simple.asm`

-------------------------------------------------------------------------------
```
\ Simple example illustrating use of BeebAsm

oswrch = &FFEE
osasci = &FFE3
addr = &70

ORG &2000         ; code origin (like P%=&2000)

.start
    LDA #22:JSR oswrch
    LDA #7:JSR oswrch
    LDA #mytext MOD 256:STA addr
    LDA #mytext DIV 256:STA addr+1
    LDY #0
.loop
    LDA (addr),Y
    BEQ finished
    JSR osasci
    INY
    BNE loop
.finished
    RTS

.mytext EQUS "Hello world!", 13, 0
.end

SAVE "MyCode", start, end
```
-------------------------------------------------------------------------------

...and then build it with the following command:

`beebasm -i simple.asm -do test.ssd -boot MyCode -v`

This will do the following:

* create a new disc image called test.ssd, set to *OPT 4,3
* assemble the 6502 code and create an executable on the disc called 'MyCode'
* create a !Boot file containing '*RUN MyCode'
* output a listing of the assembled code

Note how the syntax in the source file is very much like BBC BASIC, with a few small differences which we'll look at in detail later.

Note also that the source code tells the assembler what should be saved - in this example, all of the assembled code (from .start to .end), with the filename 'MyCode'.  This might at first seem strange, but it's actually very simple and powerful: you have absolute control over what gets saved.  If we wished, we could assemble more code elsewhere and save it as a separate file, whilst all the defined labels remained visible to both chunks of code.

## 4. COMMAND LINE OPTIONS

At its very most basic, you need know only one command line option:

`-i <filename>`

This specifies the name of the source file for BeebAsm to process.  In the absence of any switches specifying disc image filenames, SAVE commands in the source code will write out object files directly to the current directory.

`-o <filename>`

If this is specified, then the SAVE command can be used without supplying a filename, and this one will be used instead.  This allows BeebAsm to be used like a conventional assembler, specifying both input and output filenames from the command line.

`-do <filename>`

This specifies the name of a new disc image to be created.  All object code files will be saved to within this disc image.

`-boot <DFS filename>`

If specifed, BeebAsm will create a !Boot file on the new disc image, containing the command `*RUN <DFS filename>`.  The new disc image will already be set to `*OPT 4,3` (`*EXEC !Boot`).

`-opt <value>`

If specified, this sets the disc option of the generated disc image (i.e. the `*OPT 4,n` value) to the value specified.  This overrides the -boot option.

`-title <title>`

If specified, this sets the disc title of the generated disc image (i.e. the string set by `*TITLE`) to the value specified.

`-di <filename>`

If specified, BeebAsm will use this disc image as a template for the new disc image, rather than creating a new blank one.  This is useful if you have a BASIC loader which you want to run before your executable.  Note this cannot be the same as the `-do` filename!

`-v`

Verbose output.  Assembled code will be output to the screen.

`-vc`

Use Visual C++-style error messages.

`-d`

Dumps all global symbols in Swift-compatible format after assembly. This is used internally by Swift, and is just documented for completeness.

`-w`

If specified, there must be whitespace between opcodes and their labels. This introduces an incompatibility with the BBC BASIC assembler, which allows things like `ck_axy=&70:stack_axy` (i.e. `STA &70`), but makes it possible for macros to have names which begin with an opcode name, e.g.:

```
    MACRO stack_axy
        PHA:TXA:PHA:TYA:PHA
    ENDMACRO
```

Things like `STA&4000` are permitted with or without `-w`.

`-D <symbol> `

`-D <symbol>=<value>`

Define `<symbol>` before starting to assemble. If `<value>` is not given, `-1` (`TRUE`)
will be used by default. Note that there must be a space between `-D` and the symbol. `<value>` may be in decimal, hexadecimal (prefixed with $, & or 0x) or binary (prefixed with %).

`-D` can be used in conjunction with conditional assignment to provide default values within the source which can be overridden from the command line.

## 5. SOURCE FILE SYNTAX

Assembler instructions are written with the standard 6502 syntax.

A label is defined by preceding it with a `"."`, as per the BBC Micro assembler, e.g.  `.loop`

Instructions can be written one-per-line, or many on one line, separated by colons.  A label need not be followed by a colon.

Comments are introduced by a semicolon or backslash.  Unlike the BBC Micro assembler, these continue to the end of the line, and are not terminated by a colon (because this BBC Micro feature is horrible!).

Numeric literals are in decimal by default, and can be integers or reals. Hex literals are prefixed with `"&"`. A character in single quotes (e.g. `'A'`) returns its ASCII code.

BeebAsm can accept complex expressions, using a wide variety of operators and functions.  Here's a summary:

```
+ - * /            Addition, subtraction, multiplication, division.
<<                 Arithmetic shift left; same precedence as multiplication
>>                 Arithmetic shift right; same precedence as division
^                  Raise to the power of.
() or []           Bracketed expression.  Use [] to avoid confusion with
                   6502 indirect instructions.
= or ==            Test equality.  Returns 0 or -1.
<> or !=           Test non-equality.  Returns 0 or -1.
< > <= >=          Other comparisons.  Returns 0 or -1.
AND                Bitwise AND.
OR                 Bitwise OR.
EOR                Bitwise EOR.
DIV                Integer division.
MOD                Integer modulus.

LO(val) or <val    Return lsb of 16-bit expression (like 'val MOD 256')
HI(val) or >val    Return msb of 16-bit expression (like 'val DIV 256')
-                  Negate (unary minus)
SQR(val)           Return square root of val
SIN(val)           Return sine of val
COS(val)           Return cosine of val
TAN(val)           Return tangent of val
ASN(val)           Return arc-sine of val
ACS(val)           Return arc-cosine of val
ATN(val)           Return arc-tangent of val
RAD(val)           Convert degrees to radians
DEG(val)           Convert radians to degrees
INT(val)           Round to integer (towards zero)
ABS(val)           Take the absolute value
SGN(val)           Return -1, 0 or 1, depending on the sign of the argument
RND(val)           RND(1) returns a random number between 0 and 1
                   RND(n) returns an integer between 0 and n-1
NOT(val)           Return the bitwise 1's complement of val
LOG(val)           Return the base 10 log of val
LN(val)            Return the natural log of val
EXP(val)           Return e raised to the power of val
```

Also, some constants are defined:

```
PI                 The value of PI (3.1415927...)
FALSE              Returns 0
TRUE               Returns -1
* or P%            A special symbol which returns the current address being
                   assembled at.
CPU                The value set by the CPU assembler directive (see below)
```

Within `EQUB/EQUS` only you can also use the expressions:

```
TIME$              Return assembly date/time in format "Day,DD Mon Year.HH:MM:SS"

TIME$("fmt")       Return assembly date/time in a format determined by "fmt", which
                   is the same format used by the C library strftime().
```

The assembly date/time is constant throughout the assembly; every use of `TIME$`
will return the same date/time.

Variables can be defined at any point using the BASIC syntax, i.e. `addr = &70`.

Note that it is not possible to reassign variables once defined. However `FOR...NEXT` blocks have their own scope (more on this later).

Variables can be defined if they are not already defined using the conditional assignment syntax `=?`, e.g. `addr =? &70`. This is useful in conjunction with the `-D` command line option to provide default values for variables in the source while allowing them to be overridden on the command line. (Because variables cannot be reassigned once defined, it is not possible to define a variable with `-D` *and* with non-conditional assignment.)

(Thanks to Stephen Harris <sweh@spuddy.org> and "ctr" for the `-D`/conditional assignment support.)


## 6. ASSEMBLER DIRECTIVES

These are keywords which control the assembly of the source file. 

Here's a summary:

`ORG <addr>`

Set the address to be assembled from.  This can be changed multiple times   during a source file if you wish (for example) to assemble two separate blocks of code at different addresses, but share the labels between both   blocks.  This is exactly equivalent to BBC BASIC's `P%=<addr>`.


`CPU <n>`

Selects the target CPU, which determines the range of instructions that will be accepted. The default is `0`, which provides the original 6502 instruction set. The only current alternative is 1, which provides the 65C02 instruction set (including `PLX`, `TRB` etc, but not the Rockwell  additions like `BBR`).


`SKIP <bytes>`

Moves the address pointer on by the specified number of bytes.  Use this to reserve a space of a fixed size in the code.


`SKIPTO <addr>`

Moves the address pointer to the specified address.  An error is generated if this address is behind the current address pointer.


`ALIGN <alignment>`

Used to align the address pointer to the next boundary, e.g. use `ALIGN &100` to move to the next page (useful perhaps for positioning a table at a page boundary so that index accesses don't incur a "page crossed" penalty.


`COPYBLOCK <start>,<end>,<dest>`

Copies a block of assembled data from one location to another.  This is useful to copy code assembled at one location into a program's data area for relocation at run-time.


`INCLUDE "filename"`

Includes the specified source file in the code at this point.


`INCBIN "filename"`

Includes the specified binary file in the object code at this point.


`EQUB a [, b, c, ...]`

Insert the specified byte(s) into the code.  Note, unlike BBC BASIC, that a comma-separated sequence can be inserted.


`EQUW a [, b, c, ...]`

Insert the specified 16-bit word(s) into the code.


`EQUD a [, b, c, ...]`

Insert the specified 32-bit word(s) into the code.


`EQUS "string" [, "string", byte, ...]`

Inserts the specified string into the code.  Note that this can take a comma-separated list of parameters which may also include bytes.  So, to zero-terminate a string, you can write:

```
EQUS "My string", 0
```

In fact, under the surface, there is no difference between `EQUS` and `EQUB`, which is also able to take strings!


`MAPCHAR <ascii code>, <remapped code>`

`MAPCHAR <start ascii code>, <end ascii code>, <remapped code>`

By default, when `EQUS "string"` is assembled, the ASCII codes of each character are written into the object code.  `MAPCHAR` allows you to specify which value should be written to the object code for each character.

Suppose you have a font which contains the following symbols - `space`, followed by `A-Z`, followed by digits `0-9`, followed by `.,!?-'`

You could specify this with `MAPCHAR` as follows:

```
MAPCHAR ' ', 0
MAPCHAR 'A','Z', 1
MAPCHAR '0','9', 27
MAPCHAR '.', 37
MAPCHAR ',', 38
MAPCHAR '!', 39
MAPCHAR '?', 40
MAPCHAR '-', 41
MAPCHAR ''', 42
```
Now, when writing strings with `EQUS`, these codes will be written out instead of the default ASCII codes.


`GUARD <addr>`

Puts a 'guard' on the specified address which will cause an error if you attempt to assemble code over this address.


`CLEAR <start>, <end>`

Clears all guards between the `<start>` and `<end`> addresses specified.  This can also be used to reset a section of memory which has had code assembled in it previously.  BeebAsm will complain if you attempt to assemble code over previously assembled code at the same address without having `CLEAR`ed it first.


`SAVE "filename", start, end [, exec [, reload] ]`

Saves out object code to either a DFS disc image (if one has been specified), or to the current directory as a standalone file.  A source file must have at least one SAVE statement in it, otherwise nothing will be output.  BeebAsm will warn if this is the case.

`'exec'` can be optionally specified as the execution address of the file when saved to a disc image.

`'reload'` can additionally be specified to save the file on the disc image to a different address to that which it was saved from.  Use this to assemble code at its 'native' address,  but which loads at a DFS-friendly address, ready to be relocated to its correct address upon execution.


`PRINT`

Displays some text.  `PRINT` takes a comma-separated list of strings or values. 

To print a value in hex, prefix the expression with a `'~'` character.

Examples:
```
PRINT "Value of label 'start' =", ~start
PRINT "numdots =", numdots, "dottable size =", dotend-dotstart
```
			
`ERROR "message"`

Causes BeebAsm to abort assembly with the provided error message.  This can be useful for enforcing certain constraints on generated code, for example:
```
  .table
    FOR n, 1, 32
      EQUB 255 / n
    NEXT
  
IF HI(P%)<>HI(table)
    ERROR "Table crosses page boundary"
ENDIF
```

`FOR <var>, start, end [, step] ... NEXT`

I wanted this to have exactly the same syntax as BASIC, but I couldn't without rewriting my expression parser, so we're stuck with this for now.

It works exactly like BASIC's `FOR...NEXT`.  For example:
```
FOR n, 0, 10, 2    ; loop with n = 0, 2, 4, 6, 8, 10
  PRINT n
  LDA #0:STA &900+n
  LDA #n:STA &901+n
NEXT
```

The variable n only exists for the scope of the `FOR...NEXT` loop. Also, any labels or variables defined within the loop are only visible within it.  However, unlike BBC BASIC, forward references to labels inside the loop will work properly, so, for example, this little multiply routine is perfectly ok:
```
.multiply
\\ multiplies A*X, puts result in product/product+1
CPX #0:BEQ zero
DEX:STX product+1
LSR A:STA product:LDA #0
FOR n, 0, 7
  BCC skip:ADC product+1:.skip   \\ would break BBC BASIC!
  ROR A:ROR product
NEXT
STA product+1:RTS
.zero
STX product:STX product+1:RTS
```

`IF...ELIF...ELSE...ENDIF`

Use to assemble conditionally.  Like anything else in BeebAsm, these statements can be placed on one line, separated by colons, but even if they are, `ENDIF` must be present to denote the end of the `IF` block (unlike BBC BASIC).

Examples of use:
```
\\ build a rather strange table
FOR n, 0, 9
  IF (n AND 1) = 0
    a = n*n
  ELSE
    a = -n*n
  ENDIF
  EQUB a
NEXT

IF debugraster:LDA #3:STA &FE21:ENDIF

IF rom
  ORG &8000
  GUARD &C000
ELIF tube
  ORG &B800
  GUARD &F800
ELSE
  ORG &3C00
  GUARD &7C00
ENDIF
```

`{ ... }`

Curly braces can be used to specify a block of code in which all symbols and labels defined will exist only within this block.  Effectively, this is a mechanism for providing 'local labels' without the slightly cumbersome syntax demanded by some other assemblers.  These can be nested.  Any symbols defined within a block will override any of the same name outside of the block, exactly like C/C++ - not sure if I like this behaviour, but for now it will stay.

Example of use:
```
.initialise
{
    LDY #31
    LDA #0
.loop              ; label visible only within the braces
    STA buffer,Y
    DEY
    BPL loop
    RTS
}

.copy
{
    LDY #31
.loop              ; perfectly ok to define .loop again in a new block
    LDA source,Y
    STA dest,Y
    DEY
    BPL loop
    RTS
}
```
Labels can be defined within a scope which exist outside that scope in two ways.

A label defined using `'.*label'` will be globally visible:
```
.copy_10
{
    LDX #10
.*copy_x
    DEX
.loop
    LDA from,X
    STA to,X
    DEX
    BPL loop
    RTS
}

JSR copy_10
LDX #20
JSR copy_x
```
A label defined using `'.^label'` will be visible in the parent scope:
```
.contrived
{
    LDX #255
    {
        LDY #3
    .^loop
        DEX
        BEQ exit
        DEY
        BNE loop
    }
    LDY #7
    JMP loop ; .loop *is* visible here
.exit
    RTS
}
JMP loop ; error, .loop is *not* visible here
```
For a more realistic example of the use of `'.^label'`, see `scopejumpdemo2.6502`.

(Thanks to Steven Flintham for the implementation of `.*` and `.^.`)


`PUTFILE <host filename>, [<beeb filename>,] <start addr> [,<exec addr>]`

This provides a convenient way of copying a file from the host OS directly to the output disc image.  If no 'beeb filename' is provided, the host filename will be used (and must therefore be 7 characters or less in length). A start address must be provided (and optionally an execution address can be provided too).


`PUTTEXT <host filename>, [<beeb filename>,] <start addr> [,<exec addr>]`

This command is the same as `PUTFILE`, except that the host file is assumed to be a text file and its line endings will be automatically converted to CR (the BBC standard line ending) from any of CR, LF, CRLF or LFCR.

  
`PUTBASIC <host filename> [,<beeb filename>]`

This takes a BASIC program as a plain text file on the host OS, tokenises it,and outputs it to the disc image as a native BASIC file.  Credit to Thomas Harte for the BASIC tokenising routine.  Line numbers can be provided in the text file if desired, but if not present they will be automatically generated. 

See `autolinenumdemo.bas` for an example.
  
  
`MACRO <name> [,<parameter list...>]`

`ENDMACRO`

This pair of commands is used to define assembler macros.  Their use is best
illustrated by an example:
```
MACRO ADDI8 addr, val
  IF val=1
    INC addr
  ELIF val>1
    CLC
    LDA addr
    ADC #val
    STA addr
  ENDIF
ENDMACRO
```

This defines a macro called `ADDI8` ("ADD Immediate 8-bit") whose function is to add a constant to a memory address.  It expects two parameters: the memory address and the constant value.  The body of the macro contains an `IF` block which will generate the most appropriate code according to the constant value passed in.

Then, at any point afterwards, the macro can be used as follows:
```
ADDI8 &900, 1            ; increment address &900 by 1
ADDI8 bonus, 10          ; add 10 to the memory location 'bonus'
ADDI8 pills, pill_add    ; pills += pill_add
```
Macros can also be called from other macros, as demonstrated by this somewhat contrived example:
```
MACRO ADDI16 addr, val
  IF val=0
      ; do nothing
  ELIF val=1
    INC addr
    BNE skip1
    INC addr+1
    .skip1
  ELIF HI(val)=0
    ADDI8 addr, val
    BCC skip2
    INC addr+1
    .skip2
  ELSE
      CLC
    LDA addr
    ADC #LO(val)
    STA addr
    LDA addr+1
    ADC #HI(val)
    STA addr+1
  ENDIF
ENDMACRO
```
Care should be taken with macros, as the details of the code assembled are hidden.  In the above `ADDI16` example, the `C` flag is not set consistently, depending on the inputs to the macro (e.g. it remains unchanged if `val=0` or `1`, and will not be correct if `val<256`).


`ASSERT a [, b, c, ...]`

Abort assembly if any of the expressions is false.


`RANDOMIZE <n>`

Seed the random number generator used by the RND() function.  If this is not used, the random number generator is seeded based on the current time and so each build of a program using `RND()` will be different.

## 7. TIPS AND TRICKS

BeebAsm's approach of treating memory as a canvas which can be written to, saved, and rewritten if desired makes it very easy to create certain types of applications.

Imagine wanting to create a program which used the BBC Micro's main RAM, plus 2 sideways RAM banks.  If there was executable code in main RAM and in both banks, it's quite likely that you'd want to share label names amongst all of these blocks of code, so that main RAM routines could page in the appropriate RAM bank and call a routine in it, and likewise sideways RAM banks could call routines in main RAM.

Here's one way you could do that in BeebAsm:

```
\\ Declare origin of main RAM code
ORG &1100

\\ Put a guard at the start of screen
GUARD &5800

.mainstart
  LDA #5:STA &FE30   ; page in RAM bank 2
  JSR bank2routine
  ...
.mainroutine
  ...
.mainend

SAVE "Main", mainstart, mainend, mainentry


\\ Declare origin of bank 1 code
ORG &8000

\\ Put a guard after the RAM bank so we don't stray over our boundary
GUARD &C000

.bank1start
  ...
  JSR mainroutine
  ...
.bank1end

SAVE "Bank1", bank1start, bank1end


\\ Clear memory used by previous bank
CLEAR &8000, &C000

\\ Declare origin of bank 2 code
ORG &8000

\\ Put a guard after the RAM bank so we don't stray over our boundary
GUARD &C000

.bank2start
  ...
.bank2routine
  RTS
  ...
.bank2end

SAVE "Bank2", bank2start, bank2end
```

Because all of this code is assembled in one session, label and variable names persist across the assembly of all blocks of code.

For tidiness, you could move the source code for each block of code into a different file, and then just INCLUDE these in your main source file:
```
INCLUDE "main.asm"
INCLUDE "bank1.asm"
INCLUDE "bank2.asm"
```

## 8. DEMO

There's a little assembler demo included called `"demo.asm"`.
Build it with something like:

```beebasm -i demo.asm -do demo.ssd -boot Code -v```

and it will create a bootable disc image.

As well as demonstrating some of the features of BeebAsm (including building lookup tables), it's also a fairly good demo of pushing the hardware to its limits, in terms of creating a flicker-free animation, updating at 50Hz. (This is not to say that it's particularly impressive, but nonetheless, it really is pushing the hardware!!)

There is also a demo called `"relocdemo.asm"`, which shows how the 'reload address' feature of SAVE can be used to write self-relocating code.  This is based on the above demo, but it runs at a low address (&300).


## 9. VERSION HISTORY
```
??/??/????  1.10  ?????
12/05/2018  1.09  Added ASSERT
                  Added CPU (as a constant)
                  Added PUTTEXT
                  Added RANDOMIZE
                  Added TIME$
                  Added command line options: -title, -vc, -w, -D
		  Added conditional assignment (=?)
                  Improved error handling in PUTFILE/PUTBASIC
                  Added optional automatic line numbering for PUTBASIC
                  Show a call stack when an error occurs inside a macro
                  Allow label scope-jumping using '.*label' and '.^label'
                  Allow high bits to be set on load/execution addresses
                  Show branch displacement when "Branch out of range" occurs
                  Fixed bugs in duplicate filename detection on disc image
                  Fixed spurious "Branch out of range" error in rare case
19/01/2012  1.08  Fixed makefile for GCC (MinGW) builds.
                  Added COPYBLOCK command to manage blocks of memory.
06/10/2011  1.07  Fixed mixed-case bug for keywords (so now oddities such as
                  INy are parsed correctly).
                  Now function keywords require an open bracket immediately
                  afterwards (which now means that symbol names such as
                  HIGHSCORE, which start with the keyword HI, are valid).
16/06/2011  1.06  Fixed bug in EQUD with unsigned int values.
                  Added ERROR directive.
25/04/2011  1.05  Added macros.
                  Added PUTFILE and PUTBASIC (to tokenise a plaintext BASIC
                  file, using code by Thomas Harte).
02/12/2009  1.04  Additions by Kevin Bracey:
                  Added 65C02 instruction set and CPU directive.
                  Added ELIF, EQUD.
                  SKIP now lists the current address.
27/07/2009  1.03  Bugfix: Corrected the unary < and > operators to
                  correspond to HI and LO as appropriate.
                  Added '$' as a valid hex literal prefix.
                  Added '%' as a binary literal prefix.
06/11/2008  1.02  Bugfix: now it's possible to save location &FFFF.
                  Added 'reload address' parameter to SAVE.
                  Properly integrated the GPL License text into the
                  distribution.
14/04/2008  1.01  Bugfixes: allow lower case index in abs,x or abs,y.
                  Fails if file already exists in output disc image.
                  Symbol names may now begin with assembler opcode names.
30/03/2008  1.00  First stable release.  Corrected a few C++ compliance
                  issues in the source code.
22/01/2008  0.06  Fixed bug with forward-referenced labels in indirect
                  instructions.
09/01/2008  0.05  Added MAPCHAR.  Fixed SKIPTO (see, told you I was doing
                  it quickly!).  Enforce '%' as an end-of-symbol
                  character.  Fixed bug in overlayed assembly.  Warns if
                  there is no SAVE command in the source file.
06/01/2008  0.04  Added braces for scoping labels.  Added INCBIN, SKIPTO.
                  Added some missing functions (NOT, LOG, LN, EXP).
                  Tightened up error checking on assembling out of range
                  addresses (negative, or greater than &FFFF).  Now
                  distinguishes internally between labels and other
                  symbols.
05/01/2008  0.03  Added symbol dump for use with Swift.
20/12/2007  0.02  Fixed small bug which withheld filename and line number
                  display in error messages.
16/12/2007  0.01  First released version.
```



## 10. REPORTING BUGS

BeebAsm was writen by Rich Talbot-Watkins but is now maintained by the members of the 'stardot' forums. 

The official source repository is at https://github.com/stardot/beebasm. 

Please post any questions or comments relating to BeebAsm on the 'Development tools' forum at stardot: http://www.stardot.org.uk/forums/viewforum.php?f=55

Thank you!
