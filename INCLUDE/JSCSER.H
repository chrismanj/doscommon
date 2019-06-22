/* Error constants */

#define TIMEOUT 1
#define UNDERFLOW 2

/* registers in UART */

#define SER_RBF 0 /* the read buffer */
#define SER_THR 0 /* the write buffer */
#define SER_DL 0  /* the baud rate divisor (WORD) */
#define SER_IER 1 /* the int. enable register */
#define SER_IIR 2 /* the int. identification register */
#define SER_FCR 2 /* FIFO conrtol register */
#define SER_LCR 3 /* control data config. and divisor latch */
#define SER_MCR 4 /* modem control reg. */
#define SER_LSR 5 /* line status reg. */
#define SER_MSR 6 /* modem status of cts, ring etc. */

/* bit patterns for interrupt enable register */

#define IER_ERBFI 0x01 /* Enable Receiver Buffer Full Interrupt */
#define IER_ETBEI 0x02 /* Enable Transmitter Buffer Empty Interrupt */
#define IER_ELSI 0x04  /* Enable Line Status Interrupt */
#define IER_EDSSI 0x08 /* Enable Delta Status Signals Interrupt */

/* bit patterns for interrupt identification register */

#define PENDING_INT 0x01
#define MODEM_STATUS_INT 0x00
#define THR_EMPTY_INT 0x02
#define RBF_FULL_INT 0x04
#define ERROR_INT 0x06
#define FIFO_ENABLED 0x40

/* bit patterns for line control register */

#define DATA_BITS_5 0x00
#define DATA_BITS_6 0x01
#define DATA_BITS_7 0x02
#define DATA_BITS_8 0x03

#define STOP_BITS_1 0x00
#define STOP_BITS_2 0x04

#define PARITY_NONE 0x00
#define PARITY_ODD 0x08
#define PARITY_EVEN 0x18
#define PARITY_MARK 0x28
#define PARITY_SPACE 0x38
#define LCR_SBR 0x40
#define LCR_DLAB 0x80 /* used to turn reg 0,1 into divisor latch */

/* bit patterns for modem control register */

#define MCR_DTR 0x01
#define OPP_MCR_DTR 0xfe
#define MCR_RTS 0x02
#define OPP_MCR_RTS 0xfd
#define MCR_GP01 0x04
#define MCR_GP02 0x08 /* enable interrupt */
#define MCR_LOOP 0x10
#define OPP_MCR_LOOP 0xef

/* bit patterns for line status register */

#define LSR_RBF 0x01 /* Receive buffer has chars in it */
#define LSR_OE 0x02  /* Overrun error occured */
#define OPP_LSR_OE 0xfd
#define LSR_PE 0x04 /* Parity error occured */
#define OPP_LSR_PE 0xfb
#define LSR_FE 0x08 /* Framing error occured */
#define OPP_LSR_FE 0xf7
#define LSR_BD 0x10       /* Break detected */
#define LSR_THRE 0x20     /* Transmit Holding Register Empty */
#define LSR_TEMT 0x40     /* Transmitter empty */
#define LSR_ERR16550 0x80 /* At least on char in receive buffer errored */

/* bit patterns for modem status register */

#define DELTA_MSR_CTS 0x01
#define DELTA_MSR_DSR 0x02
#define DELTA_MSR_RI 0x04
#define DELTA_MSR_DCD 0x08
#define MSR_CTS 0x10
#define MSR_DSR 0x20
#define MSR_RI 0x40
#define MSR_DCD 0x80

/* bit patterns for FIFO control register */

#define FIFO_enable 0x01
#define rec_FIFO_res 0x02
#define xmt_FIFO_res 0x04
#define rec_FIFO_trig_1 0x00
#define rec_FIFO_trig_4 0x40
#define rec_FIFO_trig_8 0x80
#define rec_FIFO_trig_14 0xc0

#define PIC_IMR 0x21 /* pic's interrupt mask reg. */
#define PIC_ICR 0x20 /* pic's interupt control reg. */

#define UART16550 0
#define UART8250 1

#define s_port_state struct port_state_str
struct port_state_str
{
   int IER;
   int LCR;
   int MCR;
   WORD DL;
   void(_interrupt _far *Old_ISR)();
   int int_enabled;
};

#define s_com_port struct com_port_str
struct com_port_str
{
   /* Pointer to receive buffer */
   SBQueue *rbuff;
   /* Pointer to send buffer */
   SBQueue *sbuff;
   /* True means XON/XOFF enabled for this port */
   BOOLN xon_xoff_enabled;
   /* True means RTS/CTS enabled for a given port */
   BOOLN rts_cts_enabled;
   /* True means output is currently being blocked by XON/XOFF flow control to
     this port */
   BOOLN output_blocked_by_xoff;
   /* True means output is currently being blocked by RTS/CTS flow control to
     this port */
   BOOLN output_blocked_by_cts;
   /* # chars that must be in this port's buffer before flow control is used to
     block incoming data */
   WORD rec_buff_maxchars;
   /* # chars that must be in this port's buffer before flow control stops
     blocking incoming data */
   WORD rec_buff_minchars;
   /* True means input is currently being blocked by XON/XOFF flow control to
     this port */
   BOOLN input_blocked_by_xoff;
   /* True means input is currently being blocked by RTS/CTS flow control to
     this port */
   BOOLN input_blocked_by_cts;
   /* True means some type of flow control is being used to block incoming data
     to this port */
   BOOLN input_blocked_by_flow_control;
   /* Flag to indicate whether or not the flow control has been engaged or
     disengaged since last read */
   BOOLN input_flow_changed;
   /* Port ISR number (0 - (MAXPORTS - 1)) */
   int ISR_num;
   /* Port interrupt number (Currently only supports IRQs 0 - 7) */
   int port_interrupt;
   /* State of COM ports before this program got hold of 'em */
   s_port_state old_port_state;
   /* Some routines set this variable if there was an error */
   int comm_error;
   /* This variable is used to keep track of errors signaled by the chip */
   int hw_error;
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
   WORD com_RBF;
   WORD com_THR;
   WORD com_MCR;
   WORD com_LSR;
   WORD com_MSR;
   WORD com_LCR;
   WORD com_DL;
   WORD com_IER;
   WORD com_IIR;
   WORD com_FCR;
   /* The value of the com ports MSR register */
   int com_status;
   /* Set to TRUE when the MSR changes */
   BOOLN modem_status_changed;
   /* True means the xmit interrupt has been stopped, false means the xmit
     interrupt is still going */
   BOOLN xmit_interrupt_stopped;
};

void SetPortAddressAndIRQ(s_com_port *, WORD, WORD);
WORD ReadySerial(s_com_port *);
BYTE SerialRead(s_com_port *);
BYTE SerialReadWTimeout(s_com_port *);
int SerialWrite(s_com_port *, BYTE);
int SerialStringWrite(s_com_port *, char *);
int ConfigurePort(s_com_port *, WORD, WORD, WORD);
int SetBaud(s_com_port *, DWORD);
void SavePortState(s_com_port *, s_port_state *);
void RestorePortState(s_com_port *, s_port_state *);
s_com_port *OpenSerial(WORD, int, long, int, int, int, WORD, WORD);
void CloseSerial(s_com_port **, BOOLN);
void SetXonXoff(s_com_port *, BOOLN);
BOOLN GetXonXoff(s_com_port *);
void SetReadTimeout(s_com_port *, long);
long GetReadTimeout(s_com_port *);
void SetRtsCts(s_com_port *, BOOLN);
BOOLN GetRtsCts(s_com_port *);
BOOLN SerialStatusChanged(s_com_port *);
int CarrierDetected(s_com_port *);
int DataSetReady(s_com_port *);
int ClearToSend(s_com_port *);
int GetCommError(s_com_port *);
BOOLN GetInputFlowStatus(s_com_port *);
BOOLN GetOutputFlowStatus(s_com_port *);
BOOLN GetInputFlowChanged(s_com_port *);
void FlushPortInputBuffer(s_com_port *);
void FlushPortOutputBuffer(s_com_port *);
void DropDTR(s_com_port *);
void RaiseDTR(s_com_port *);
int GetDTR(s_com_port *);
int GetRTS(s_com_port *);
void InputSerial(s_com_port *, char *, int, long, BOOLN);
BYTE WaitForCharSerial(s_com_port *, BYTE);
BYTE WaitForCharsSerial(s_com_port *, char *);
void SetLoopMode(s_com_port *);
void ResetLoopMode(s_com_port *);
BOOLN LoopModeSet(s_com_port *);
BOOLN HWError(s_com_port *);
void ClrHWError(s_com_port *);
BOOLN OverrunErr(s_com_port *);
BOOLN ParityErr(s_com_port *);
BOOLN FramingErr(s_com_port *);
