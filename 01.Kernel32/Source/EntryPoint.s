
[ORG 0x00]
[BITS 16]

SECTION .text

START:
mov ax, 0x1000
mov ds, ax
mov es, ax

cli

lgdt [GDTR]



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Protected Mode
; Disable Paging, Cache, Internal FPU, and Align Check
; Enable Protected Mode
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov eax, 0x4000003b	; PG=0, CD=1, NW=0, AM=0, WP=0, NE=1, ET=1, TS=1, EM=0, MP=1, PE=1
mov cr0, eax


jmp dword 0x08:(PROTECTEDMODE - $$ + 0x10000)


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; Protected Mode
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]
PROTECTEDMODE:
mov ax, 0x10
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax

mov ss, ax
mov esp, 0xfffe
mov ebp, 0xfffe

push (SWITCHSUCCESSMSG - $$ + 0x10000)
push 2
push 0
call PRINTMESSAGE
add esp, 0xc


jmp dword 0x08:0x10200


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PRINTMESSAGE
; void PRINTMESSAGE(int x, int y, char *msg)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

PRINTMESSAGE:
push ebp
mov ebp, esp
push esi
push edi
push eax

mov eax, dword [ebp + 0xc]
mov esi, 160
mul esi
mov edi, eax

mov eax, dword [ebp + 8]
shl eax, 1
add edi, eax

mov esi, dword [ebp + 0x10]


.MESSAGELOOP:
mov al, byte [esi]
cmp al, 0
je .MESSAGEEND

mov byte [edi + 0xb8000], al

add esi, 1
add edi, 2

jmp .MESSAGELOOP
.MESSAGEEND:

pop eax
pop edi
pop esi
leave
ret



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data Section
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 8, db 0

; GDTR structure
dw 0x0000
GDTR:
dw GDTEND - GDT - 1
dd (GDT - $$ + 0x10000)

GDT:
NULLDescriptor:
dd 0
dd 0

CODEDESCRIPTOR:
dw 0xffff	; Limit [15:0]
dw 0x0000	; Base [15:0]
db 0x00		; Base [23:16]
db 0x9a		; P=1, DPL=0, Code Segment, Execute/Read
db 0xcf		; G=1, D=1, L=0, Limit[19:16]
db 0x00		; Base [31:24]

DATADESCRIPTOR:
dw 0xffff	; Limit [15:0]
dw 0x0000	; Base [15:0]
db 0x00		; Base [23:16]
db 0x92		; P=1, DPL=0, Data Segment, Read/Write
db 0xcf		; G=1, D=1, L=0, Limit[19:16]
db 0x00		; Base [31:24]

GDTEND:

SWITCHSUCCESSMSG: db 'Switch to Protected Mode', 0

times 512 - ($ - $$) db 0x00
