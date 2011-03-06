\ ******************************************************************
\ *
\ *		BeebAsm demo
\ *
\ *		Spinning star globe
\ *
\ *		Change the speed of rotation with Z and X keys
\ *		Press Esc to quit
\ *
\ *		Try assembling the code with debugrasters = TRUE (line 20)
\ *		It shows how the frame is split up into various stages of
\ *		processing.
\ *
\ *		The red part is where we start to plot the dots - starting as
\ *		soon before the first screenline of the next frame is rastered
\ *		as possible.
\ *
\ *		The magenta part is a small loop where we wait for 'vsync'.
\ *		It's not actually vsync, but in fact a fixed time from actual
\ *		vsync (timed by timer 1) from which point we can start to erase
\ *		points from the top of the screen down.
\ *
\ *		The blue part is the time when we are erasing dots.  Because
\ *		the dots are sorted from top to bottom of screen, we can
\ *		overlap these updates with the actual screen rasterisation.
\ *
\ ******************************************************************

\\ Define globals

numdots			= 160
radius			= 100
timerlength		= 64*8*26
debugrasters	= FALSE


\\ Define some zp locations

ORG 0

.xpos			SKIP 1
.ypos			SKIP 1
.colour			SKIP 1
.write			SKIP 2
.vsync			SKIP 1
.angle			SKIP 2
.speed			SKIP 2
.counter		SKIP 1
.temp			SKIP 1
.behindflag		SKIP 1


\\ Set start address

ORG &1100

\ ******************************************************************
\ *	The entry point of the demo
\ ******************************************************************

.start

	\\ Set up hardware state and interrupts

	SEI
	LDX #&FF:TXS				; reset stack
	STX &FE44:STX &FE45
	LDA #&7F:STA &FE4E			; disable all interrupts
	STA &FE43					; set keyboard data direction
	LDA #&C2:STA &FE4E			; enable VSync and timer interrupt
	LDA #&0F:STA &FE42			; set addressable latch for writing
	LDA #3:STA &FE40			; keyboard write enable
	LDA #0:STA &FE4B			; timer 1 one shot mode
	LDA #LO(irq):STA &204
	LDA #HI(irq):STA &205		; set interrupt handler

	\\ Clear the screen

	LDX #&40
	LDA #0
	TAY
.clearloop
	STA &4000,Y
	INY
	BNE clearloop
	INC clearloop+2
	DEX
	BNE clearloop

	\\ Set up CRTC for MODE 2

	LDX #13
.crtcloop
	STX &FE00
	LDA crtcregs,X
	STA &FE01
	DEX
	BPL crtcloop

	\\ Set up video ULA for MODE 2

	LDA #&F4
	STA &FE20

	\\ Set up palette for MODE 2

	LDX #15
.palloop
	LDA paldata,X
	STA &FE21
	ORA #&80
	STA &FE21
	DEX
	BPL palloop

	\\ Initialise vars

	LDA #0:STA angle:STA angle+1
	STA vsync
	STA speed
	LDA #1:STA speed+1

	\\ Enable interrupts, ready to start the main loop

	CLI

	\\ First we wait for 'vsync' so we are synchronised

.initialwait
	LDA vsync:BEQ initialwait:LDA #0:STA vsync

	\\ This is the main loop!

.mainloop

	\\ Plot every dot on the screen

	LDX #0
.plotdotloop
	STX counter

	; setup y pos ready for plot routine

	LDA doty,X:STA ypos

	; get sin index

	CLC:LDA dotx,X:ADC angle+1:TAY
	CLC:ADC #64:STA behindflag

	; get colour from sin index

	LDA coltable,Y:STA colour

	; perform sin(x) * radius
	; discussion of the multiplication method below in the table setup

	SEC:LDA sintable,Y:STA temp:SBC dotr,X
	BCS noneg:EOR #&FF:ADC #1:.noneg
	CPY #128:TAY:BCS negativesine

	CLC:LDA dotr,X:ADC temp:TAX
	BCS morethan256:SEC
	LDA multtab1,X:SBC multtab1,Y:JMP donemult
	.morethan256
	LDA multtab2,X:SBC multtab1,Y:JMP donemult

	.negativesine
	CLC:LDA dotr,X:ADC temp:TAX
	BCS morethan256b:SEC
	LDA multtab1,Y:SBC multtab1,X:JMP donemult
	.morethan256b
	LDA multtab1,Y:SBC multtab2,X
	.donemult

	CLC:ADC #64:STA xpos

	; routine to plot a dot
	; also we remember the calculated screen address in the dot tables

	LDA ypos:LSR A:LSR A:AND #&FE
	TAX
	LDA xpos:AND #&FE:ASL A:ASL A
	STA write
	LDY counter:STA olddotaddrlo,Y
	TXA:ADC #&40:STA write+1:STA olddotaddrhi,Y
	LDA ypos:AND #7:STA olddotaddry,Y:TAY
	LDA xpos:LSR A:LDA colour:ROL A:TAX
	LDA colours,X
	ORA (write),Y
	STA (write),Y
	BIT behindflag:BMI behind

	; if the dot is in front, we double its size

	DEY:BPL samescreenrow
	DEC write+1:DEC write+1:LDY #7:.samescreenrow
	LDA colours,X
	ORA (write),Y
	STA (write),Y
	.behind

	; loop to the next dot

	LDX counter
	INX:CPX #numdots
	BEQ waitforvsync
	JMP plotdotloop

	\\ Wait for VSync here

.waitforvsync
	IF debugrasters
		LDA #&00 + PAL_magenta:STA &FE21
	ENDIF
.waitingforvsync
	LDA vsync:BEQ waitingforvsync
	CMP #2:BCS exit		; insist that it runs in a frame!
	LDA #0:STA vsync

	\\ Now delete all the old dots.
	\\ We actually do this when the screen is still rasterising down..!

	TAX
.eraseloop
	LDY olddotaddrlo,X:STY write
	LDY olddotaddrhi,X:STY write+1
	LDY olddotaddry,X
	STA (write),Y
	DEY:BPL erasesamerow
	DEC write+1:DEC write+1:LDY #7:.erasesamerow
	STA (write),Y
	INX:CPX #numdots
	BNE eraseloop

	IF debugrasters
		LDA #&00 + PAL_red:STA &FE21
	ENDIF

	\\ Add to rotation

	CLC:LDA angle:ADC speed:STA angle
	LDA angle+1:ADC speed+1:STA angle+1

	\\ Check keypresses

	LDA #66:STA &FE4F:LDA &FE4F:BPL notx
	CLC:LDA speed:ADC #16:STA speed:BCC notx:INC speed+1:.notx
	LDA #97:STA &FE4F:LDA &FE4F:BPL notz
	SEC:LDA speed:SBC #16:STA speed:BCS notz:DEC speed+1:.notz
	LDA #112:STA &FE4F:LDA &FE4F:BMI exit

	JMP mainloop

	\\ Exit - in the least graceful way possible :)

.exit
	JMP (&FFFC)



\ ******************************************************************
\ *	IRQ handler
\ ******************************************************************

.irq
	LDA &FE4D:AND #2:BNE irqvsync
.irqtimer
	LDA #&40:STA &FE4D:INC vsync
	IF debugrasters
		LDA #&00 + PAL_blue:STA &FE21
	ENDIF
	LDA &FC
	RTI
.irqvsync
	STA &FE4D
	LDA #LO(timerlength):STA &FE44
	LDA #HI(timerlength):STA &FE45
	IF debugrasters
		LDA #&00 + PAL_black:STA &FE21
	ENDIF
	LDA &FC
	RTI



\ ******************************************************************
\ *	Colour table used by the plot code
\ ******************************************************************

.colours
	EQUB &00, &00		; black pixels
	EQUB &02, &01		; blue pixels
	EQUB &08, &04		; red pixels
	EQUB &0A, &05		; magenta pixels
	EQUB &20, &10		; green pixels
	EQUB &22, &11		; cyan pixels
	EQUB &28, &14		; yellow pixels
	EQUB &2A, &15		; white pixels



\ ******************************************************************
\ *	Values of CRTC regs for MODE 2
\ ******************************************************************

.crtcregs
	EQUB 127			; R0  horizontal total
	EQUB 64				; R1  horizontal displayed - shrunk a little
	EQUB 91				; R2  horizontal position
	EQUB 40				; R3  sync width
	EQUB 38				; R4  vertical total
	EQUB 0				; R5  vertical total adjust
	EQUB 32				; R6  vertical displayed
	EQUB 34				; R7  vertical position
	EQUB 0				; R8  interlace
	EQUB 7				; R9  scanlines per row
	EQUB 32				; R10 cursor start
	EQUB 8				; R11 cursor end
	EQUB HI(&4000/8)	; R12 screen start address, high
	EQUB LO(&4000/8)	; R13 screen start address, low


\ ******************************************************************
\ *	Values of palette regs for MODE 2
\ ******************************************************************

PAL_black	= (0 EOR 7)
PAL_blue	= (4 EOR 7)
PAL_red		= (1 EOR 7)
PAL_magenta = (5 EOR 7)
PAL_green	= (2 EOR 7)
PAL_cyan	= (6 EOR 7)
PAL_yellow	= (3 EOR 7)
PAL_white	= (7 EOR 7)

.paldata
	EQUB &00 + PAL_black
	EQUB &10 + PAL_blue
	EQUB &20 + PAL_red
	EQUB &30 + PAL_magenta
	EQUB &40 + PAL_green
	EQUB &50 + PAL_cyan
	EQUB &60 + PAL_yellow
	EQUB &70 + PAL_white



\ ******************************************************************
\ *	sin table
\ ******************************************************************

; contains ABS sine values
; we don't store the sign as it confuses the multiplication.
; we can tell the sign very easily from whether the index is >128

ALIGN &100	; so we don't incur page-crossed penalties
.sintable
FOR n, 0, 255
	EQUB ABS(SIN(n/128*PI)) * 255
NEXT


\ ******************************************************************
\ *	colour table
\ ******************************************************************

ALIGN &100
.coltable
FOR n, 0, 255
	EQUB (SIN(n/128*PI) + 1) / 2.0001 * 7 + 1
NEXT


\ ******************************************************************
\ *	multiplication tables
\ ******************************************************************

; This is a very quick way to do multiplies, based on the fact that:
;
;							(a+b)^2  =  a^2 + b^2 + 2ab    (I)
;							(a-b)^2  =  a^2 + b^2 - 2ab    (II)
;
; (I) minus (II) yields:	(a+b)^2 - (a-b)^2 = 4ab
;
; 		  or, rewritten:	ab = f(a+b) - f(a-b),
;											where f(x) = x^2 / 4
;
; We build a table of f(x) here with x=0..511, and then can perform
; 8-bit * 8-bit by 4 table lookups and a 16-bit subtract.
;
; In this case, we will discard the low byte of the result, so we
; only need the high bytes, and can do just 2 table lookups and a
; simple 8-bit subtract.

ALIGN &100
.multtab1
FOR n, 0, 255
	EQUB HI(n*n DIV 4)
NEXT
.multtab2
FOR n, 256, 511
	EQUB HI(n*n DIV 4)
NEXT


\ ******************************************************************
\ *	dot tables
\ ******************************************************************

; contains the phase of this dot

ALIGN &100
.dotx
FOR n, 0, numdots-1
	EQUB RND(256)
NEXT


; contains the y position of the dot
; the dots are sorted by y positions, highest on screen first - this means we can do
; 'raster chasing'!
; the y positions are also biased so there are fewer at the poles, and more at the equator!

ALIGN &100
.doty
FOR n, 0, numdots-1
	x = (n - numdots/2 + 0.5) / (numdots/2)
	y = (x - SIN(x*PI) * 0.1) * radius
	EQUB 128 + y
NEXT


; contains the radius of the ball at this y position

ALIGN &100
.dotr
FOR n, 0, numdots-1
	x = (n - numdots/2 + 0.5) / (numdots/2)
	y = (x - SIN(x*PI) * 0.1) * radius
	r = SQR(radius*radius - y*y) / 2
	EQUB r
NEXT


\ ******************************************************************
\ *	End address to be saved
\ ******************************************************************
.end



\ ******************************************************************
\ *	Space reserved for tables but not initialised with anything
\ * Therefore these are not saved in the executable
\ ******************************************************************

; these store the screen address of the last dot
; at the end of the frame, we go through these tables, storing zeroes to
; all these addresses in order to delete the last frame

ALIGN &100
.olddotaddrlo	SKIP numdots

ALIGN &100
.olddotaddrhi	SKIP numdots

ALIGN &100
.olddotaddry	SKIP numdots



\ ******************************************************************
\ *	Save the code
\ ******************************************************************

SAVE "Code", start, end
