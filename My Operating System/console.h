#pragma once

extern void InitializeConsole();
extern void stdout_flush();
extern void ConsolePutc(unsigned char c);
extern uint8_t ConsoleSetColor(const uint8_t c);
extern void ConsoleClrScr(const uint8_t color);
extern void ConsoleGotoXY(uint8_t x, uint8_t y);
extern uint8_t ConsoleSetColor(const uint8_t color);
