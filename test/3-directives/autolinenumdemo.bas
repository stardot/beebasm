REM Line numbers are now optional in BASIC programs used with the putbasic
REM command.
100PRINT "but you can give them if you want, so the change is backwards-";
110PRINT "compatible."
FOR I%=1 TO 10
RESTORE (1000+RND(3)*10)
READ word$
PRINT word$
NEXT
PROCgoodbye
END
REM Apart from backwards compatibility, being able to specify line numbers
REM is handy for the rare cases where BASIC needs them, such as computed
REM RESTORE statements.
1010DATA foo
1020DATA bar
1030DATA baz
DEF PROCgoodbye
PRINT "TTFN"
ENDPROC
