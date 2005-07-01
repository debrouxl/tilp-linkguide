;**************************************************************************
;*
;* Title:		Test
;* Version:		1.1
;* Last updated:	27.08.02
;* Target:		AT90S2313
;*
;* Support E-mail:	roms@lpg.ticalc.org
;*
;* Code Size		:142 words
;* Low Register Usage	:2
;* High Register Usage	:5
;* Interrupt Usage	:nothing
;*
;* DESCRIPTION
;*
;* This application note tests the TI module.
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
UART_RXC:
UART_DRE:
UART_TXC:
ANA_COMP:
	reti

;***** Include subroutines

.include "d-bus.asm"  ;to use TI protocol subroutines (buffer)


;***** Initialization (common part) 

        .def   	init	=r16
	.def	data	=r17

RESET:  
	ldi	init,low(RAMEND)	;init Stack Pointer	
	out	SPL,init
	ldi	init,0b00000010	 	;set port D bit 1 to output
	out	DDRD,init
	clr	init
        out     DDRB,init		;all pins in input (Hi-Z)
	out     PORTB,init		;enable pull-ups
	
;**************************************************************************
;*
;*      Test program for TI routines
;*
;**************************************************************************
ti_sub:	
	ldi	ti_reg,0x08
	rcall	ti_put
	ldi	ti_reg,0x87
	rcall	ti_put
	ldi	ti_reg,'A'
	rcall	ti_put
	ldi	ti_reg,0x00
	rcall	ti_put
	rcall	ti_get
	rcall	ti_get
	rcall	ti_get
	rcall	ti_get
jmp:	rjmp	jmp

.exit
