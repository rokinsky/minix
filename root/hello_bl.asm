mov ah, 0xe  ; argument - doprecyzowanie funkcji przerwania (wypisz znaki przesuń kursor)
mov al, 'H'  ; argument - znak do wypisania
int 0x10     ; wywołanie przerwania nr 16 - obsługa ekranu    

mov al, 'E'
int 0x10

mov al, 'L'
int 0x10 

mov al, 'L'  
int 0x10        

mov al, 'O' 
int 0x10         

mov al, 0xd
int 0x10

mov al, 0xa
int 0x10

loop:
  jmp loop

times 510 - ($ - $$) db 0
dw 0xaa55
