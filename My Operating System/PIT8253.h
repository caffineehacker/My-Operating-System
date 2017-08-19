#pragma once

#define PIT_COUNTER_0_ADDR 0x40
#define PIT_COUNTER_1_ADDR 0x41
#define PIT_COUNTER_2_ADDR 0x42
#define PIT_CONTROLWORD_ADDR 0x43

/* PIT CONTROLWORD
	*  Bit 0: (BCP) Binary Counter
          o 0: Binary
          o 1: Binary Coded Decimal (BCD)
*/
/* BCP (Binary Counter) */
#define PIT_CONTROLWORD_BCP_MASK 0x1

/* Mode 0: Interrupt or Terminal Count */
#define PIT_CONTROLWORD_OPERATINGMODE_INTERRUPT_MASK 0x0
/* Programmable one-shot */
#define PIT_CONTROLWORD_OPERATINGMODE_ONESHOT_MASK 0x2
/* Rate Generator */
#define PIT_CONTROLWORD_OPERATINGMODE_RATEGEN_MASK 0x4
/* Mode 3: Square Wave Generator */
#define PIT_CONTROLWORD_OPERATINGMODE_SQWAVE_MASK 0x6
/* Mode 4: Software Triggered Strobe */
#define PIT_CONTROLWORD_OPERATINGMODE_SOFTSTROBE_MASK 0x8
/* Mode 5: Hardware Triggered Strobe */
#define PIT_CONTROLWORD_OPERATINGMODE_HARDSTROBE_MASK 0xA

/* Bits 4-5: (RL0, RL1) Read/Load Mode. We are going to read or send data to a counter register */
/* Counter value is latched into an internal control register at the time of the I/O write operation. */
#define PIT_CONTROLWORD_RLMODE_LATCH_MASK 0x00
/* Read or Load Least Significant Byte (LSB) only */
#define PIT_CONTROLWORD_RLMODE_LSB_MASK 0x10
/* Read or Load Most Significant Byte (MSB) only */
#define PIT_CONTROLWORD_RLMODE_MSB_MASK 0x20
/* Read or Load LSB first then MSB */
#define PIT_CONTROLWORD_RLMODE_LSBMSB_MASK 0x30

/* Bits 6-7: (SC0-SC1) Select Counter. See above sections for a description of each. */
/* Counter 0 */
#define PIT_CONTROLWORD_SELECTCOUNTER_0_MASK 0x00
/* Counter 1 */
#define PIT_CONTROLWORD_SELECTCOUNTER_1_MASK 0x40
/* Counter 2 */
#define PIT_CONTROLWORD_SELECTCOUNTER_2_MASK 0x80

extern void pit_initialize(uint8_t irq, uint8_t irCodeSeg);
extern void pit8253_irq();
extern void pit_start_counter0(uint32_t freq, uint8_t mode);
extern uint32_t GetSystemTicks();
extern void sleep_ms(int ms);