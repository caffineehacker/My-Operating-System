#pragma once

#include <cstdint>

extern void __cdecl outb(uint16_t port, uint8_t val);
extern uint8_t __cdecl inb(uint16_t port);
