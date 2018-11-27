
[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07c0:START

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
TOTALSECTORCOUNT:	dw 1024

SECTORNUMBER:		db 0x02
HEADNUMBER:			db 0x00
TRACKNUMBER:		db 0x00

SECTOR_PER_TRACK:	db 0x00
LAST_HEAD:			db 0x00
LAST_TRACK:			dw 0x00

MESSAGE1:			db 'MINT64 OS Boot Loader Start', 0
DISKERRORMSG: 		db '  DISK Error', 0
RESETERRORMSG:		db '  RESET Error', 0
IMAGELOADINGMSG: 	db 'OS Image Loading...', 0
LOADINGCOMPLETEMSG: db 'Loading Complete', 0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


START:
mov ax, 0x07c0
mov ds, ax		; set DS = CS = 0x7c0

mov ax, 0xb800
mov es, ax		; set ES = 0xb800

mov ax, 0x0000
mov ss, ax		; set SS = 0x0000
mov sp, 0xFFFE
mov bp, sp		; bp = sp = 0xFFFE


mov di, 0
.SCREENCLEARLOOP:
mov word [es: di], 0x4a00
inc di
inc di

cmp di, 80 * 25 * 2
jl .SCREENCLEARLOOP


push MESSAGE1
push 0
push 0
call PRINTMESSAGE
add sp, 6			; Print MESSAGE1

push IMAGELOADINGMSG
push 1
push 0
call PRINTMESSAGE
add sp, 6			; Print IMAGELOADINGMSG



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Reset Floppy Disk
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RESETDISK:
mov ah, 0	; BIOS Service number 0 (Reset)
mov dl, 0	; 0x00: Floppy disk
int 0x13
jc HANDLERESETERROR



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Reset Disk Parameters
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
push es

mov ah, 0x08	; BIOS Service number 8 (Read Drive Parameters)
mov dl, 0x00	; 0x00: Floppy disk

mov di, 0x1000
mov es, di
xor di, di		; ES:DI. Pointer to drive parameter table (only for floppies)

int 0x13
jc HANDLERESETERROR

; Save the value of cx register
mov ax, cx

; Sector per Track
shl cl, 2
shr cl, 2
mov byte [ds: SECTOR_PER_TRACK], cl

; Last index of Cylinders
mov cx, ax ; Restore cx register
and cx, 0xc0 ; 0000 0000 1100 0000
shl cx, 2

shr ax, 8
or cx, ax
mov word [ds: LAST_TRACK], cx

; Last index of Heads
mov byte [ds: LAST_HEAD], dh


cmp byte [ds: SECTOR_PER_TRACK], 36
jne HANDLERESETERROR
cmp byte [ds: LAST_HEAD], 1
jne HANDLERESETERROR
cmp word [ds: LAST_TRACK], 79
jne HANDLERESETERROR

pop es

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Read Sectors from Floppy Disk
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
mov si, 0x1000
mov es, si		; set ES = 0x1000
mov bx, 0000	; es:bx = 0x10000


mov di, word [ds: TOTALSECTORCOUNT]

READDATA:
cmp di, 0
je READEND
sub di, 0x1

xor bx, bx

;;;;;;;;;;;;;;;;;;
mov ah, 0x02					; BIOS service number 2 (Read Sector)
mov al, 0x01					; # of sectors to read
mov ch, byte [ds: TRACKNUMBER]	; Track #
mov cl, byte [ds: SECTORNUMBER] ; Sector #
;shl ch, 6
;and cl, ch
;mov ch, byte [ds: TRACKNUMBER]
;shr ch, 2

mov dh, byte [ds: HEADNUMBER]	; Head #
mov dl, 0x00					; 0x00: Floppy disk
int 0x13						; 0x13: Disk I/O Service (BIOS Service)
jc HANDLEDISKERROR
cmp al, 1
jne HANDLEDISKERROR
cmp ah, 0
jne HANDLEDISKERROR
;;;;;;;;;;;;;;;;;;

mov si, es
add si, 0x0020	; size of sector (512 Bytes)
mov es, si

mov al, byte [ds: SECTORNUMBER]
inc al
mov byte [ds: SECTORNUMBER], al

dec al
cmp byte [ds: SECTOR_PER_TRACK], al
;cmp al, 37
jne READDATA

mov byte [ds: SECTORNUMBER], 0x01
xor byte [ds: HEADNUMBER], 0x01

cmp byte [ds: HEADNUMBER], 0x01
je READDATA

add byte [ds: TRACKNUMBER], 0x01
jmp READDATA

READEND:


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Print Complete Message
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
push LOADINGCOMPLETEMSG
push 1
push 20
call PRINTMESSAGE
add sp, 6

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Execute OS Image
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
jmp 0x1000:0x0000


 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Print Disk ERROR Message
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
HANDLEDISKERROR:
push DISKERRORMSG
push 1
push 20
call PRINTMESSAGE

jmp $		; Infinite loop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Print Reset ERROR Message
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
HANDLERESETERROR:
push RESETERRORMSG
push 1
push 20
call PRINTMESSAGE

jmp $		; Infinite loop

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PRINTMESSAGE(x, y, msg) - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
PRINTMESSAGE:
push bp
mov bp, sp
push es
push si
push di
push ax
push cx
push dx

mov ax, 0xb800
mov es, ax

mov ax, word [ss: bp + 6]
mov si, 80 * 2
mul si
mov di, ax

mov ax, word [ss: bp + 4]
shl ax, 1
add di, ax

mov si, word [ss: bp + 8]

.MESSAGELOOP:
mov cl, byte [ds: si]
cmp cl, 0
je .MESSAGEEND

mov byte [es: di], cl
inc si
add di, 2
jmp .MESSAGELOOP

.MESSAGEEND:
pop dx
pop cx
pop ax
pop di
pop si
pop es
pop bp
ret




times 510 - ($ - $$) db 0x00
db 0x55
db 0xAA
