

;*************************************************
;	Gdt.inc
;		-GDT Routines
;
;	OS Development Series
;*************************************************

%ifndef __GDT_INC_67343546FDCC56AAB872_INCLUDED__
%define __GDT_INC_67343546FDCC56AAB872_INCLUDED__

bits	16

;*******************************************
; InstallGDT()
;	- Install our GDT
;*******************************************

InstallGDT:

	cli                  ; clear interrupts
	pusha                ; save registers
	lgdt 	[toc]        ; load GDT into GDTR
	sti	                 ; enable interrupts
	popa                 ; restore registers
	ret	                 ; All done!

;*******************************************
; Global Descriptor Table (GDT)
;*******************************************
 
gdt_data: 
 
; Null descriptor (Offset: 0x0)--Remember each descriptor is 8 bytes!
	dd 0 				; null descriptor
	dd 0 
 
; Kernel Space code (Offset: 0x8 bytes)
	dw 0FFFFh 			; limit low
	dw 0 				; base low
	db 0 				; base middle
	db 10011010b 			; access - Notice that bits 5 and 6 (privilege level) are 0 for Ring 0
	db 11001111b 			; granularity
	db 0 				; base high
 
; Kernel Space data (Offset: 16 (0x10) bytes
	dw 0FFFFh 			; limit low (Same as code)
	dw 0 				; base low
	db 0 				; base middle
	db 10010010b 			; access - Notice that bits 5 and 6 (privilege level) are 0 for Ring 0
	db 11001111b 			; granularity
	db 0				; base high
 
; User Space code (Offset: 24 (0x18) bytes)
	dw 0FFFFh 			; limit low
	dw 0 				; base low
	db 0 				; base middle
	db 11111010b 			; access - Notice that bits 5 and 6 (privilege level) are 11b for Ring 3
	db 11001111b 			; granularity
	db 0 				; base high
 
; User Space data (Offset: 32 (0x20) bytes
	dw 0FFFFh 			; limit low (Same as code)
	dw 0 				; base low
	db 0 				; base middle
	db 11110010b 			; access - Notice that bits 5 and 6 (privilege level) are 11b for Ring 3
	db 11001111b 			; granularity

	
end_of_gdt:
toc: 
	dw end_of_gdt - gdt_data - 1 	; limit (Size of GDT)
	dd gdt_data 			; base of GDT

; give the descriptor offsets names

%define NULL_DESC 0
%define CODE_DESC 0x8
%define DATA_DESC 0x10

%endif ;__GDT_INC_67343546FDCC56AAB872_INCLUDED__
