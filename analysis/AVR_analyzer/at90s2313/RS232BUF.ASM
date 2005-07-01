;**************************************************************************
;*
;* Title:		Full Duplex Interrupt Driven RS232 w/ buffers 
;* Version:		1.3
;* Last updated:	27.08.02
;* Target:		AT90S2313
;*
;* Support E-mail:	roms@lpg.ticalc.org
;*
;* Code Size		:?? words
;* Low Register Usage	:12
;* High Register Usage	:14 (r11 to r13, r21 to r25, X, Y, Z)
;* Interrupt Usage	:2 UART interrupts (UART_DRE & UART_RXC)
;*
;* DESCRIPTION
;*
;* This application note implements a full duplex buffered RS232 
;* communication layer with optional hardware flow control (CTS/RTS).
;* It use for this some RAM and the UART as well as some interrupts
;* routines.
;* The RAM is used for implementing the 2 buffers (the lower half part)
;* and for the stack (the upper half part).
;* Note: the flags of the u_status are not implemented.
;*
;* Version 1.0: original port from 8051 routines
;*	   1.1: fixes
;*	   1.2: hardware flow control with CTS/RTS
;*	   1.3: proofreading and typo fixes
;*
;* *** Initialization
;*
;*  0. Init the I/O port with the right values...
;*  1. Call rs_init
;*  2. Enable global interrupts (with 'sei')
;*
;* *** Receive
;*
;*  1. Wait until RX_EMPTY in 'u_status' becomes clear
;*  2. Call rs_get (non-blocking)
;*  3. Or call directly rs_get_w (blocking)
;*
;* *** Transmit
;*
;* (0. Initialize the UART by executing uart_init and sei)
;*  1. Wait until TX_FULL in 'u_status' becomes clear
;*  2. Set 'rs_buffer'
;*  3. Call 'rs_put'
;*
;**************************************************************************

;.include "2313def.inc"

;***** BAUD-rate settings at 10 Mhz for the UBRR register

;				;BAUD-RATES @10MHz XTAL with error (<2%)
;.equ	N9600=64		; 9600		0.16%
;.equ	N14400=42		;14400 		0.94%
;.equ	N19200=32		;19200		1.36%
;.equ	N28800=21		;28800		1.36%
;.equ	N38400=15		;38400		1.73%
;.equ	N57600=10		;57600		1.36%

;				;BAUD-RATES @11.0592MHz XTAL with error (<2%)
.equ	N9600=71		; 9600		0.00%
.equ	N14400=47		;14400 		0.00%
.equ	N19200=35		;19200		0.00%
.equ	N28800=23		;28800		0.00%
.equ	N38400=17		;38400		0.00%
.equ	N57600=11		;57600		0.00%

;***** rs232 Global Registers

.def	rs_tmp		=r22	;Temporary storage register (irq routines)
.def	rs_scratch	=r23	;Scratch register (normal routines)
.def	rs_data		=r11	;Serial buffer
.def	rs_status	=r21	;Status buffer

;**** rs232 Buffers: 48 bytes max for each buffer
;* Note: if half-duplex, the pointers can be the same ones
;* which allow bigger buffer. I will have to test it...

.def	rx_rd_ptr	=r13	;receive pointer for reading from buffer
.def	rx_wr_ptr	=r28	;receive pointer for writing to buffer (YL)
.def	rx_counter	=r24	;receive buffer counter
.equ	RX_BUFFER	=$60	;address of receive buffer (RAM)
.equ	RX_BUF_SIZE	=32	;buffer size

.def	tx_rd_ptr	=r26	;transmit pointer for reading from buffer (XL)
.def	tx_wr_ptr	=r12	;transmit pointer for writing to buffer
.def	tx_counter	=r25	;transmit buffer counter
.equ	TX_BUFFER	=$90	;address of transmit buffer (RAM)
.equ	TX_BUF_SIZE	=32	;buffer size


.equ	BUFFER_UP	=10	;drop  CTS if this limit is reached (2/3)
.equ	BUFFER_DOWN	=20	;raise CTS if this limit is reached (1/3)

;***** Bit positions in the rs232 status-register

.equ	RX_FULL		=5	;receive buffer full
.equ	RX_EMPTY	=4	;receive buffer empty

.equ	TX_FULL		=1	;transmit buffer full
.equ	TX_EMPTY	=0	;transmit buffer empty


;***** Bit positions of the different lines

.equ	TxD=1			;Transmit line 		  (port D)
.equ	RxD=0			;Receive line 		  (port D)

.equ	RTS=5			;Request To Send line 	  (port B)
.equ	CTS=6			;Clear To Send line 	  (port B)
.equ	DTR=7			;Data Terminal Ready line (port B)
				;Data Set Ready is not connected !


;**************************************************************************
;*
;* rs_init - Subroutine for RS232 initialization
;*
;*
;* DESCRIPTION
;* This routine initializes the UART. It sets the baud-rate register,
;* some flags and sets the buffer pointers.
;* To enable the UART the global interrupt flag must be set (with SEI).
;*
;*
;* Total number of words	: 18
;* Total number of cycles	: 18 (including the RET instruction)
;* Low register usage		: 8 (rs_tmp, X, Y, Z, rs_status)
;* High register usage		: 8
;* 
;**************************************************************************

rs_init:
	ldi	rs_tmp,N57600		;init the baud-rate generator
	out	UBRR,rs_tmp
	sbi	UCR,TXEN		;transmitter enable
	sbi	UCR,RXEN		;receiver enable

	ldi	ZH,0			;init the Z pointer
	ldi	ZL,0
	ldi	XH,0
	ldi	XL,TX_BUFFER		;init the tx_rd_ptr
	mov	tx_wr_ptr,XL		;init the tx_wr_ptr
	ldi	YH,0
	ldi	YL,RX_BUFFER		;init the rx_wr_ptr
	mov	rx_rd_ptr,YL		;init the rx_rd_ptr
	mov	rx_counter,ZL		;init counters
	mov	tx_counter,ZL

;	ldi	rs_tmp,0b00010001	;init the rs232 status register
	ldi	rs_tmp,((1<<TX_EMPTY) | (1<<RX_EMPTY))
	mov	rs_status,rs_tmp

	sbi	UCR,RXCIE		;enable rx interrupt but not tx yet
	ret


;**************************************************************************
;*
;* rs_put - Subroutine for UART transmittal
;*
;*
;* DESCRIPTION
;* This routine places the data to be sent in a buffer. 
;* This data must be located in rs_data. 
;*
;* //This routine modify the TX_FULL flag.
;*
;*
;* Total number of words	: 13/17
;* Total number of cycles	: ?? (including RET)
;* Low register usage		: 4 (tx_counter, tw_wr_ptr, Z)
;* High register usage		: 4
;* 
;**************************************************************************

rs_put:
rs_pt1:	cpi	tx_counter,TX_BUF_SIZE	;buffer full ?
	breq	rs_pt1			;then wait
	mov	ZL,tx_wr_ptr		;retrieve pointer
	st	Z+,rs_data		;put data in the buffer
	inc	tx_counter		;update counter

;	cpi	tx_counter,TX_BUF_SIZE	;if buffer full
;	brne	rs_pt3
;	sbr	rs_status,1<<TX_FULL	;set flag
;rs_pt3:	cbr	rs_status,1<<TX_FULL	;else clear

	cpi	ZL,TX_BUFFER+TX_BUF_SIZE;check if pointer is out
	brne	rs_pt2
	ldi	ZL,TX_BUFFER		;then wraparound the pointer
rs_pt2:	mov	tx_wr_ptr,ZL		;and update pointer
	sbic	UCR,UDRIE		;return if tx is already launched	
	ret
	sbi	UCR,UDRIE		;triggers the tx interrupt
	ret				;return


;**************************************************************************
;*
;* rs_get - Subroutine for UART receiving
;*
;*
;* DESCRIPTION
;* This routine checks if the receiving buffer is empty and if not, it gets
;* a character from the buffer and places it in rs_data.
;*
;* Checks the rx_counter to know there is a char to receive.
;*
;*
;* Total number of words	: 13
;* Total number of cycles	: ?? (including RET)
;* Low register usage		: 5 (rx_counter, Z, rs_data, rx_rd_ptr)
;* High register usage		: 5
;* 
;**************************************************************************

rs_get_w:
	cpi	rx_counter,0		;if buffer is empty
	breq	rs_get_w		;then wait
rs_get:	dec	rx_counter		;decrease the counter

;	brne	rs_gt5			;if buffer empty
;	sbr	rs_status,1<<RX_EMPTY	;set flag
;rs_gt5:	cbr	rs_status,1<<RX_EMPTY	;else clear flag

	cpi	rx_counter,BUFFER_DOWN	;if counter >= buffer lower limit
	brsh	rs_gt6			;then go on
	sbi	PORTD,CTS		;else CTS=1 (PC can transmit)

rs_gt6:	mov	ZL,rx_rd_ptr		;retrieve pointer
	ld	rs_data,Z+		;get data from the buffer
	cpi	ZL,RX_BUFFER+RX_BUF_SIZE;check if pointer is out
	brne	rs_gt2
	ldi	ZL,RX_BUFFER		;then wraparound the pointer

rs_gt2:	mov	rx_rd_ptr,ZL		;and update pointer
	ret

;**************************************************************************
;*
;* UART_DRE - UART Data Register Empty Handler Interrupt
;*
;*
;* DESCRIPTION
;* This routine is the interrupt handler of the UART_DRE interrupt.
;* This routine is executed as long as there are characters to send
;* to the serial port. For 'n' characters, it is called 'n+1' times.
;*
;* The serial routines uses a status register (rs_status): READ-ONLY.
;*	TX_EMPTY	Set when the transmit buffer is empty
;*	TX_FULL		Set when the transmit buffer is full.
;*
;* //When the TX_FULL flag is clear, the (new) data can be written into
;* //the rs_buffer.
;*
;*
;* 
;* Total number of words	: 15
;* Total number of cycles	: min. 9, max. 12 - depending on if it is
;*				  receiving or transmitting, what bit is
;*				  pending, etc.
;* Low register usage		: 4 (rs_tmp, tc_counter, X)
;* High register usage		: 4
;* 
;**************************************************************************

UART_DRE:
	in	rs_tmp,SREG		;Save the Status Register
	push	rs_tmp
	cpi	tx_counter,0		;is buffer empty ?
	brne	udre2			;if not, jump
	cbi	UCR,UDRIE		;disable tx interrupt
	rjmp	udre3	

udre2:	ld	rs_tmp,X+		;get char from buffer
	out	UDR,rs_tmp		;and transmit in
	dec	tx_counter		;update counter

;	brne	udre5			;if buffer empty
;	sbr	rs_status,1<<TX_EMPTY	;set flag
;udre5:	cbr	rs_status,1<<TX_EMPTY	;else clear flag

	cpi	XL,TX_BUFFER+TX_BUF_SIZE;check if pointer is out
	brne	udre3
	ldi	XL,TX_BUFFER		;then wraparound the pointer

udre3:	pop	rs_tmp			;Restore the Status Register
	out	SREG,rs_tmp
	reti

;**************************************************************************
;*
;* UART_RXC - UART Receive Complete Handler Interrupt
;*
;*
;* DESCRIPTION
;* This routine is the interrupt handler of the UART_RXC interrupt.
;* This routine is executed whenever a character has been received
;* from the serial port.
;*
;* The serial routines uses a status register (rs_status): READ-ONLY.
;*	RX_EMPTY	Set when the receive buffer is empty
;*	RX_FULL		Set when the receive buffer is full.
;*
;* //When the RX_EMPTY flag is clear, the (new) data can be read from 
;* //rs_buffer.
;*
;*
;* 
;* Total number of words	: 14
;* Total number of cycles	: min. 12, max. 14 - depending on if it is
;*				  receiving or transmitting, what bit is
;*				  pending, etc.
;* Low register usage		: 4 (rs_tmp, rx_counter, Y)
;* High register usage		: 4
;* 
;**************************************************************************

UART_RXC:
	in	rs_tmp,SREG		;Save the Status Register
	push	rs_tmp

	in	rs_tmp,UDR		;retrieve char
	st	Y+,rs_tmp		;and put it in the buffer
	inc	rx_counter		;update counter

;	cpi	rx_counter,RX_BUF_SIZE	;if buffer full
;	brne	rxc5
;	sbr	rs_status,1<<RX_FULL	;set flag
;rxc5:	cbr	rs_status,1<<RX_FULL	;else clear

	cpi	rx_counter,BUFFER_UP	;if counter <= buffer upper limit
	brlo	rxc6			;then go on
	sbi	PORTD,CTS		;else CTS=0 (PC waits)

rxc6:	cpi	YL,RX_BUFFER+RX_BUF_SIZE;check if pointer is out
	brne	rxc3
	ldi	YL,RX_BUFFER		;then wraparound the pointer

rxc3:	pop	rs_tmp			;Restore the Status Register
	out	SREG,rs_tmp
	reti