#pragma once

#include "idt.h"

/* Initialize HAL */
extern int _cdecl HAL_Initialize();
/* Shutdown HAL */
extern int _cdecl HAL_Shutdown();
/* Signal that an interrupt handler is finished and we need to send EOI */
extern void _cdecl HARDWARE_INTERRUPT_DONE(unsigned int intno);

/* Install an interrupt handler */
extern void _cdecl InstallInterruptHandler(uint8_t IRQNumber, void (_cdecl *handler)(void));
extern uint32_t _cdecl GetTickCount();
