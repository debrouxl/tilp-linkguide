;**************************************************************************
;*
;* Title:		Test_232
;* Version:		1.1
;* Last updated:	27.08.02
;* Target:		AT90S2313
;*
;* Support E-mail:	rlievin@mail.com
;*
;* Code Size		:122 words
;* Low Register Usage	:2
;* High Register Usage	:5
;* Interrupt Usage	:nothing
;*
;* DESCRIPTION
;*
;* This application note tests the rs232buf module.
;*
;* Version 1.0: original.
;* Version 1.1: typox fixes, clean-up.
;*
;**************************************************************************

.include "2313def.inc"

;***** WatchDog settings

;.equ	WDP=4	;0.24s at 5.0V (0.75s at 3.0V)
.equ	WDP=5	;0.49s at 5.0V (1.5s at 3.0V)
;.equ	WDP=6	;0.97s at 5.0V


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

;***** Initialization (common part) 

        .def   	init	=r16
	.def	data	=r17

RESET:  
	ldi	init,low(RAMEND)	;init Stack Pointer	
	out	SPL,init
	ldi	init,0b00000010	 	;set port D bit 1 to output
	out	DDRD,init
	clr	init
	out     PORTB,init		;all outputs pins at 0
        out     DDRB,init		;all pins in input (Hi-Z)
	
;wait:	rcall	check			;wait that red=1 & white=1 (watchdog reset)
;	brtc	wait			;
;	sei				;enable interrupts
;	ldi	scratch,(1<<WDE) + WDP	;load the watchdog prescaler 
;	out	WDTCR,scratch		;and enable watchdog


;**************************************************************************
;*
;*	Test/Example Program
;*
;**************************************************************************

	rcall	rs_init			;init rs232 routines
	sei				;enable interrupts

	ldi	data,'S'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'a'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'l'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'u'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'t'
	mov	rs_data,data
	rcall	rs_put
	ldi	data,'!'
	mov	rs_data,data
	rcall	rs_put

endloop:
	cpi	rx_counter,0		;if buffer is empty
	breq	endloop			;then wait
	rcall	rs_get

	rcall	rs_put
	rjmp	endloop

stop:	rjmp	stop

.exit
