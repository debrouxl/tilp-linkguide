;**************************************************************************
;*
;* Title:		Analyzer
;* Version:		1.2
;* Last updated:	27.08.02
;* Target:		AT90S2313
;*
;* Support E-mail:	roms@lpg.ticalc.org
;*
;* Code Size		:135 words
;* Low Register Usage	:?
;* High Register Usage	:?
;* Interrupt Usage	:nothing
;*
;* DESCRIPTION
;*
;* This program spy the TI wires, retrieves the exchanged bytes and
;* send them to the serial port.
;*
;* Version 1.0: direct use of UART without buffer.
;* Version 1.1: typox fixes, clean-up
;* Version 1.2: LED monitoring
;*
;**************************************************************************

.include "2313def.inc"

;***** UART settings @ 11.0592 MHz

.equ	N=11		;57600		0.00%

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
;	rjmp	UART_RXC		;UART, RX complete handler

	.org	UDREaddr
;	rjmp	UART_DRE		;UART, Data register empty handler

	.org	UTXCaddr
;	rjmp	UART_TXC		;UART, TX complete handler

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

;.include "rs232buf.asm"	:to use RS232 subroutines (buffer)
.include "d-bus.asm"  ;to use TI subroutines


;***** Initialization (common part) 

        .def   	init	=r16

	.equ	LED1=2			;PC -> TI
	.equ	LED2=3			;WDT led
	.equ	LED3=4			;TI -> PC

RESET:  
	ldi	init,low(RAMEND)	;init Stack Pointer	
	out	SPL,init

	ldi	init,0b00000010	 	;set port D bit 1 to output
	out	DDRD,init

	ldi	init,0b00011100		;set port B bit 2,3,4 to output (LED)
        out     DDRB,init		;
	ser 	init			;all output pins at 0
	out	PORTB,init		;

	ldi	init,N			;init the baud rate generator
	out	UBRR,init
	sbi	UCR,TXEN		;transmitter enable
	sbi	UCR,RXEN		;receiver enable

;**************************************************************************
;*
;*      Main program, starts here
;*
;**************************************************************************


loop:
	sbi	PORTB,LED3		;LED1 off
	rcall	ti_spy			;spy TI transmission and gets a byte

wait:
	cbi	PORTB,LED3		;LED2 off
	sbis	USR,UDRE		;check if register is empty
	rjmp	wait
	out	UDR,ti_reg		;and send the decoded byte to serial
	rjmp	loop

.exit
