#pragma once

#define interrupt _declspec(naked)

/* Start an interrupt handler */
#define INTERRUPT_START() _asm { pushad }
/* Return from an interrupt handler */
#define INTERRUPT_RETURN() _asm { popad }; _asm { iretd }
#define INTERRUPT_RETURN_RETVAL() _asm { mov [esp+28], eax }; _asm { popad }; _asm { iretd }

#define INTERRUPT_END(x) _asm {}
