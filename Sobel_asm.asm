default rel
global apply_sobel_gradient_calculation_1bpp

extern atan2
extern normalizeAngle

; DEFINES
	%define IMAGE_OS_DATA			16
	%define IMAGE_OS_WIDTH			0
	%define IMAGE_OS_HEIGHT			4
	%define IMAGE_OS_BPP			8

section .data
	align 16
	shuffle_packed_word_255:			DW 0x00FF,0x00FF,0x00FF,0x00FF,		0x00FF,0x00FF,0x00FF,0x00FF

section .text

apply_sobel_gradient_calculation_1bpp: ;(short* xResult, short * yResult, Image* dstLum, Image* dstAngle);
	; PREPARE STACK
	push rbp
	mov rbp, rsp
	push rbx
	push r12
	push r13
	push r14
	push r15

	mov r8d, [rdx+IMAGE_OS_WIDTH]						;eax = src->width * src->height * 2 (words)
	imul r8d, [rdx+IMAGE_OS_HEIGHT]
	sal  r8d, 1



	movdqa xmm15, [shuffle_packed_word_255]				; xmm2 = all 255 words

	mov	r12, rdi
	mov r13, rsi

	mov r14, [rcx+IMAGE_OS_DATA]	; r14 -> angleData
	mov r15, [rdx+IMAGE_OS_DATA]	; r15 -> lumData

	xor r9d, r9d

loop:
	add r9d, 16
	cmp	r9d, r8d
	jge endLoop
	sub r9d, 16

	movdqa xmm2, [r12]		; xResult
	movdqa xmm3, [r13]		; yResult

	pabsw  xmm4, xmm2		; convert to absolute value
	pabsw  xmm5, xmm3		; convert to absolute value

	paddusw	xmm4, xmm5

	pminuw	xmm3, xmm15		; saturate to 255

	; TODO second 8 words
	packuswb xmm4, xmm4
	movdqu [r15], xmm4

	; Angle
	push r8
	push r9

%rep 8
	mov ax, [r12]
	cwd				; SIGN EXTEND
	xor edi, edi
	or di, dx
	sal edi, 16
	or di, ax

	mov ax, [r13]
	cwd				; SIGN EXTEND
	xor esi, esi
	or si, dx
	sal esi, 16
	or si, ax

	cvtsi2sd xmm0, edi	;cvt to double
	cvtsi2sd xmm1, esi  ;cvt to double
	call atan2
	call normalizeAngle
	mov [r14], al
	add r14, 1
	add r12, 2
	add r13, 2
%endrep
;;;;;;;;;;;


	pop r9
	pop r8

	add r15, 8
	add	r9d, 16
	;add	r12, 16
	;add 	r13, 16
	jmp	loop
endLoop:

	; RESTORE STACK
	pop	r15
	pop r14
	pop	r13
	pop	r12
	pop rbx
	pop rbp

	ret			; return



;for (unsigned int i = 0; i < src->width * src->height * src->bpp; i++)
;	{
;		for (int c = 0; c < src->bpp; c++)
;		{
;			float v = abs(xResult[i]) + abs(yResult[i]);//sqrt(xResult.data[i] * xResult.data[i] + yResult.data[i] * yResult.data[i]);
;			if(v > 255)
;			{
;				dstLum->data[i] = 255;
;			}
;			else
;			{
;				dstLum->data[i] = (unsigned char)v;
;			}
;
;			short angle = 0;
;
;			angle = normalizeAngle(atan2f(xResult[i], yResult[i]));
;
;			dstAngle->data[i] = angle;
;		}
;	}
