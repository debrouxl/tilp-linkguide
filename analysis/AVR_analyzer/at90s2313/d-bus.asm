;*************************** TI protocol subroutines ************************
;*
;* Title:               d-bus: subroutines for D-bus management
;* Version:		1.4
;* Last updated:        30.08.02
;* Target:		AT90Sxxxx (All AVR devices)
;*
;* Support E-mail:      roms@lpg.ticalc.org
;*
;* Code Size            : 108 words
;* Low Register Usage   : 1 (r18)
;* High Register Usage  : 3 (r16, r17, r18)
;* Interrupt Usage	: none
;*
;* DESCRIPTION
;*
;* This unit provide some routines to communicate with TI calculators.
;* This is a part of the AVRlink project.
;*
;* Version 1.1: I added a new routine: ti_spy for the AVRanalyzer.
;*         1.2: some improvements/tests about the tempo routine.
;*         1.3: added a peak detector routine.
;*         1.4: make ti_spy more robust (invalid frames, incomplete bit)
;*
;* AVRlink, (c) 2000-2002 by Romain Lievin
;* This code is placed under GPL license
;*
;**************************************************************************


;                    1                 0                      0
;R = XF0|1: _______        ______|______    __________|________    __________
;R = XF0|0:        ________      |      ____          |        ____
;W = XF1|1: _        ____________|________      ______|__________       _____
;W = XF1|0:  ________            |        ______      |          _______



;***** Bit positions in the port B *****

.equ    RED     =0              ;Red wire	(port B)
.equ    WHITE   =1              ;White wire	(port B)

;***** Register declarations (don't use r30=Z) *****

.def    bitcnt  =r16            ;bit counter
.def    scratch =r17            ;scratch register
.def    ti_reg	=r18		;data register for TI routines

;**************************************************************************
;*
;* "check"
;*
;* This subroutine checks if the 2 lines are equals to 1
;*
;* Total number of words        : 7
;* Total number of cycles       : 3 to 7
;* Low register usage           : 0 (bit T=red & white)
;* High register usage          : 0
;*
;**************************************************************************

ti_check:  
	clt			;clear the T bit
	sbis    PINB,RED        ;if red wire=0 then return
        ret
        sbis    PINB,WHITE      ;if white wire=0 then return
        ret
	set			;if red=1 & white=1, set the T bit
	ret

;**************************************************************************
;*
;* "ti_put"
;*
;* This subroutine transmits the byte stored in the "ti_reg" register
;* This subroutines resets the watchdog when it starts.
;*
;* Total number of words        : 27
;* Total number of cycles       : ??
;* Low register usage           : 1 (ti_reg)
;* High register usage          : 2 (ti_reg, bitcnt)
;*
;**************************************************************************

wti_put:	
	wdr			;reset the watchdog
ti_put:
	rcall	ti_check	;if red=0 or white=0, return
	brts	ti_put1
	ret	
ti_put1:clc                     ;clear carry
        ldi     bitcnt,8        ;8 bits to send
nextb:	ror	ti_reg        	;get next bit
	brcc	tbit_1
;
tbit_0:	
	sbi	DDRB,WHITE	;white=0
r0poll:	sbic	PINB,RED	;wait that red=0
	rjmp	r0poll
	rcall	tempoN		;not compulsory
	cbi	DDRB,WHITE	;white=1	
r1poll:	sbis	PINB,RED	;wait that red=1
	rjmp	r1poll
	rjmp	end_tx		;go to counter
;
tbit_1:	
	sbi	DDRB,RED	;red=0
w0poll:	sbic	PINB,WHITE	;wait that white=0
	rjmp	w0poll
	rcall	tempoN		;not compulsory
	cbi	DDRB,RED	;red=1	
w1poll:	sbis	PINB,WHITE	;wait that white=1
	rjmp	w1poll
	rjmp	end_tx		;go to counter
;
end_tx:	
	rcall	tempoN		;not compulsory
	dec	bitcnt		;if not all bit sent
	brne	nextb		;send next
	ret			;end

;**************************************************************************
;*
;* "ti_get"
;*
;* This subroutine receives the byte which is stored in the "ti_reg" register
;* This subroutines resets the watchdog when it starts.
;*
;* Total number of words        : 31
;* Total number of cycles       : ??
;* Low register usage           : 1 (ti_reg)
;* High register usage          : 2 (ti_reg, bitcnt)
;*
;**************************************************************************

wti_get:	
	wdr			;reset the watchdog
ti_get:
	ldi	bitcnt,8	;8 bits to receive
	clc			;clear carry
ti_get1:
	sbis    PINB,RED        ;wait that red or white=0
        rjmp	ti_get2
        sbic    PINB,WHITE
	rjmp	ti_get1
;
ti_get2:
	sbic	PINB,RED	;if red=1, jump
	rjmp	gbit_1
;
gbit_0:	
	clc			;clear carry
	ror	ti_reg		;shift bit into ti_rx
	rcall	tempoN		;not compulsory
	sbi	DDRB,WHITE	;white=0
t0poll:	sbis	PINB,RED	;wait that red=1
	rjmp	t0poll
	rcall	tempoN		;not compulsory
	cbi	DDRB,WHITE	;white=1
	rjmp	end_rx
;
gbit_1:	
	sec			;set carry
	ror	ti_reg		;shift bit into ti_rx
	rcall	tempoN		;not compulsory
	sbi	DDRB,RED	;red=0
t1poll:	sbis	PINB,WHITE	;wait that white=1
	rjmp	t1poll
	rcall	tempoN		;not compulsory
	cbi	DDRB,RED	;red=1
	rjmp	end_rx
;
end_rx:	
	rcall	tempoN		;a small tempo (REQUIRED)
	rcall	tempoN
	dec	bitcnt		;if not all bit sent
	brne	ti_get1		;send next
	ret			;end

;**************************************************************************
;*
;* "ti_spy"
;*
;* This subroutine monitors the Red & White lines and when a TI byte is
;* transmited on the lines, it decodes and stores in the "ti_reg" register
;* This subroutines does not reset the watchdog when it starts.
;*
;* Total number of words        : 31
;* Total number of cycles       : ??
;* Low register usage           : 1 (ti_reg)
;* High register usage          : 2 (ti_reg, bitcnt)
;*
;**************************************************************************

ti_spy:	
	ldi	bitcnt,0
	mov	ti_reg,bitcnt	;clear the register
	ldi	bitcnt,8	;8 bits to receive
	clc			;clear carry

tispy_s0:			;s0 = state 0

tispy_s1:
	sbis    PINB,RED        ;check if a line goes low
	rjmp	tispy_s2	;red=0, goto S2
	sbis    PINB,WHITE
	rjmp	tispy_s5	;white=0, gto s5
	rjmp	tispy_s1	;else, wait for

;
; red = 0:
;

tispy_s2:	
	sbic	PINB,RED	;error, return to S0
	rjmp	tispy_s0
	sbic	PINB,WHITE	;if white=0, goto S3
	rjmp	tispy_s2

tispy_s3:
;	sbic	PINB,WHITE	;error, return to S0
;	rjmp	tispy_s0
	sbis	PINB,RED	;if red=1, goto S4
	rjmp	tispy_s3

tispy_s4:
	sbis	PINB,WHITE	;if white=1, goto S8
	rjmp	tispy_s4

	clc			;clear carry
	ror	ti_reg		;shift bit into ti_reg

	rjmp	tispy_s8

;
; white = 0
;

tispy_s5:
	sbic	PINB,WHITE	;error, return to S0
	rjmp	tispy_s0
	sbic	PINB,RED	;if red=0, goto S6
	rjmp	tispy_s5

tispy_s6:
;	sbic	PINB,RED	;error, return to S0
;	rjmp	tispy_s0
	sbis	PINB,WHITE	;if white=1, goto S7
	rjmp	tispy_s6

tispy_s7:	
	sbis	PINB,RED	;if red=1, goto S8
	rjmp	tispy_s7

	sec			;set carry
	ror	ti_reg		;shift bit into ti_reg

	rjmp	tispy_s8

;
; count bits...
;

tispy_s8:	
	dec	bitcnt		;if not all bit sent
	brne	tispy_s0	;get next
	ret			;end

;**************************************************************************
;*
;* "ti_peak"
;*
;* This subroutine monitors the Red & White lines and when a line has changed
;* it stores the line status in the "ti_reg" register.
;* This subroutines does not reset the watchdog when it starts.
;* Warning: this routine can not be used with ti_put/get due to r17 !
;*
;* Total number of words        : 8
;* Total number of cycles       : 3/5
;* Low register usage           : 1 (ti_reg)
;* High register usage          : 2 (ti_reg, scratch)
;*
;**************************************************************************
	
	.def    last =r17       ;contains the previous line status

ti_peak1:
	in	ti_reg,PINB
	andi	ti_reg,((1<<RED) | (1<<WHITE))
	ret

ti_peak2:
	in	ti_reg,PINB
pdl:	eor	ti_reg, last	;detect a change
	breq	pdl
	in	last, PINB	;save new state
	ret			;end

;**************************************************************************
;*
;* "tempo"
;*
;* This subroutine generates a delay. The total execution time is function
;* of scratch (1 microsecond at 12 MHz by default)
;*
;* Total number of words        : 4
;* Total number of cycles       : 2+3*scratch
;* Low register usage           : 0
;* High register usage          : 1 (scratch)
;*
;**************************************************************************

	.def    counter =r17    ;our counter

tempoN:	
	ldi	counter,8	;scratch=4 -> 1 microsecond
tempo:	dec	counter
	brne	tempo
	ret

	;; Tips
;
;output mode only (DDBn = 1)
;	sbi	PORTn	S=1
;	cbi	PORTn	S=0
;ouput mode and monitored input (PORTBn = 0)
;	cbi	DDRn	S=1/E
;	sbi	DDRn	S=0