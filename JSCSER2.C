#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

int detect_IRQ(unsigned base)
{
  /* returns: -1 if no intlevel found, or intlevel 0-15 */
  char ier, mcr, imrm, imrs, maskm, masks, irqm, irqs;

  _disable();                /* disable all CPU interrupts */
  ier = (char)inp(base + 1); /* read IER */
  outp(base + 1, 0);         /* disable all UART ints */
  while (!(inp(base + 5) & 0x20));                        /* wait for the THR to be empty */
  mcr = (char)inp(base + 4); /* read MCR */
  outp(base + 4, 0x0F);      /* connect UART to irq line */
  imrm = (char)inp(0x21);    /* read contents of master ICU mask register */
  imrs = (char)inp(0xA1);    /* read contents of slave ICU mask register */
  outp(0xA0, 0x0A);          /* next read access to 0xA0 reads out IRR */
  outp(0x20, 0x0A);          /* next read access to 0x20 reads out IRR */
  outp(base + 1, 2);         /* let's generate interrupts... */
  maskm = (char)inp(0x20);   /* this clears all bits except for the one */
  masks = (char)inp(0xA0);   /* that corresponds to the int */
  outp(base + 1, 0);         /* drop the int line */
  maskm &= ~inp(0x20);       /* this clears all bits except for the one */
  masks &= ~inp(0xA0);       /* that corresponds to the int */
  outp(base + 1, 2);         /* and raise it again just to be sure... */
  maskm &= (char)inp(0x20);  /* this clears all bits except for the one */
  masks &= (char)inp(0xA0);  /* that corresponds to the int */
  outp(0xA1, ~masks);        /* now let us unmask this interrupt only */
  outp(0x21, ~maskm);
  outp(0xA0, 0x0C);       /* enter polled mode; Mike Surikov reported */
  outp(0x20, 0x0C);       /* that order is important with Pentium/PCI systems */
  irqs = (char)inp(0xA0); /* and accept the interrupt */
  irqm = (char)inp(0x20);
  inp(base + 2);       /* reset transmitter interrupt in UART */
  outp(base + 4, mcr); /* restore old value of MCR */
  outp(base + 1, ier); /* restore old value of IER */
  if (masks)
    outp(0xA0, 0x20); /* send an EOI to slave */
  if (maskm)
    outp(0x20, 0x20); /* send an EOI to master */
  outp(0x21, imrm);   /* restore old mask register contents */
  outp(0xA1, imrs);
  _enable();
  if (irqs & 0x80) /* slave interrupt occured */
    return (irqs & 0x07) + 8;
  if (irqm & 0x80) /* master interrupt occured */
    return irqm & 0x07;
  return -1;
}

int main(int argc, char *argv[])
{
  if (argc > 1)
    printf("%d", detect_IRQ((unsigned)atoi(argv[1])));
  return 0;
}
