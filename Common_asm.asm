section .text           ; Code section.
global convolute_asm

; DEFINES
	%define IMAGE_WIDTH_OFFSET 								0

convolute_asm: ; (Image* src, SquareMatrix mat)
	; PREPARE STACK
	push rbp
	mov rbp, rsp
	sub rsp, 24
	push rbx
	push r12
	push r13
	push r14
	push r15

	;int dataSize = src->width * src->height * src->bpp;
	;mov r8,


	; RESTORE STACK
	pop	r15
	pop r14
	pop	r13
	pop	r12
	pop rbx
	add rsp, 24
	pop rbp

	mov	rax,0	; normal, no error, return value
	ret			; return
