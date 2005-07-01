;**************************************************************************
;*
;* Title:		XSerOut2 
;* Version:		1.0
;* Last updated:	01.01.03
;* Target:		AT90S2313
;*
;* Support E-mail:	rlievin@mail.com
;*
;* Code Size		:72 words
;* Low Register Usage	:2
;* High Register Usage	:5
;* Interrupt Usage	:nothing
;*
;* DESCRIPTION
;*
;* This application note sends a char.
;*
;**************************************************************************

.include "2313def.inc"

;***** BAUD-rate settings at 10 Mhz for the UBRR register

				;BAUD-RATES @10MHz XTAL
;.equ	N=64			; 9600
;.equ	N=42			;14400 
;.equ	N=32			;19200
;.equ	N=21			;28800
;.equ	N=15			;38400
;.equ	N=10			;57600

				;BAUD-RATES @11.0592MHz XTAL
;.equ	N=71			; 9600
;.equ	N=48			;14400 
;.equ	N=35			;19200
;.equ	N=23			;28800
;.equ	N=17			;38400
.equ	N=11			;57600


;***** UART Global Registers

;.def	u_tmp		=r16	;Scratchregister


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

;**************************************************************************
;*
;*	Test/Example Program
;*
;* This example program can be used for evaluation of the UART agains a PC
;* running a terminal emulator. If a key is pressed on the PC keyboard, the
;* message 'You typed <characher>' is sent back. The character is also
;* presented on port B.
;*
;**************************************************************************

        .def   	init	=r16
	.def	data	=r17

RESET:  
	ldi	init,low(RAMEND)	;init Stack Pointer	
	out	SPL,init
	ldi	init,0b00000010	 	;set port D bit 1 to output
	out	DDRD,init
	ldi	init,N			;init the baud rate generator
	out	UBRR,init
	sbi	UCR,TXEN		;transmitter enable
wait1:
	sbis	USR,UDRE		;check if register is empty
	rjmp	wait1
	ldi	data,'O'
	out	UDR,data
wait2:
	sbis	USR,UDRE		;check if register is empty
	rjmp	wait2
	ldi	data,'k'
	out	UDR,data
wait3:
	sbis	USR,UDRE		;check if register is empty
	rjmp	wait3
	ldi	data,' '
	out	UDR,data

	rjmp	wait1

endloop:
	rjmp	endloop