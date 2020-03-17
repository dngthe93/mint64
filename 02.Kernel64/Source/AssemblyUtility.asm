[BITS 64]

SECTION .text

global kInPortByte, kOutPortByte, kLoadGDTR, kLoadTR, kLoadIDTR
global kEnableInterrupt, kDisableInterrupt, kReadRFLAGS
global kReadTSC
global kSwitchContext, kHlt



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kInPortByte(WORD wPort) - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kInPortByte:
push rdx

;rdi: first parameter = wPort
mov rdx, rdi
xor rax, rax
in al, dx

pop rdx
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kOutPortByte(WORD wPort, BYTE bData) - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kOutPortByte:
push rdx
push rax

mov rdx, rdi
mov rax, rsi
out dx, al

pop rax
pop rdx
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kLoadGDTR(QWORD qwGDTRAddress) - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kLoadGDTR:
lgdt [rdi]
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kLoadTR(WORD wTSSSegmentOffset) - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kLoadTR:
ltr di
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kLoadIDTR(QWORD qwIDTRAddress) - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kLoadIDTR:
lidt [rdi]
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kEnableInterrupt() - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kEnableInterrupt:
sti
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kDisableInterrupt() - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kDisableInterrupt:
cli
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kReadRFLAGS() - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kReadRFLAGS:
pushfq
pop rax
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kReadTSC() - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kReadTSC:
push rdx

rdtsc

shl rdx, 32
or rax, rdx

pop rdx
ret


; Save the current context
; 0: No parameters
%macro KSAVECONTEXT 0
	; Save all of the gernal purpose registers (except for already saved regs)
	push rbp
	push rax
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	;  Segment selectors
	mov ax, ds
	push rax
	mov ax, es
	push rax
	push fs
	push gs
%endmacro


; Load the saved context
; 0: No parameters
%macro KLOADCONTEXT 0
	; Restore the segment selectors
	pop gs
	pop fs
	pop rax
	mov es, ax
	pop rax
	mov ds, ax

	; Restore the GPRs
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rbx
	pop rax
	pop rbp
%endmacro


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kSwitchContext(CONTEXT *pstCurrentContext, CONTEXT *pstNextContext)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kSwitchContext:
push rbp
mov rbp, rsp

; Backup the RFLAGS. 'cmp' op may changes the RFLAGS bits
pushfq
; if (pstCurrentContext == NULL)
;    goto .LoadContext; // Load the next context w/o saving the current context
cmp rdi, 0
je .LoadContext
popfq

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Save the current context
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
push rax

; SS segment
mov ax, ss
mov qword [rdi + (23 * 8)], rax

; RSP register: skip RBP & Ret addr
mov rax, rbp
add rax, 16
mov qword [rdi + (22 * 8)], rax

; RFLAGS register
pushfq
pop rax
mov qword [rdi + (21 * 8)], rax

; CS segment
mov ax, cs
mov qword [rdi + (20 * 8)], rax

; RIP register: Ret address
mov rax, qword [rbp + 8]
mov qword [rdi + (19 * 8)], rax

pop rax
pop rbp

; Set RSP register to RDI + (19 * 8),
;  to save the rest register using 'push' op at the KSAVECONTEXT macro
mov rsp, rdi
add rsp, (19 * 8)

; Save the rest of register
KSAVECONTEXT


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Restore the current context
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.LoadContext:

mov rsp, rsi
KLOADCONTEXT
iretq


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kHlt()
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kHlt:
hlt
hlt
ret
