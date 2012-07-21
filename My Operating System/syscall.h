#pragma once

extern void InitializeSyscallHandler();
extern void SetSyscallFunctionHandler(void*(*function)(void*), int functionNumber);
