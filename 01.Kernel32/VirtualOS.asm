[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x1000:START

SECTORCOUNT:		dw 0x0000
TOTALSECTORCOUNT	equ 1024

START:
mov ax, cs
mov ds, ax
mov ax, 0xb800
mov es, ax

%macro busyloop 0
	push ax
	push bx
	xor ax, ax
	xor bx, bx
	%%label1:
	inc ax
	jne %%label1
	inc bx
	test bx, 0x100
	je %%label1
	pop bx
	pop ax
%endmacro


%assign i 0
%rep TOTALSECTORCOUNT
	%assign i i+1

	;mov ax, 2
	;mul word [ds: SECTORCOUNT]
	;mov si, ax
	mov si, word [ds: SECTORCOUNT]
	shl si, 1

	;mov si, ((i - 1) * 2)
	mov byte [es: si + (160 * 2)], '0' + (i % 10)
	
	add word [ds: SECTORCOUNT], 1

	;busyloop
	
	%if i == TOTALSECTORCOUNT
		;jmp $
		mov byte [es: 0x0], 'X'
		mov byte [es: 0x1], 0
		db 0xeb
		db 0xfe
	%else
;		mov ax, cs
;		add ax, 0x20
;		push ax
;		push 0
;		retf
		jmp (0x1000 + (i * 0x20)):0x0000
	%endif
	
	times (512 - (($-$$) % 512)) db 0x00
%endrep
