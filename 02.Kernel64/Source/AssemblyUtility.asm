[BITS 64]

SECTION .text

global kInPortByte, kOutPortByte

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
