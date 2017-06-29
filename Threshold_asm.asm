default rel
global applyThresholdLowHigh_asm_impl_1bpp

extern malloc
extern free

; DEFINES
	%define IMAGE_OS_DATA			16
	%define IMAGE_OS_WIDTH			0
	%define IMAGE_OS_HEIGHT			4
	%define IMAGE_OS_BPP			8

section .data
	align 16
	all_ones_128:		DQ	0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF
section .text

applyThresholdLowHigh_asm_impl_1bpp: ;(const Image* src, const unsigned char thresholdlow, const unsigned char thresholdHigh, const unsigned char vlow, const unsigned char vHigh, Image* dst)
	; PREPARE STACK
	push rbp
	mov rbp, rsp
	sub rsp, 8 ; <- dst
	push rbx
	push r12
	push r13
	push r14
	push r15

	; rdi = src
	; sil = thresholdlow
	; dl  = thresholdhigh
	; cl = vlow
	; r8b = vhigh
	; r9 = dst

	; Fill dst attrs
	mov eax, [rdi + IMAGE_OS_WIDTH]
	mov [r9 + IMAGE_OS_WIDTH], eax		;dst->width = src->width;
	mov eax, [rdi + IMAGE_OS_HEIGHT]
	mov [r9 + IMAGE_OS_HEIGHT], eax		;dst->height = src->height;
	mov [r9 + IMAGE_OS_BPP], byte 1		;dst->bpp = 1;


	mov r11d, [rdi+IMAGE_OS_WIDTH]						;eax = src->width * src->height * src->bpp
	imul r11d, [rdi+IMAGE_OS_HEIGHT]

	; MALLOC CALL
	push rdi
	push rsi
	push rcx
	push rdx
	push r8
	push r9
	sub rsp, 8

	mov edi, r11d

	call malloc
	mov [r9 + IMAGE_OS_DATA], rax		;dst->data = newData;

	add rsp, 8
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rsi
	pop rdi

	;; Prepare xmm comparers
	and  esi, 	0x000000FF
	and  edx,  	0x000000FF
	and  ecx,  	0x000000FF
	and  r8d, 	0x000000FF

	pxor xmm0, 	xmm0
	movd xmm0, 	esi		; < thres low

	pxor xmm1,  xmm1	; < thres high
	movd xmm1, 	edx

	pxor xmm2, xmm2		; < val low
	movd xmm2, ecx

	pxor xmm3, xmm3		; < val high
	movd xmm3, r8d

	pxor xmm4, xmm4		; <- all zeros, fill with first 8bits
	pshufb xmm0, xmm4
	pshufb xmm1, xmm4
	pshufb xmm2, xmm4
	pshufb xmm3, xmm4

	mov r10d, 0

	movdqa xmm4, [all_ones_128]

	mov rdi, [rdi + IMAGE_OS_DATA]
loop:
	add r10d, 16
	cmp r10d, r11d
	jge endLoop
	sub r10d, 16


	movdqa 	xmm5, [rdi]

	movdqa	xmm6, xmm5

	pcmpgtb	xmm5, xmm0 ; thres low flags
	pcmpgtb xmm6, xmm1 ; thress high flags
	movdqa  xmm7, xmm6
	pxor	xmm7, xmm4 ; thres high inverse --- logical not xmm3 (xmm4 is all ones)
	pand	xmm5, xmm7 ; abs thres low flag (without thres high flags)

	pand	xmm5, xmm3
	pand	xmm6, xmm4

	por	xmm5, xmm6

	; Write result
	movdqa  [rax], xmm5

	add rax, 16
	add rdi, 16
	add r10d, 16
	jmp loop
endLoop:


	; RESTORE STACK
	pop	r15
	pop r14
	pop	r13
	pop	r12
	pop rbx
	add rsp, 8
	pop rbp

	ret			; return
