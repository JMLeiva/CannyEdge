default rel
global applyGrayscale_asm

extern malloc
extern free

; DEFINES
	%define IMAGE_OS_DATA			0
	%define IMAGE_OS_WIDTH			8
	%define IMAGE_OS_HEIGHT			12
	%define IMAGE_OS_BPP			16

section .data
	align 16
	shuffle_packed_bbp3_byte_0:			DB 0x00,0x80,0x80,0x80,		0x01,0x80,0x80,0x80,	0x02,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_packed_bbp3_byte_1:			DB 0x03,0x80,0x80,0x80,		0x04,0x80,0x80,0x80,	0x05,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_packed_bbp3_byte_2:			DB 0x06,0x80,0x80,0x80,		0x07,0x80,0x80,0x80,	0x08,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_packed_bbp3_byte_3:			DB 0x09,0x80,0x80,0x80,		0x0A,0x80,0x80,0x80,	0x0B,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_packed_bbp3_byte_4:			DB 0x0C,0x80,0x80,0x80,		0x0D,0x80,0x80,0x80,	0x0E,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_packed_bbp4_byte_0:			DB 0x00,0x80,0x80,0x80,		0x01,0x80,0x80,0x80,	0x02,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_packed_bbp4_byte_1:			DB 0x04,0x80,0x80,0x80,		0x05,0x80,0x80,0x80,	0x06,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_packed_bbp4_byte_2:			DB 0x08,0x80,0x80,0x80,		0x09,0x80,0x80,0x80,	0x0A,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_packed_bbp4_byte_3:			DB 0x0C,0x80,0x80,0x80,		0x0D,0x80,0x80,0x80,	0x0E,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shufle_repack_1:					DB 0x00,0x04,0x08,0x0C,		0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shufle_repack_2:					DB 0x00,0x80,0x80,0x80,		0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80

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

	; FILL xmm4 with BPP [BPP, BPP, ....] 4 times
	mov r12d, 3 ;movzx r12d byte [rdi+IMAGE_OS_BPP]


	pxor xmm0, xmm0
	pxor xmm1, xmm1

	movd xmm0, r12d
	movsd xmm1, xmm0

	pslldq xmm0, 4
	orps xmm0, xmm1
	pslldq xmm0, 4
	orps xmm0, xmm1
	pslldq xmm0, 4
	orps xmm0, xmm1

	cvtdq2ps xmm0, xmm0	; CONVERT DWords to Float

	mov r8d, 0 											;r8d = unsigned grayIndex = 0;
	mov r9d, 0 											;rd9 = unsigned short val = 0;
	mov r11, 0											; r11 = count = 0

	mov r13d, [rdi + IMAGE_OS_WIDTH]					; r13d = width
	mov r14d, [rdi + IMAGE_OS_HEIGHT]					; r14d = height
	mov r15b, [rdi + IMAGE_OS_BPP]						; r15b = bpp
	mov rdi, [rdi]										; rdi = src data
	mov r12, rax										; r12 = rax

	cmp r15b, 3
	je  loop_bpp3
	cmp r15b, 4
	je loop_bpp4

endLoop:

	mov [rsi + IMAGE_OS_DATA], rax		;dst->data = newData;
	mov [rsi + IMAGE_OS_WIDTH], r13d	;dst->width = src->width;
	mov [rsi + IMAGE_OS_HEIGHT], r14d	;dst->height = src->height;
	mov [rsi + IMAGE_OS_BPP], byte 1	;dst->bpp = 1;

	; RESTORE STACK
	pop	r15
	pop r14
	pop	r13
	pop	r12
	pop rbx
	add rsp, 24
	pop rbp

	ret			; return



; BPP = 3
loop_bpp3:													;for (unsigned int i = 0; i < size; i += src->bpp * 4)
	add r11, 16											; Check forehand if next 4 pixels are within limits
	cmp	r11d, ecx
	jge	endLoop
	sub r11, 16

	mov r10, rdi
	add r10, r11										; r10d = index = image->data + count

	movdqu xmm1, [r10]									; FIX TO BE ALIGNED
	movdqu xmm2, xmm1
	movdqu xmm3, xmm1
	movdqu xmm4, xmm1
	movdqu xmm5, xmm1

	pshufb xmm1, [shuffle_packed_bbp3_byte_0]
	pshufb xmm2, [shuffle_packed_bbp3_byte_1]
	pshufb xmm3, [shuffle_packed_bbp3_byte_2]
	pshufb xmm4, [shuffle_packed_bbp3_byte_3]
	pshufb xmm5, [shuffle_packed_bbp3_byte_4]

	; CVT to float
	cvtdq2ps xmm1, xmm1
	cvtdq2ps xmm2, xmm2
	cvtdq2ps xmm3, xmm3
	cvtdq2ps xmm4, xmm4
	cvtdq2ps xmm5, xmm5

	; Divide by bpp
	divps xmm1, xmm0
	divps xmm2, xmm0
	divps xmm3, xmm0
	divps xmm4, xmm0
	divps xmm5, xmm0


	; RECONVERT TO INT
	cvtps2dq xmm1, xmm1
	cvtps2dq xmm2, xmm2
	cvtps2dq xmm3, xmm3
	cvtps2dq xmm4, xmm4
	cvtps2dq xmm5, xmm5


	phaddd xmm1, xmm2
	phaddd xmm3, xmm4
	phaddd xmm5, xmm5 ;high half unused

	phaddd xmm1, xmm3
	phaddd xmm5, xmm5

	pshufb xmm1, [shufle_repack_1] ;REPACK 4 pixels
	pshufb xmm5, [shufle_repack_2] ;REPACK 1 Pixel

	movd r15d, xmm1
	mov [r12], r15d
	add r12, 4
	movd r15d, xmm5
	mov[r12], r15b
	add r12, 1

	add r11d, 15


	jmp loop_bpp3



; BPP = 4
loop_bpp4:												;for (unsigned int i = 0; i < size; i += src->bpp * 4)
	add r11, 16											; Check forehand if next 4 pixels are within limits
	cmp	r11d, ecx
	jge	endLoop
	sub r11, 16

	mov r10, rdi
	add r10, r11										; r10d = index = image->data + count

	movdqa xmm1, [r10]									; FIX TO BE ALIGNED
	movdqu xmm2, xmm1
	movdqu xmm3, xmm1
	movdqu xmm4, xmm1

	pshufb xmm1, [shuffle_packed_bbp4_byte_0]
	pshufb xmm2, [shuffle_packed_bbp4_byte_1]
	pshufb xmm3, [shuffle_packed_bbp4_byte_2]
	pshufb xmm4, [shuffle_packed_bbp4_byte_3]

	; CVT to float
	cvtdq2ps xmm1, xmm1
	cvtdq2ps xmm2, xmm2
	cvtdq2ps xmm3, xmm3
	cvtdq2ps xmm4, xmm4

	; Divide by bpp
	divps xmm1, xmm0
	divps xmm2, xmm0
	divps xmm3, xmm0
	divps xmm4, xmm0

	; RECONVERT TO INT
	cvtps2dq xmm1, xmm1
	cvtps2dq xmm2, xmm2
	cvtps2dq xmm3, xmm3
	cvtps2dq xmm4, xmm4

	phaddd xmm1, xmm2
	phaddd xmm3, xmm4

	phaddd xmm1, xmm3
	phaddd xmm5, xmm5

	pshufb xmm1, [shufle_repack_1] ;REPACK 4 pixels


	movd r15d, xmm1
	mov [r12], r15d
	add r12, 4

	add r11d, 16
	jmp loop_bpp4






