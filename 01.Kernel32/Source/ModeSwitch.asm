[BITS 32]

global kReadCPUID, kSwitchAndExecute64bitKernel

SECTION .text


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kReadCPUID(DWORD dwEAX, DWORD *pdwEAX, EBX, ECX, EDX) - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kReadCPUID:
push ebp
mov ebp, esp

push eax
push ebx
push ecx
push edx
push esi

mov eax, dword [ebp + 8]
cpuid

mov esi, dword [ebp + 0xc]
mov dword [esi], eax

mov esi, dword [ebp + 0x10]
mov dword [esi], ebx

mov esi, dword [ebp + 0x14]
mov dword [esi], ecx

mov esi, dword [ebp + 0x18]
mov dword [esi], edx

pop esi
pop edx
pop ecx
pop ebx
pop eax

pop ebp
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; kSwitchAndExecute64bitKernel() - cdecl
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
kSwitchAndExecute64bitKernel:

; Set the PAE bit in CR4
mov eax, cr4
or eax, 0x20
mov cr4, eax


; Set CR3 to the address of the PML4 (= 1MB)
mov eax, 0x100000
mov cr3, eax


; Set IA32_EFER.LME
mov ecx, 0xc0000080 ; Address of the IA32_EFER MSR register
rdmsr
or eax, 0x100
wrmsr


; NW = 0, CD = 0, PG = 1  in CR0
mov eax, cr0
or eax, 0xe0000000
xor eax, 0x60000000
mov cr0, eax


; Change the CS segment selector and go to the 2MB area
jmp 0x08:0x200000

jmp $

