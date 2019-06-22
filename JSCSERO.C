#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#include "..\common\include\types.h"
#include "..\common\include\debug.h"
#include "..\common\include\jsctime.h"
#include "..\common\include\jscser.h"
#include "..\common\include\bqueue.h"

#define XOFF 19
#define XON 17

/* GLOBALS */

#define s_com_port struct com_port_struct

struct com_port_struct
{
  /* Pointer to receive buffer */
  s_bqueue *rbuff;
  /* Pointer to send buffer */
  s_bqueue *sbuff;
  /* True means XON/XOFF enabled for this port */
  bool xon_xoff_enabled;
  /* True means RTS/CTS enabled for a given port */
  bool rts_cts_enabled;
  /* True means output is currently being blocked by XON/XOFF flow control to
     this port */
  bool output_blocked_by_xoff;
  /* True means output is currently being blocked by RTS/CTS flow control to
     this port */
  bool output_blocked_by_cts;
  /* # chars that must be in this port's buffer before flow control is used to
     block incoming data */
  word rec_buff_maxchars;
  /* # chars that must be in this port's buffer before flow control stops
     blocking incoming data */
  word rec_buff_minchars;
  /* True means some type of flow control is being used to block incoming data
     to this port */
  bool input_blocked_by_flow_control;
  /* Flag to indicate whether or not the flow control has been engaged or
     disengaged since last read */
  bool input_flow_changed;
  /* Pointers to interrupt routine */
  void(_interrupt _far *ISR)();
  /* Port interrupt number (Currently only supports IRQs 0 - 7) */
  int port_inter;
  /* State of COM ports before this program got hold of 'em */
  s_port_state old_port_state;
  /* Some routines set this variable if there was an error */
  int comm_error;
  /* Time in 1/100s of a second to wait for incoming data before giving up */
  long read_timeout;
  /* Type of UART (UART16550 or UART8250)*/
  int UARTType;
  /* Com port registers. I store the addresses here for speed. It is faster
     to access say, the RBF register, as port->com_RBF rather than
     the way most other routines I've seen do it which is usually something
     like port_address + com_RBF (since this is an object it would have to be
     done more like port->address + com_RBF). Eliminating the addition gives
     us a little more speed. */
  word com_RBF;
  word com_THR;
  word com_MCR;
  word com_LSR;
  word com_MSR;
  word com_LCR;
  word com_DL;
  word com_IER;
  word com_IIR;
  word com_FCR;
  /* The value of the com ports MSR register */
  word com_status;
  /* Set to TRUE when the MSR changes */
  bool modem_status_changed;
  /* True means the xmit interrupt has been stopped, false means the xmit
     interrupt is still going */
  bool xmit_interrupt_stopped;
};

/* Function prototypes for static functions */
static void Serial_ISR(word);

/******************************************************************************\
 These are the interrupt service routines for the com ports. They get the next
 character out of the receive buffer register 0 and places it into the software
 buffer.
\******************************************************************************/

static void _interrupt _far Serial_ISR0(void)
{
  Serial_ISR(port0);
}

static void _interrupt _far Serial_ISR1(void)
{
  Serial_ISR(port1);
}

static void _interrupt _far Serial_ISR2(void)
{
  Serial_ISR(port2);
}

static void _interrupt _far Serial_ISR3(void)
{
  Serial_ISR(port3);
}

static void KickStartXmitInt(word port)
{
  outp(com_THR[port], Dequeue(sbuff[port]));
  outp(com_IER[port], IER_ERBFI + IER_ETBEI + IER_ELSI + IER_EDSSI);
  xmit_interrupt_stopped[port] = FALSE;
}

static void Serial_ISR(word port)
{
  int ch, x;

  while (1)
  {
    ch = inp(com_IIR[port]) & 0x07; /* Find out what caused the interrupt */

    switch (ch)
    {
    case ERROR_INT:
      ch = inp(com_LSR[port]);
      break;

    case RBF_FULL_INT:

      ch = inp(com_RBF[port]);
      if (xon_xoff_enabled[port] == TRUE)
      {
        if (ch == XON)
        {
          if (xmit_interrupt_stopped[port] == TRUE && output_blocked_by_xoff[port] == TRUE && output_blocked_by_cts[port] == FALSE && sbuff[port]->count)
            KickStartXmitInt(port);
          output_blocked_by_xoff[port] = FALSE;
          break;
        }
        if (ch == XOFF)
        {
          output_blocked_by_xoff[port] = TRUE;
          outp(com_IER[port], IER_ERBFI + IER_ELSI + IER_EDSSI);
          xmit_interrupt_stopped[port] = TRUE;
          break;
        }
      }
      Enqueue(rbuff[port], (char)ch);
      if (UARTType[port] == UART16550)
        while (inp(com_LSR[port]) & LSR_RBF)
          Enqueue(rbuff[port], (char)inp(com_RBF[port]));
      if (rbuff[port]->count == rec_buff_maxchars[port])
      {
        if (xon_xoff_enabled[port] == TRUE)
        {
          while (!(inp(com_LSR[port]) & LSR_THRE))
            ;                        /* Wait until xmit buffer empty */
          outp(com_THR[port], XOFF); /* send xoff */
          input_blocked_by_flow_control[port] = input_flow_changed[port] = TRUE;
        }

        if (rts_cts_enabled[port] == TRUE)
        {
          outp(com_MCR[port], MCR_GP02 + MCR_GP01 + MCR_DTR);
          input_blocked_by_flow_control[port] = input_flow_changed[port] = TRUE;
        }
      }
      break;

    case THR_EMPTY_INT:
      if (UARTType[port] == UART16550)
      {
        for (x = 0; x < 16 && sbuff[port]->count; x++)
          outp(com_THR[port], Dequeue(sbuff[port]));
      }
      else
        outp(com_THR[port], Dequeue(sbuff[port]));
      if (!sbuff[port]->count)
      {
        outp(com_IER[port], IER_ERBFI + IER_ELSI + IER_EDSSI);
        xmit_interrupt_stopped[port] = TRUE;
      }
      break;

    case MODEM_STATUS_INT:
      com_status[port] = inp(com_MSR[port]);
      if (rts_cts_enabled[port] == TRUE)
      {
        if (com_status[port] & MSR_CTS)
        {
          if (xmit_interrupt_stopped[port] == TRUE && output_blocked_by_xoff[port] == FALSE && output_blocked_by_cts[port] == TRUE && sbuff[port]->count)
            KickStartXmitInt(port);
          output_blocked_by_cts[port] = FALSE;
        }
        else
        {
          outp(com_IER[port], IER_ERBFI + IER_ELSI + IER_EDSSI);
          output_blocked_by_cts[port] = xmit_interrupt_stopped[port] = TRUE;
        }
      }
      modem_status_changed[port] = TRUE;
      break;

    default:
      outp(PIC_ICR, 0x20); /* Restore PIC */
      return;
    }
  }
}

/******************************************************************************\
 Initialize serial routines. *This routine MUST be called* before the serial
 routines can be used.
\******************************************************************************/

void InitSerial(void)
{
  SetPortAddressAndIRQ(0, 0x3f8, 4);
  SetPortAddressAndIRQ(1, 0x2f8, 3);

  ISR[0] = Serial_ISR0;
  ISR[1] = Serial_ISR1;

  xon_xoff_enabled[0] = FALSE;
  xon_xoff_enabled[1] = FALSE;

  rts_cts_enabled[0] = FALSE;
  rts_cts_enabled[1] = FALSE;
}

/******************************************************************************\
 Sets a ports address and IRQ (0 - 7). This routine should not be called to
 change the IRQ of an open serial port.
\******************************************************************************/

void SetPortAddressAndIRQ(word port, word new_address, word new_interrupt)
{
  port_inter[port] = new_interrupt;
  com_RBF[port] = new_address + SER_RBF;
  com_THR[port] = new_address + SER_THR;
  com_MCR[port] = new_address + SER_MCR;
  com_LSR[port] = new_address + SER_LSR;
  com_MSR[port] = new_address + SER_MSR;
  com_LCR[port] = new_address + SER_LCR;
  com_DL[port] = new_address + SER_DL;
  com_IER[port] = new_address + SER_IER;
  com_IIR[port] = new_address + SER_IIR;
  com_FCR[port] = new_address + SER_FCR;
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

word ReadySerial(word port)
{
  return rbuff[port]->count;
}

/******************************************************************************\
  This function reads a character from the circulating buffer and returns it
  to the caller. It returns 0 if there are no characters in the buffer.
\******************************************************************************/

byte SerialRead(word port)
{
  if (rbuff[port]->count) /* Test for presence of character(s) in buffer */
  {
    byte ch = Dequeue(rbuff[port]);

    /* Test to see if input to a port is currently being blocked by flow
       control. If so, see if the buffer is empty enough to stop blocking
       input. */

    if (rbuff[port]->count == rec_buff_minchars[port] && input_blocked_by_flow_control[port] == TRUE)
    {
      if (xon_xoff_enabled[port] == TRUE)
        SerialWrite(port, XON);
      if (rts_cts_enabled[port] == TRUE)
        outp(com_MCR[port], MCR_GP02 + MCR_GP01 + MCR_RTS + MCR_DTR);
      input_blocked_by_flow_control[port] = FALSE;
      input_flow_changed[port] = TRUE;
    }
    return ch;
  }
  else
    return 0; /* buffer was empty return a NULL */
}

/******************************************************************************\
\******************************************************************************/

void SetReadTimeout(word port, long timeout_val)
{
  read_timeout[port] = timeout_val;
}

/******************************************************************************\
\******************************************************************************/

byte SerialReadWTimeout(word port)
{
  long timer;

  timer = StartTimer();
  while (!ReadySerial(port) && (TimerValue(timer) < read_timeout[port]))
    ;
  if (TimerValue(timer) >= read_timeout[port])
  {
    comm_error[port] = 1;
    return 0;
  }
  else
    return SerialRead(port);
}

/******************************************************************************\
\******************************************************************************/

byte WaitForCharSerial(word port, byte char_to_wait_for)
{
  byte ch = ~char_to_wait_for;
  long timer = StartTimer();

  while ((ch != char_to_wait_for) && (TimerValue(timer) < read_timeout[port]))
    ch = SerialReadWTimeout(port);
  if ((TimerValue(timer) >= read_timeout[port]) && (ch != char_to_wait_for))
    comm_error[port] = 1;
  return ch;
}

/******************************************************************************\
\******************************************************************************/

byte WaitForCharsSerial(word port, char *chars)
{
  byte ch;
  long timer = StartTimer();
  bool found = FALSE;
  char *cur_char;

  while (found == FALSE && (TimerValue(timer) < read_timeout[port]))
  {
    ch = SerialReadWTimeout(port);
    cur_char = chars;
    while (*cur_char != 0 && found == FALSE)
      if (ch == *cur_char++)
        found = TRUE;
  }
  if (TimerValue(timer) >= read_timeout[port])
  {
    comm_error[port] = 1;
    return 0;
  }
  return ch;
}

/******************************************************************************\
 This function writes a character to the transmit buffer. If the transmit buffer
 is full upon entry, it will wait 1/2 a second for a byte to be freed.
\******************************************************************************/

int SerialWrite(word port, byte ch)
{
  long timer;

  timer = StartTimer();
  /* Check to see if the send buffer is full, if it is wait 1/2 a second for a
     spot to be freed */
  while ((sbuff[port]->count == sbuff[port]->size) && (TimerValue(timer) <= 50L))
    ;
  if (TimerValue(timer) > 50L)
    return 1;

  /* Place character in send buffer */
  _disable();
  Enqueue(sbuff[port], ch);
  if (xmit_interrupt_stopped[port] == TRUE && output_blocked_by_xoff[port] == FALSE && output_blocked_by_cts[port] == FALSE)
    KickStartXmitInt(port);
  _enable();
  return 0;
}

/******************************************************************************\
 This function writes a string to the transmit buffer, but first it
 waits for the transmit buffer to be empty.  Note: it is not interrupt
 driven and it turns off interrupts while it's working.
\******************************************************************************/

int SerialStringWrite(word port, char *string)
{
  int retval = 0;

  while (*string && retval == 0)
    retval = SerialWrite(port, *string++);
  return retval;
}

/******************************************************************************\
 Configures the data length, parity & stop bits for a port.

 Pass: Port
       Data Length: 0 = 5 bits per data word
        1 = 6 bits per data word
        2 = 7 bits per data word
        3 = 8 bits per data word

       Parity: 0 = None
         1 = Odd
         2 = Even
         3 = Mark
         4 = Space

       Stop Bits: 0 = 1 stop bit
      1 = 2 stop bits
\******************************************************************************/

int ConfigurePort(word port, word data_length, word parity, word stop_bits)
{
  word configuration;

  configuration = data_length | stop_bits | parity;
  outp(com_LCR[port], configuration);
  if (inp(com_LCR[port]) == configuration)
    return 0;
  else
    return 1;
}

/******************************************************************************\
 Sets the baud rate for a port
\******************************************************************************/

int SetBaud(word port, dword baud)
{
  int old_lcr = inp(com_LCR[port]);
  int retval = 0;
  word brd = (word)((dword)115200 / baud);
  outp(com_LCR[port], LCR_DLAB); /* Turn on divisor latch regs */
  outpw(com_DL[port], brd);
  if (inpw(com_DL[port]) != brd)
    retval = 1;
  outp(com_LCR[port], old_lcr); /* Turn off divisor latch regs */
  return retval;
}

/******************************************************************************\
\******************************************************************************/

void SavePortState(word port, s_port_state *port_state)
{
  port_state->IER = inp(com_IER[port]);
  port_state->LCR = inp(com_LCR[port]);
  port_state->MCR = inp(com_MCR[port]);

  outp(com_LCR[port], LCR_DLAB); /* Turn on divisor latch regs */
  port_state->DL = inpw(com_DL[port]);
  outp(com_LCR[port], port_state->LCR); /* Turn off divisor latch regs */

  port_state->Old_ISR = _dos_getvect(port_inter[port] + 0x08);
  port_state->int_enabled = inp(PIC_IMR) & (0x01 << port_inter[port]);
}

/******************************************************************************\
\******************************************************************************/

void RestorePortState(word port, s_port_state *port_state)
{
  outp(com_IER[port], port_state->IER);
  outp(com_MCR[port], port_state->MCR | MCR_DTR);
  outp(com_LCR[port], LCR_DLAB); /* Turn on divisor latch regs */
  outpw(com_DL[port], port_state->DL);
  outp(com_LCR[port], port_state->LCR);
  _disable();
  _dos_setvect(port_inter[port] + 0x08, port_state->Old_ISR);
  _enable();
  outp(PIC_IMR, inp(PIC_IMR) | port_state->int_enabled);
}

/******************************************************************************\
 This function will open up the serial port, set it's configuration, turn on
 interrupts and load the ISR.
\******************************************************************************/

int OpenSerial(word port, word address, int irq, long baud, int data_length,
               int parity, int stop_bits, word rec_buffsize,
               word send_buffsize)
{
  int retval = 0;

  outp(com_IER[port], 0);

  rbuff[port] = NULL;
  sbuff[port] = NULL;

  if ((rbuff[port] = InitByteQueue(rec_buffsize)) != NULL)
  {
    if ((sbuff[port] = InitByteQueue(send_buffsize)) != NULL)
    {
      output_blocked_by_xoff[port] = FALSE;
      input_blocked_by_flow_control[port] = FALSE;
      input_flow_changed[port] = TRUE;
      rec_buff_minchars[port] = (word)(rec_buffsize / 5);
      rec_buff_maxchars[port] = (word)(rec_buff_minchars[port] * 4);
      comm_error[port] = 0;
      read_timeout[port] = 2000L;

      SetPortAddressAndIRQ(port, address, irq);
      SavePortState(port, &old_port_state[port]);
      if ((retval = SetBaud(port, baud)) == 0)
      {
        if ((retval = ConfigurePort(port, data_length, parity, stop_bits)) == 0)
        {
          /* Determine if we have a 16550A or better */
          outp(com_FCR[port], FIFO_enable | rec_FIFO_res | xmt_FIFO_res | rec_FIFO_trig_14);
          if (inp(com_IIR[port]) & FIFO_ENABLED)
            UARTType[port] = UART16550;
          else
          {
            UARTType[port] = UART8250;
            outp(com_FCR[port], 0);
          }

          /* Enable the interrupts */
          _disable();
          outp(com_MCR[port], MCR_GP02 + MCR_GP01 + MCR_RTS + MCR_DTR);
          outp(com_IER[port], IER_ERBFI + IER_ETBEI + IER_ELSI + IER_EDSSI);
          _dos_setvect(port_inter[port] + 0x08, ISR[port]);
          outp(PIC_IMR, inp(PIC_IMR) & ~(0x01 << port_inter[port]));
          _enable();

          /* Set the status flags */
          com_status[port] = inp(com_MSR[port]);
          modem_status_changed[port] = TRUE;
          xmit_interrupt_stopped[port] = TRUE;
        }
      }
    }
  }
  if (retval != 0)
  {
    if (rbuff[port] != NULL)
    {
      DestroyByteQueue(rbuff[port]);
      if (sbuff[port] != NULL)
      {
        DestroyByteQueue(sbuff[port]);
        RestorePortState(port, &old_port_state[port]);
      }
    }
  }
  return retval;
}

/******************************************************************************\
 This function closes the port which entails turning off interrupts, restoring
 the old interrupt vector, and freeing the memory for the buffer.
\******************************************************************************/

void CloseSerial(word port)
{
  RestorePortState(port, &old_port_state[port]);
  outp(com_FCR[port], 0);
  DestroyByteQueue(rbuff[port]);
  DestroyByteQueue(sbuff[port]);
}

/******************************************************************************\
\******************************************************************************/

bool SerialStatusChanged(word port)
{
  bool temp = modem_status_changed[port];
  modem_status_changed[port] = FALSE;
  return temp;
}

/******************************************************************************\
\******************************************************************************/

int CarrierDetected(word port)
{
  return com_status[port] & MSR_DCD;
}

/******************************************************************************\
\******************************************************************************/

int DataSetReady(word port)
{
  return com_status[port] & MSR_DSR;
}

/******************************************************************************\
\******************************************************************************/

int ClearToSend(word port)
{
  return com_status[port] & MSR_CTS;
}

/******************************************************************************\
 To turn on XON/XOFF flow control for a port pass the port # and 0 to turn off
 XON/XOFF or anything else to turn it on.
\******************************************************************************/

void SetXonXoff(word port, bool setting)
{
  xon_xoff_enabled[port] = setting;
  input_blocked_by_flow_control[port] = FALSE;
  output_blocked_by_xoff[port] = FALSE;
}

/******************************************************************************\

\******************************************************************************/

bool GetXonXoff(word port)
{
  return xon_xoff_enabled[port];
}

/******************************************************************************\
 To turn on RTS/CTS flow control for a port pass the port # and 0 to turn off
 RTS/CTS or anything else to turn it on.
\******************************************************************************/

void SetRtsCts(word port, bool setting)
{
  rts_cts_enabled[port] = setting;
  input_blocked_by_flow_control[port] = FALSE;
  if (com_status[port] & MSR_CTS)
    output_blocked_by_cts[port] = FALSE;
  else
    output_blocked_by_cts[port] = TRUE;
  outp(com_MCR[port], MCR_GP02 + MCR_GP01 + MCR_RTS + MCR_DTR);
}

/******************************************************************************\
 Returns true if input to a port is being blocked by some type of flow control.
\******************************************************************************/

bool GetInputFlowStatus(word port)
{
  return input_blocked_by_flow_control[port];
}

/******************************************************************************\
 Returns true if input to a port has started or stopped being blocked by flow
 control since the last time this routine was called.
\******************************************************************************/

bool GetInputFlowChanged(word port)
{
  bool stat;

  stat = input_flow_changed[port];
  input_flow_changed[port] = FALSE;
  return stat;
}

/******************************************************************************\
 Returns true if output from a port is being blocked by some type of flow
 control.
\******************************************************************************/

bool GetOutputFlowStatus(word port)
{
  return output_blocked_by_xoff[port] | output_blocked_by_cts[port];
}

/******************************************************************************\
 Flushes a ports input buffer.
\******************************************************************************/

void FlushPortInputBuffer(word port)
{
  FlushQueue(rbuff[port]);
}

/******************************************************************************\
 Flushes a ports output buffer.
\******************************************************************************/

void FlushPortOutputBuffer(word port)
{
  FlushQueue(sbuff[port]);
}

/******************************************************************************\

  Routine: DropDTR

 Function: Drops the DTR signal.

     Pass: A handle to the port the DTR should be dropped on.

   Return: Nothing

\******************************************************************************/

void DropDTR(word port)
{
  outp(com_MCR[port], inp(com_MCR[port]) & OPP_MCR_DTR);
}

/******************************************************************************\

  Routine: RaiseDTR

 Function: Raises the DTR signal.

     Pass: A handle to the port the DTR should be raised on.

   Return: Nothing

\******************************************************************************/

void RaiseDTR(word port)
{
  outp(com_MCR[port], inp(com_MCR[port]) | MCR_DTR);
}

/******************************************************************************\

  Routine: InputSerial

 Function:

     Pass:

   Return:

\******************************************************************************/

void InputSerial(word port, char *string, int length, long timeout, bool echo)
{
  int x;
  char ch = 0;
  long timer;

  SerialStringWrite(port, string);
  x = strlen(string);
  timer = StartTimer();
  do
  {
    while (!ReadySerial(port) && TimerValue(timer) < timeout)
      ;
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
          SerialStringWrite(port, "\x08 \x08");
      }
      else if (echo == TRUE)
        SerialWrite(port, 0x07);
    }
    else if (x < length && ch != 10)
    {
      *(string + x++) = ch;
      if (ch != 0 && echo == TRUE)
        SerialWrite(port, ch);
    }
    else if (echo == TRUE)
      SerialWrite(port, 0x07);
  } while (ch != 0);
}

/******************************************************************************\
\******************************************************************************/

int GetCommError(word port)
{
  int retval = comm_error[port];
  comm_error[port] = 0;
  return retval;
}