#pragma once

#include "interrupt.h"

extern void divide_by_zero_fault();
extern void single_step_trap();
extern void nmi_trap();
extern void breakpoint_trap();
extern void overflow_trap();
extern void bounds_check_fault();
extern void invalid_opcode_fault();
extern void no_device_fault();
extern void double_fault_abort();
extern void invalid_tss_fault();
extern void no_segment_fault();
extern void stack_fault();
extern void general_protection_fault();
extern void page_fault();
extern void fpu_fault();
extern void alignment_check_fault();
extern void machine_check_abort();
extern void simd_fpu_fault();
