#include "idt.h"
#include <cstdio>
#include <cstring>
#include "console.h"
#include "ports.h"
#include "syscall.h"
#include "..\CRT\systemcall.h"

FILE _stdout;
FILE _stdin;
FILE _stderr;

char stdout_buffer[4096];

/* Video memory ptr 
 TODO: Get address from bios
*/
uint16_t *video_memory = (uint16_t *)0xB8000;

/* Current cursor position */
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

/* Current color */
uint8_t	_color=0;

void stdout_flush()
{
	int i = 0;
	for (i = _stdout.bufferStart; i < _stdout.bufferPos; i++)
	{
		ConsolePutc((unsigned char)_stdout.buffer[i]);
	}

	_stdout.bufferPos = 0;
	_stdout.bufferStart = 0;
	_stdout.bufferStartFilePos += i;
}

void* _get_stdout(void* data)
{
	return &_stdout;
}

void InitializeConsole()
{
	_stdout.buffer = stdout_buffer;
	_stdout.bufferLength = 4096;
	_stdout.bufferPos = 0;
	_stdout.bufferStart = 0;
	_stdout.bufferStartFilePos = 0;
	_stdout.flushFunc = stdout_flush;

	SetSyscallFunctionHandler(_get_stdout, SYSCALL_GET_STDOUT);
}

void UpdateCur(uint16_t x, uint16_t y)
{
    /* Get location in words */
    uint16_t cursorLocation = (uint16_t)(y * 80 + x);

	/* send location to vga controller to set cursor */
	/*disable();*/
    //outb(0x3D4, 14);
    //outb(0x3D5, (uint8_t)(cursorLocation >> 8)); /* Send the high byte. */
    //outb(0x3D4, 15);
    //outb(0x3D5, (uint8_t)cursorLocation);      /* Send the low byte. */
	/*enable();*/
}

/* Scroll the screen if needed */
void scroll()
{
	if (cursor_y >= 25)
	{
		uint16_t attribute = (uint16_t)(_color << 8);

		/* Move current display up one line */
		for (int i=0*80; i < 24*80; i++)
			video_memory[i] = video_memory [i+80];

		/* clear the bottom line */
		for (int i=24*80; i<25*80; i++)
			video_memory[i] = (uint16_t)(attribute | ' ');

		cursor_y = 24;
	}
}

/* Displays a character */
void ConsolePutc(unsigned char c)
{
    uint16_t attribute = (uint16_t)(_color << 8);

    /* backspace character */
    if (c == 0x08 && cursor_x)
        cursor_x--;

    /* tab character */
	else if (c == 0x09)
		cursor_x = (uint8_t)((cursor_x+8) & ~(8-1));

    /* carriage return */
    else if (c == '\r')
        cursor_x = 0;

    /* new line */
	else if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
	}

    /* printable characters */
    else if(c >= ' ')
	{
		/* display character on screen */
        uint16_t* location = video_memory + (uint16_t)(cursor_y*80 + cursor_x);
        *location = (uint16_t)(c | attribute);
        cursor_x++;
    }

    /* if we are at edge of row, go to new line */
    if (cursor_x >= 80)
	{
        cursor_x = 0;
        cursor_y++;
    }

	/* if we are at the last line, scroll up */
	if (cursor_y >= 25)
		scroll();

    /* update hardware cursor */
	UpdateCur(cursor_x, cursor_y);
}

/* Sets new font color */
uint8_t ConsoleSetColor(const uint8_t color)
{
	uint8_t t = _color;
	_color = color;
	return t;
}

/* Sets new position */
void ConsoleGotoXY(uint8_t x, uint8_t y)
{
	if (cursor_x <= 80)
	    cursor_x = x;

	if (cursor_y <= 25)
	    cursor_y = y;

	/* Update hardware cursor to new position */
	UpdateCur(cursor_x, cursor_y);
}

/* Returns position */
void ConsoleGetXY(unsigned* x, unsigned* y)
{
	if (x==0 || y==0)
		return;

	*x = cursor_x;
	*y = cursor_y;
}

/* Returns horzontal width */
int ConsoleGetHorz()
{
	return 80;
}

/* Returns vertical height */
int DebugGetVert()
{
	return 24;
}

/* Clear screen */
void ConsoleClrScr(const uint8_t color)
{
	/* Clear video memory by writing space characters to it */
	for (int i = 0; i < 80*25; i++)
        video_memory[i] = (uint16_t)(' ' | (color << 8));

    /* Move position back to start */
    ConsoleGotoXY(0, 0);
}

/* Displays a string */
void ConsolePuts(char* str)
{
	if (!str)
		return;

    for (unsigned int i = 0; i < strlen(str); i++)
        ConsolePutc((unsigned char)str[i]);
}
