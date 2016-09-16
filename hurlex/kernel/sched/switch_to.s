[global switch_to]

; 具体的线程切换操作，重点在于寄存器的保存与恢复
; 我们是这么来调用context的,传入的第一个参数是prev->context，第二个参数是current->context
switch_to:
        mov eax, [esp+4]

        mov [eax+0],  esp ; prev->context.esp
        mov [eax+4],  ebp ; prev->context.ebp
        mov [eax+8],  ebx ; prev->context.ebx
        mov [eax+12], esi ; prev->context.esi
        mov [eax+16], edi ; prev->context.edi
        pushf			  ; 将flags压栈
        pop ecx			  ; 放入ecx中
        mov [eax+20], ecx ; prev->context.eflags

        mov eax, [esp+8]

        mov esp, [eax+0]
        mov ebp, [eax+4]
        mov ebx, [eax+8]
        mov esi, [eax+12]
        mov edi, [eax+16]
        mov eax, [eax+20]
        push eax
        popf
 	
        ret					; 好吧，直接就返回了！
