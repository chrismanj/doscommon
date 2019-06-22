#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#include <c:\progproj\c\common\include\types.h>
#include <c:\progproj\c\common\include\debug.h>
#include <c:\progproj\c\common\include\jsctime.h>
#include <c:\progproj\c\common\include\bqueue.h>
#include <c:\progproj\c\common\include\jscser.h>

#define SUPPORT16550
#define XOFF     19
#define XON      17

/* Function prototypes for static functions */
static void Serial_ISR (s_com_port *);
static void _interrupt _far Serial_ISR0 (void);
static void _interrupt _far Serial_ISR1 (void);
static void _interrupt _far Serial_ISR2 (void);
static void _interrupt _far Serial_ISR3 (void);

/* Variables global to JSCSER */
static void (_interrupt _far *ISRs[])(void) = {Serial_ISR0, Serial_ISR1, Serial_ISR2, Serial_ISR3};
static s_com_port *port_ISRs[4] = {NULL, NULL, NULL, NULL};

/******************************************************************************\
 These are the interrupt service routines for the com ports. They get the next
 character out of the receive buffer register 0 and place it into the software
 buffer.
\******************************************************************************/

static void _interrupt _far Serial_ISR0 (void)
{
	Serial_ISR(port_ISRs[0]);
}

static void _interrupt _far Serial_ISR1 (void)
{
	Serial_ISR(port_ISRs[1]);
}

static void _interrupt _far Serial_ISR2 (void)
{
	Serial_ISR(port_ISRs[2]);
}

static void _interrupt _far Serial_ISR3 (void)
{
	Serial_ISR(port_ISRs[3]);
}

static void KickStartXmitInt (s_com_port *port)
{
	outp(port->com_THR, Dequeue(port->sbuff));
	outp(port->com_IER, IER_ERBFI + IER_ETBEI + IER_ELSI + IER_EDSSI);
	port->xmit_interrupt_stopped = FALSE;
}

static void Serial_ISR (s_com_port *port)
{
	int ch, x;

	while (1)
	{
		ch = inp (port->com_IIR) & 0x07; /* Find out what caused the interrupt */

		switch (ch)
		{
			case ERROR_INT:
				port->hw_error = inp(port->com_LSR);
				break;

			case RBF_FULL_INT:

				while (inp(port->com_LSR) & LSR_RBF)
				{
					ch = inp(port->com_RBF);
					if (port->xon_xoff_enabled == TRUE)
					{
						if (ch == XON)
						{
							if (port->xmit_interrupt_stopped == TRUE && port->output_blocked_by_xoff == TRUE
								&& port->output_blocked_by_cts == FALSE && port->sbuff->count)
									KickStartXmitInt (port);
							port->output_blocked_by_xoff = FALSE;
							break;
						}
						if (ch == XOFF)
						{
							port->output_blocked_by_xoff = TRUE;
							outp(port->com_IER, IER_ERBFI + IER_ELSI + IER_EDSSI);
							port->xmit_interrupt_stopped = TRUE;
							break;
						}
					}
					Enqueue (port->rbuff, (char)ch);
					if (port->rbuff->count == port->rec_buff_maxchars)
					{
						if (port->xon_xoff_enabled == TRUE)
						{
							while(!(inp(port->com_LSR) & LSR_THRE)); /* Wait until xmit buffer empty */
							outp(port->com_THR, XOFF); /* send xoff */
							port->input_blocked_by_flow_control = port->input_flow_changed = port->input_blocked_by_xoff = TRUE;
						}

						if (port->rts_cts_enabled == TRUE)
						{
							outp(port->com_MCR, MCR_GP02 + MCR_GP01 + MCR_DTR);
							port->input_blocked_by_flow_control = port->input_flow_changed = port->input_blocked_by_cts = TRUE;
						}
					}
				}
				break;

			case THR_EMPTY_INT:

				if (port->sbuff->count)
				{
					if (port->UARTType == UART16550)
					{
						for (x = 0; x < 16 && port->sbuff->count; x++)
							outp(port->com_THR, Dequeue(port->sbuff));
					}
					else
						outp(port->com_THR, Dequeue(port->sbuff));
				}
				else
				{
					outp(port->com_IER, IER_ERBFI + IER_ELSI + IER_EDSSI);
					port->xmit_interrupt_stopped = TRUE;
				}
				break;

			case MODEM_STATUS_INT:

				port->com_status = inp(port->com_MSR);
				if (port->rts_cts_enabled == TRUE)
				{
					if (port->com_status & MSR_CTS)
					{
						if (port->xmit_interrupt_stopped == TRUE && port->output_blocked_by_xoff == FALSE
							&& port->output_blocked_by_cts == TRUE && port->sbuff->count)
							KickStartXmitInt(port);
						port->output_blocked_by_cts = FALSE;
					}
					else
					{
						outp(port->com_IER, IER_ERBFI + IER_ELSI + IER_EDSSI);
						port->output_blocked_by_cts = port->xmit_interrupt_stopped = TRUE;
					}
				}
				port->modem_status_changed = TRUE;
				break;

			default:
				outp(PIC_ICR, 0x20); /* Restore PIC */
				return;
		}
	}
}

/******************************************************************************\
 Sets a ports address and IRQ (0 - 7). This routine should not be called to
 change the IRQ of an open serial port.
\******************************************************************************/

void SetPortAddressAndIRQ (s_com_port *port, WORD new_address, WORD new_interrupt)
{
	port->port_interrupt = new_interrupt;
	port->com_RBF = new_address + SER_RBF;
	port->com_THR = new_address + SER_THR;
	port->com_MCR = new_address + SER_MCR;
	port->com_LSR = new_address + SER_LSR;
	port->com_MSR = new_address + SER_MSR;
	port->com_LCR = new_address + SER_LCR;
	port->com_DL  = new_address + SER_DL;
	port->com_IER = new_address + SER_IER;
	port->com_IIR = new_address + SER_IIR;
	port->com_FCR = new_address + SER_FCR;
	port->comm_error = 0;
}

/******************************************************************************\
 This functions returns true if there are any characters waiting and 0 if the
 buffer is empty. This function should be called before attempting to use
 SerialRead(). Why? Because SerialRead() will return a NULL if there are no
 characters in the receive buffer. Unless you test first to see if there are
 characters in the receive buffer there is no way to know if the NULL was
 actually sent to the port or if SerialRead() is just trying to tell you
 there's nothing in the buffer to return.
\******************************************************************************/

WORD ReadySerial(s_com_port *port)
{
	port->comm_error = 0;
	return port->rbuff->count;
}

/******************************************************************************\
  This function reads a character from the circulating buffer and returns it
  to the caller. It returns 0 if there are no characters in the buffer.
\******************************************************************************/

BYTE SerialRead(s_com_port *port)
{
	SBQueue *rbuff = port->rbuff;
	
	if (rbuff->count) /* Test for presence of character(s) in buffer */
	{
		BYTE ch = Dequeue (rbuff);
		
		/* Test to see if input to a port is currently being blocked by flow
		control. If so, see if the buffer is empty enough to stop blocking
		input. */
		
		if (rbuff->count == port->rec_buff_minchars
			&& port->input_blocked_by_flow_control == TRUE)
		{
			if (port->xon_xoff_enabled == TRUE)
				SerialWrite (port, XON);
			if (port->rts_cts_enabled == TRUE)
				outp(port->com_MCR, inp(port->com_MCR) & MCR_RTS);
			port->input_blocked_by_flow_control = FALSE;
			port->input_flow_changed = TRUE;
		}
		port->comm_error = 0;
		return ch;
	}
	else
	{
		port->comm_error = UNDERFLOW;
		return 0; /* buffer was empty return a NULL */
	}
}

/******************************************************************************\
\******************************************************************************/

void SetReadTimeout (s_com_port *port, long timeout_val)
{
	port->comm_error = 0;
	port->read_timeout = timeout_val;
}

/******************************************************************************\
\******************************************************************************/

long GetReadTimeout (s_com_port *port)
{
	return port->read_timeout;
}

/******************************************************************************\
\******************************************************************************/

BYTE SerialReadWTimeout (s_com_port *port)
{
	long timer;
	
	timer = StartTimer();
	while (!ReadySerial(port) && (TimerValue (timer) < port->read_timeout));
	if (TimerValue (timer) >= port->read_timeout)
	{
		port->comm_error = 1;
		return 0;
	}
	else
		return SerialRead(port);
}

/******************************************************************************\
\******************************************************************************/

BYTE WaitForCharSerial (s_com_port *port, BYTE char_to_wait_for)
{
	BYTE ch = (BYTE)~char_to_wait_for;
	long timer;
	
	port->comm_error = 0;
	timer = StartTimer();
	while ((ch != char_to_wait_for) && (TimerValue(timer) < port->read_timeout))
		ch = SerialReadWTimeout (port);
	if ((TimerValue (timer) >= port->read_timeout) && (ch != char_to_wait_for))
		port->comm_error = 1;
	return ch;
}

/******************************************************************************\
\******************************************************************************/

BYTE WaitForCharsSerial (s_com_port *port, char *chars)
{
	BYTE ch;
	long timer = StartTimer();
	BOOLN found = FALSE;
	BYTE *cur_char;
	
	port->comm_error = 0;
	while (found == FALSE && (TimerValue(timer) < port->read_timeout))
	{
		ch = SerialReadWTimeout (port);
		cur_char = chars;
		while (*cur_char != 0 && found == FALSE)
			if (ch == *cur_char++)
				found = TRUE;
	}
	if (TimerValue (timer) >= port->read_timeout)
	{
		port->comm_error = 1;
		return 0;
	}
	return ch;
}

/******************************************************************************\
 This function writes a character to the transmit buffer. If the transmit buffer
 is full upon entry, it will wait 1/2 a second for a BYTE to be freed.
\******************************************************************************/

int SerialWrite(s_com_port *port, BYTE ch)
{
	long timer;
	
	port->comm_error = 0;
	timer = StartTimer();
	/* Check to see if the send buffer is full, if it is wait 1/2 a second for a
	spot to be freed */
	while ((port->sbuff->count == port->sbuff->size) && (TimerValue(timer) <= 50L));
	if (TimerValue (timer) > 50L)
		return 1;
	
	/* Place character in send buffer */
	_disable();
	Enqueue (port->sbuff, ch);
	if (port->xmit_interrupt_stopped == TRUE && port->output_blocked_by_xoff == FALSE
		&& port->output_blocked_by_cts == FALSE)
		KickStartXmitInt (port);
	_enable();
	return 0;
}

/******************************************************************************\
 This function writes a string to the transmit buffer, but first it
 waits for the transmit buffer to be empty.  Note: it is not interrupt
 driven and it turns off interrupts while it's working.
\******************************************************************************/

int SerialStringWrite (s_com_port *port, char *string)
{
	int retval = 0;
	
	port->comm_error = 0;
	while (*string && retval == 0)
		retval = SerialWrite (port, *string++);
	return retval;
}

/******************************************************************************\
 Configures the data length, parity & stop bits for a port.

 Pass: Port
       Data Length: 0 = 5 bits per data WORD
		    1 = 6 bits per data WORD
		    2 = 7 bits per data WORD
		    3 = 8 bits per data WORD

       Parity: 0 = None
	       1 = Odd
	       2 = Even
	       3 = Mark
	       4 = Space

       Stop Bits: 0 = 1 stop bit
		  1 = 2 stop bits
\******************************************************************************/

int ConfigurePort (s_com_port *port, WORD data_length, WORD parity, WORD stop_bits)
{
	WORD configuration;
	
	port->comm_error = 0;
	configuration = (WORD)data_length | (WORD)stop_bits | (WORD)parity;
	outp(port->com_LCR, configuration);
	if ((WORD)inp(port->com_LCR) == configuration)
		return 0;
	else
		return 1;
}

/******************************************************************************\
 Sets the baud rate for a port
\******************************************************************************/

int SetBaud (s_com_port *port, DWORD baud)
{
	int old_lcr = inp (port->com_LCR);
	int retval = 0;
	WORD brd = (WORD)((DWORD)115200 / baud);
	
	port->comm_error = 0;
	outp (port->com_LCR, LCR_DLAB); /* Turn on divisor latch regs */
	outpw (port->com_DL, brd);
	if (inpw(port->com_DL) != brd)
		retval = 1;
	outp (port->com_LCR, old_lcr); /* Turn off divisor latch regs */
	return retval;
}

/******************************************************************************\
\******************************************************************************/

void SavePortState(s_com_port *port, s_port_state *port_state)
{
	port->comm_error = 0;
	port_state->IER = inp(port->com_IER);
	port_state->LCR = inp(port->com_LCR);
	port_state->MCR = inp(port->com_MCR);
	
	outp(port->com_LCR, LCR_DLAB); /* Turn on divisor latch regs */
	port_state->DL = inpw(port->com_DL);
	outp(port->com_LCR, port_state->LCR);   /* Turn off divisor latch regs */
	
	port_state->Old_ISR = _dos_getvect(port->port_interrupt + 0x08);
	port_state->int_enabled = inp(PIC_IMR) & (0x01 << port->port_interrupt);
}

/******************************************************************************\
\******************************************************************************/

void RestorePortState(s_com_port *port, s_port_state *port_state)
{
	int x;
	
	port->comm_error = 0;
	outp(port->com_IER, 0);
	outp(port->com_MCR, port_state->MCR | MCR_DTR);
	outp(port->com_LCR, LCR_DLAB);                /* Turn on divisor latch regs */
	outpw(port->com_DL, port_state->DL);
	outp(port->com_LCR, port_state->LCR);
	
	/* Clear pending interrupts */
	inp(port->com_MSR);
	for (x = 0; x < 1024 && (inp(port->com_LSR) & LSR_RBF); x++)
		inp(port->com_RBF);
	for (x = 0; x < 1024 && !(inp(port->com_LSR) & LSR_THRE); x++);
	
	_dos_setvect(port->port_interrupt + 0x08, port_state->Old_ISR);
	outp(port->com_IER, port_state->IER);
	outp(PIC_IMR, inp(PIC_IMR) | port_state->int_enabled);
}

/******************************************************************************\
 This function will open up the serial port, set it's configuration, turn on
 interrupts and load the ISR.
\******************************************************************************/

s_com_port *OpenSerial(WORD address, int irq, long baud, int data_length,
					   int parity, int stop_bits, WORD rec_buffsize,
					   WORD send_buffsize)
{
	int x;
	s_com_port *port;
	BOOLN error_opening = FALSE;
	
	port = malloc (sizeof(s_com_port));
	if (port == NULL)
		return NULL;
	
	port->rbuff = NULL;
	port->sbuff = NULL;
	
	if((port->rbuff = InitByteQueue (rec_buffsize)) != NULL)
	{
		if((port->sbuff = InitByteQueue (send_buffsize)) != NULL)
		{
			port->xon_xoff_enabled              = FALSE;
			port->rts_cts_enabled               = FALSE;
			port->output_blocked_by_xoff        = FALSE;
			port->output_blocked_by_cts         = FALSE;
			port->rec_buff_maxchars             = (WORD) (port->rec_buff_minchars * 4);
			port->rec_buff_minchars             = (WORD) (rec_buffsize / 5);
			port->input_blocked_by_xoff         = FALSE;
			port->input_blocked_by_cts          = FALSE;
			port->input_blocked_by_flow_control = FALSE;
			port->input_flow_changed            = TRUE;
			port->comm_error                    = 0;
			port->hw_error                      = 0;
			port->read_timeout                  = 300L;
			port->modem_status_changed          = TRUE;
			port->xmit_interrupt_stopped        = TRUE;
			
			SetPortAddressAndIRQ (port, address, irq);
			SavePortState(port, &port->old_port_state);
			outp(port->com_IER, 0);
			if (SetBaud (port, baud) == 0)
			{
				if (ConfigurePort (port, data_length, parity, stop_bits) == 0)
				{
					/* Determine if we have a 16550A or better */
#ifdef SUPPORT16550
					outp (port->com_FCR, FIFO_enable | rec_FIFO_res | xmt_FIFO_res | rec_FIFO_trig_14);
					if (inp(port->com_IIR) & FIFO_ENABLED)
						port->UARTType = UART16550;
					else
					{
						port->UARTType = UART8250;
						outp (port->com_FCR, 0);
					}
#endif
#ifndef SUPPORT16550
					outp (port->com_FCR, 0);
					port->UARTType = UART8250;
#endif
					
					/* Find an unsused ISR */
					port->ISR_num = 0xff;
					for (x = 0; x < 4 && port->ISR_num == 0xff; x++)
						if (port_ISRs[x] == NULL)
						{
							port->ISR_num = x;
							port_ISRs[x] = port;
						}
						
						if (port->ISR_num != 0xff)
						{
							/* Clear pending interrupts */
							port->com_status = inp(port->com_MSR);
							while (inp(port->com_LSR) & LSR_RBF)
								inp(port->com_RBF);
							while(!(inp(port->com_LSR) & LSR_THRE));
							inp (port->com_IIR);
							
							/* Enable the interrupts */
							_disable();
							_dos_setvect (port->port_interrupt + 0x08, ISRs[port->ISR_num]);
							outp(port->com_IER, IER_ERBFI + IER_ELSI + IER_EDSSI);
							outp (PIC_IMR, inp(PIC_IMR) & ~(0x01 << port->port_interrupt));
							_enable();
							
							outp(port->com_MCR, MCR_GP02 + MCR_GP01 + MCR_RTS + MCR_DTR);
						}
						else
							error_opening = TRUE;
				}
				else
					error_opening = TRUE;
			}
			else
				error_opening = TRUE;
		}
		else
			error_opening = TRUE;
	}
	else
		error_opening = TRUE;
	
	if (error_opening == TRUE)
	{
		if (port->rbuff != NULL)
		{
			DestroyByteQueue (port->rbuff);
			if (port->sbuff != NULL)
			{
				DestroyByteQueue (port->sbuff);
				RestorePortState(port, &port->old_port_state);
			}
		}
		free (port);
		port = NULL;
	}
	return port;
}

/******************************************************************************\
 This function closes the port which entails turning off interrupts, restoring
 the old interrupt vector, and freeing the memory for the buffer.
\******************************************************************************/

void CloseSerial(s_com_port **port_ptr, BOOLN drop_dtr)
{
	s_com_port *port = *port_ptr;
	
	RestorePortState(port, &port->old_port_state);
	port_ISRs[port->ISR_num] = NULL;
	if (drop_dtr == TRUE)
		DropDTR (port);
	outp (port->com_FCR, 0);
	DestroyByteQueue (port->rbuff);
	DestroyByteQueue (port->sbuff);
	free (port);
	*port_ptr = NULL;
}

/******************************************************************************\
\******************************************************************************/

BOOLN SerialStatusChanged (s_com_port *port)
{
	BOOLN temp;
	
	port->comm_error = 0;
	temp = port->modem_status_changed;
	port->modem_status_changed = FALSE;
	return temp;
}

/******************************************************************************\
\******************************************************************************/

int CarrierDetected (s_com_port *port)
{
	port->comm_error = 0;
	return port->com_status & MSR_DCD;
}

/******************************************************************************\
\******************************************************************************/

int DataSetReady (s_com_port *port)
{
	port->comm_error = 0;
	return port->com_status & MSR_DSR;
}

/******************************************************************************\
\******************************************************************************/

int ClearToSend (s_com_port *port)
{
	port->comm_error = 0;
	return port->com_status & MSR_CTS;
}

/******************************************************************************\
To turn on XON/XOFF flow control for a port pass the port # and 0 to turn off
XON/XOFF or anything else to turn it on.
\******************************************************************************/

void SetXonXoff (s_com_port *port, BOOLN setting)
{
	port->comm_error = 0;
	port->xon_xoff_enabled = setting;
	if (setting == FALSE)
	{
		port->input_blocked_by_flow_control = port->input_blocked_by_cts;
		port->output_blocked_by_xoff = FALSE;
	}
}

/******************************************************************************\

\******************************************************************************/

BOOLN GetXonXoff (s_com_port *port)
{
	port->comm_error = 0;
	return port->xon_xoff_enabled;
}

/******************************************************************************\
To turn on RTS/CTS flow control for a port pass the port # and 0 to turn off
RTS/CTS or anything else to turn it on.
\******************************************************************************/

void SetRtsCts (s_com_port *port, BOOLN setting)
{
	port->comm_error = 0;
	port->rts_cts_enabled = setting;
	if (setting == FALSE)
	{
		port->input_blocked_by_flow_control = port->input_blocked_by_xoff;
		port->output_blocked_by_cts = FALSE;
		outp(port->com_MCR, inp(port->com_MCR) | MCR_RTS);
	}
	else
	{
		if (port->com_status & MSR_CTS)
			port->output_blocked_by_cts = FALSE;
		else
			port->output_blocked_by_cts = TRUE;
		if (port->rbuff->count < port->rec_buff_maxchars)
			outp(port->com_MCR, inp(port->com_MCR) | MCR_RTS);
		else
		{
			outp(port->com_MCR, inp(port->com_MCR) & OPP_MCR_RTS);
			port->input_blocked_by_flow_control = port->input_blocked_by_cts = TRUE;
		}
	}
}

/******************************************************************************\

\******************************************************************************/

BOOLN GetRtsCts (s_com_port *port)
{
	port->comm_error = 0;
	return port->rts_cts_enabled;
}

/******************************************************************************\
Returns true if input to a port is being blocked by some type of flow control.
\******************************************************************************/

BOOLN GetInputFlowStatus (s_com_port *port)
{
	port->comm_error = 0;
	return port->input_blocked_by_flow_control;
}

/******************************************************************************\
Returns true if input to a port has started or stopped being blocked by flow
control since the last time this routine was called.
\******************************************************************************/

BOOLN GetInputFlowChanged (s_com_port *port)
{
	BOOLN stat;
	
	port->comm_error = 0;
	stat = port->input_flow_changed;
	port->input_flow_changed = FALSE;
	return stat;
}

/******************************************************************************\
Returns true if output from a port is being blocked by some type of flow
control.
\******************************************************************************/

BOOLN GetOutputFlowStatus (s_com_port *port)
{
	port->comm_error = 0;
	return port->output_blocked_by_xoff | port->output_blocked_by_cts;
}

/******************************************************************************\
Flushes a ports input buffer.
\******************************************************************************/

void FlushPortInputBuffer (s_com_port *port)
{
	port->comm_error = 0;
	FlushQueue (port->rbuff);
}

/******************************************************************************\
Flushes a ports output buffer.
\******************************************************************************/

void FlushPortOutputBuffer (s_com_port *port)
{
	port->comm_error = 0;
	FlushQueue (port->sbuff);
}

/******************************************************************************\

  Routine: DropDTR
  
	Function: Drops the DTR signal.
	
	  Pass: A handle to the port the DTR should be dropped on.
	  
		Return: Nothing
		
\******************************************************************************/

void DropDTR (s_com_port *port)
{
	port->comm_error = 0;
	outp(port->com_MCR, inp(port->com_MCR) & OPP_MCR_DTR);
}

/******************************************************************************\

  Routine: RaiseDTR
  
	Function: Raises the DTR signal.
	
	  Pass: A handle to the port the DTR should be raised on.
	  
		Return: Nothing
		
\******************************************************************************/

void RaiseDTR (s_com_port *port)
{
	port->comm_error = 0;
	outp(port->com_MCR, inp(port->com_MCR) | MCR_DTR);
}

/******************************************************************************\

  Routine: GetDTR
  
	Function: Raises the DTR signal.
	
	  Pass: A handle to the port the DTR should be raised on.
	  
		Return: Nothing
		
\******************************************************************************/

int GetDTR (s_com_port *port)
{
	port->comm_error = 0;
	return inp(port->com_MCR) & MCR_DTR;
}

/******************************************************************************\

  Routine: GetRTS
  
	Function:
	
	  Pass:
	  
		Return: Nothing
		
\******************************************************************************/

int GetRTS (s_com_port *port)
{
	port->comm_error = 0;
	return inp(port->com_MCR) & MCR_RTS;
}

/******************************************************************************\

  Routine: InputSerial
  
	Function:
	
	  Pass:
	  
		Return:
		
\******************************************************************************/

void InputSerial (s_com_port *port, char *string, int length, long timeout, BOOLN echo)
{
	int x;
	char ch = 0;
	long timer;
	
	port->comm_error = 0;
	SerialStringWrite (port, string);
	x = strlen(string);
	timer = StartTimer();
	do
	{
		while (!ReadySerial(port) && TimerValue(timer) < timeout);
		ch = (char)SerialRead(port);
		if (ch == 13 || TimerValue(timer) >= timeout)
		{
			ch = 0;
			*(string + x) = 0;
		}
		else if (ch == 8)
		{
			if (x > 0)
			{
				x--;
				if (echo == TRUE)
					SerialStringWrite (port, "\x08 \x08");
			}
			else if (echo == TRUE)
				SerialWrite (port, 0x07);
		}
		else if (x < length && ch != 10)
		{
			*(string + x++) = ch;
			if (ch != 0 && echo == TRUE)
				SerialWrite (port, ch);
		}
		else if (echo == TRUE)
			SerialWrite (port, 0x07);
	} while (ch != 0);
}

/******************************************************************************\
\******************************************************************************/

int GetCommError (s_com_port *port)
{
	int retval;
	
	retval = port->comm_error;
	port->comm_error = 0;
	return retval;
}

/******************************************************************************\
\******************************************************************************/

void SetLoopMode (s_com_port *port)
{
	outp(port->com_MCR, inp(port->com_MCR) | MCR_LOOP);
}

/******************************************************************************\
\******************************************************************************/

void ResetLoopMode (s_com_port *port)
{
	outp(port->com_MCR, inp(port->com_MCR) & OPP_MCR_LOOP);
}

/******************************************************************************\
\******************************************************************************/

BOOLN LoopModeSet (s_com_port *port)
{
	if (inp(port->com_MCR) & MCR_LOOP)
		return TRUE;
	else
		return FALSE;
}

/******************************************************************************\

  Routine: FramingErr
  
	Function: Determine if a hardware error has been detected.
	
	  Pass: A handle to the port to check.
	  
		Return: TRUE if a framing error has been detected, otherwise FALSE.
		
\******************************************************************************/

BOOLN HWError (s_com_port *port)
{
	if (port->hw_error == 0)
		return FALSE;
	else
		return TRUE;
}

/******************************************************************************\

  Routine: ClrHWError
  
	Function: Clear the flag which indicates whether a hardware error has occured.
	
	  Pass: A handle to the port to check.
	  
		Return: Nothing
		
\******************************************************************************/

void ClrHWError (s_com_port *port)
{
	port->hw_error = 0;
}

/******************************************************************************\

  Routine: OverrunErr
  
	Function: Determine if a overrun error has been detected since the last call to
	this function.
	
	  Pass: A handle to the port to check.
	  
		Return: TRUE if a overrun error has been detected, otherwise FALSE.
		
\******************************************************************************/

BOOLN OverrunErr (s_com_port *port)
{
	if (port->hw_error & LSR_OE)
	{
		port->hw_error |= OPP_LSR_OE;
		return TRUE;
	}
	else
		return FALSE;
}

/******************************************************************************\

  Routine: ParityErr
  
	Function: Determine if a parity error has been detected since the last call to
	this function.
	
	  Pass: A handle to the port to check.
	  
		Return: TRUE if a parity error has been detected, otherwise FALSE.
		
\******************************************************************************/

BOOLN ParityErr (s_com_port *port)
{
	if (port->hw_error & LSR_PE)
	{
		port->hw_error |= OPP_LSR_PE;
		return TRUE;
	}
	else
		return FALSE;
}

/******************************************************************************\

  Routine: FramingErr
  
	Function: Determine if a framing error has been detected since the last call to
	this function.
	
	  Pass: A handle to the port to check.
	  
		Return: TRUE if a framing error has been detected, otherwise FALSE.
		
\******************************************************************************/

BOOLN FramingErr (s_com_port *port)
{
	if (port->hw_error & LSR_FE)
	{
		port->hw_error |= OPP_LSR_FE;
		return TRUE;
	}
	else
		return FALSE;
}

