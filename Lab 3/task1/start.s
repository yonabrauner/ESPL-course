global main
extern util.c
extern malloc
extern strlen
BUFSZ EQU 1000


section .data
    data_ptr: dd 0
    newline db 10  ; Newline character
    Infile dd 0           ; stdin file descriptor
    Outfile dd 1           ; stdout file descriptor

section .bss
    char: resb 1    

section .text
    global _start1
    global system_call
_start1:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc
    call    main        ; int main( int argc, char *argv[], char *envp[] )
    call    encode
    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state
    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller

main:
    push ebp
    mov ebp, esp
    push ebx
    mov ecx, [ebp+8] ; Get first argument ac
    mov edx, [ebp+12] ; Get 2nd argument av
    mov ebx, 1
    
Next:
    mov eax, [edx+ebx*4]
    pushad
    push eax
    call strlen
    mov edx, eax ; Count of bytes
    pop ecx
    mov eax, 4  
    mov ebx, 1
    int 0x80
    popad
    pushad
    mov eax, 4  
    mov ebx, 1  
    mov ecx, newline  ; Address of newline string
    mov edx, 1        ; Length of newline string
    int 0x80
    popad
    inc ebx
    cmp ebx, ecx
    jnz Next
    mov ecx, BUFSZ  
    mov edx, 1000  
    int 0x80
    add esp, 4
    mov [data_ptr], eax
    pop ebx
    

encode:
    push ebp
    mov ebp, esp
    pushad

read_loop:
    mov eax, 3
    mov ebx, [Infile]
    mov ecx, char
    mov edx, 1
    int 0x80
    
    cmp eax, 0
    je end
    
    
    cmp byte [ecx], 'z'
    jg skip_encode
    cmp byte [ecx], 'A'
    jl skip_encode
    add byte [ecx], 1 

skip_encode:
    mov eax, 4
    mov ebx, [Outfile]
    mov ecx, char
    mov edx, 1
    int 0x80

    jmp read_loop ; Jump back to read the next character

end:
    popad
    mov esp, ebp
    pop ebp
    ret

