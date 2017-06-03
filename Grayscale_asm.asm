default rel
global applyGrayscale_asm

extern malloc
extern free

; DEFINES
	%define IMAGE_OS_DATA			0
	%define IMAGE_OS_WIDTH			8
	%define IMAGE_OS_HEIGHT			12
	%define IMAGE_OS_BPP			16

section .text

applyGrayscale_asm: ;(const Image* src, Image* dst)
	; PREPARE STACK
	push rbp
	mov rbp, rsp
	sub rsp, 24
	push rbx
	push r12
	push r13
	push r14
	push r15


	mov ecx, [rdi+IMAGE_OS_WIDTH]						;ecx = src->width * src->height * src->bpp
	imul ecx, [rdi+IMAGE_OS_HEIGHT]
	mov eax, ecx										;eax = src->width * src->height (GRAYSCALE SIZE)
	imul ecx, [rdi+IMAGE_OS_BPP]						;ecx = src->width * src->height * src->bpp * src->bpp

	; MALLOC CALL
	push rdi
	push rsi
	push rcx

	mov edi, eax
	call malloc											;rax = unsigned char* newData = (unsigned char*)malloc(src->width * src->height);

	pop rcx
	pop rsi
	pop rdi



	mov r8d, 0 											;r8d = unsigned grayIndex = 0;
	mov r9d, 0 											;rd9 = unsigned short val = 0;

loop:													;for (unsigned int i = 0; i < size; i += src->bpp)


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







	;for (unsigned int i = 0; i < src->width * src->height * src->bpp; i += src->bpp)
	;{
	;	val = 0;

	;	for (unsigned char b = 0; b < src->bpp || b < 3; b++)
	;	{
	;		val += src->data[i + b];
	;	}

	;	newData[grayIndex] = val / src->bpp;
	;	grayIndex++;
	;}


	;dst->data = newData;
	;dst->width = src->width;
	;dst->height = src->height;
	;dst->bpp = 1;
