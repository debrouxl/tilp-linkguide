;**************************************************************************
;*
;* Title:		Analyzer
;* Version:		1.0
;* Last updated:	27.08.02
;* Target:		AT90S2313
;*
;* Support E-mail:	roms@lpg.ticalc.org
;*
;* Code Size		:?? words
;* Low Register Usage	:?
;* High Register Usage	:?
;* Interrupt Usage	:nothing
;*
;* DESCRIPTION
;*
;* This program is a peak detector. It sends a byte whenever a transition is
;* detected on the D-bus.
;*
;* Version 1.0: direct use of UART without buffer.
;*
;**************************************************************************

.include "2313def.inc"

;**************************************************************************
;*
;*	VECTOR TABLE - EXECUTION STARTS HERE
;*
;**************************************************************************
	.cseg

	.org 	$0000
	rjmp	RESET			;Reset handler

	.org 	INT0addr
	rjmp	EXT_INT0		;External interrupt 0 handler

	.org 	INT1addr
	rjmp	EXT_INT1		;External interrupt 0 handler

	.org 	ICP1addr
	rjmp	TIM_CAPT1		;Timer/counter1 capture event handler

	.org	OC1addr
	rjmp	TIM_COMP1		;Timer/counter1 compare match handler

	.org	OVF1addr
	rjmp	TIM_OVF1		;Timer/counter1 overflow handler

	.org	OVF0addr
	rjmp	TIM_OVF0		;Timer/counter0 coverflow handler

	.org	URXCaddr
	rjmp	UART_RXC		;UART, RX complete handler

	.org	UDREaddr
	rjmp	UART_DRE		;UART, Data register empty handler

	.org	UTXCaddr
	rjmp	UART_TXC		;UART, TX complete handler

	.org ACIaddr
	rjmp	ANA_COMP		;Analog comparator handler

; unused vectors: return of interrupt

EXT_INT0:
EXT_INT1:
TIM_CAPT1:
TIM_COMP1:
TIM_OVF1:
TIM_OVF0:
;UART_RXC:
;UART_DRE:
UART_TXC:
ANA_COMP:
	reti

;***** Include subroutines

.include "rs232buf.asm"	:to use RS232 subroutines (buffer)
.include "d-bus.asm"  ;to use TI subroutines


;***** Initialization (common part) 

        .def   	init	=r16		;warning: use by ti_prot (ti_put/get/spt) !
	.def	data	=r17			;ti_prot (scratch)

RESET:  
	ldi	init,low(RAMEND)	;init Stack Pointer	
	out	SPL,init
	ldi	init,0b00000010	 	;set port D bit 1 to output
	out	DDRD,init
	clr	init
        out     DDRB,init		;all pins in input (Hi-Z)
	out     PORTB,init		;enable pull-ups
	ldi	init,N57600		;init the baud rate generator
	out	UBRR,init
	sbi	UCR,TXEN		;transmitter enable
	sbi	UCR,RXEN		;receiver enable

;**************************************************************************
;*
;*      Main program, starts here
;*
;**************************************************************************

	rcall	rs_init			;init rs232 routines
	sei						;enable interrupts

	ldi	data,'R'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'e'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'a'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'d'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'y'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'!'
	mov	rs_data,data
	rcall	rs_put


	ldi	init, 'A'
endloop:
	rcall	ti_peak1		;poll D-bus
	add	ti_reg, init		;make monitoring easy

;wait:
;	sbis	USR,UDRE		;check if register is empty
;	rjmp	wait
;	out	UDR,ti_reg		;and send the decoded byte to serial
;	rjmp	endloop

	mov	rs_data,ti_reg
	rcall	rs_put
	rjmp	endloop

.exit
