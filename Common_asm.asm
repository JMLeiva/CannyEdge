global convolute_asm_1bpp
extern malloc

; DEFINES
	%define IMAGE_OS_DATA			16
	%define IMAGE_OS_WIDTH			0
	%define IMAGE_OS_HEIGHT			4
	%define IMAGE_OS_BPP			8

	%define MAT_OS_SIZE				0
	%define MAT_OS_DATA				16
	%define VAR_OFFSET_X			0
	%define VAR_OFFSET_Y			4


section .rodata
	align 16
	shuffle_packed_4_byte_to_float:			DB 0x00,0x80,0x80,0x80,		0x01,0x80,0x80,0x80,	0x02,0x80,0x80,0x80,	0x03,0x80,0x80,0x80

	align 16
	shuffle_get_2nd_float:					DB 0x04,0x05,0x06,0x07,		0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_get_3rd_float:					DB 0x08,0x09,0x0A,0x0B,		0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80
	align 16
	shuffle_get_4th_float:					DB 0x0C,0x0D,0x0E,0x0F,		0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80,	0x80,0x80,0x80,0x80


	align 16
	and_keep_1_float:						DD 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000
	align 16
	and_keep_2_float:						DD 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000
	align 16
	and_keep_3_float:						DD 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000


section .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; MAIN FUNCTION
convolute_asm_1bpp: ; (Image* src, SquareMatrix* mat) (mat is 16bytes aligned horizontally)
	; PREPARE STACK
	push rbp
	mov rbp, rsp
	push rbx
	push r12
	push r13
	push r14
	push r15

	mov ecx, [rdi+IMAGE_OS_WIDTH]					;ecx = src->width * src->height * src->bpp
	imul ecx, [rdi+IMAGE_OS_HEIGHT]					;eax = src->width * src->height (GRAYSCALE SIZE)
	xor  eax, eax
	mov  al, [rdi+IMAGE_OS_BPP]
	imul ecx, eax					;ecx = dataSize = src->width * src->height * src->bpp * src->bpp

	; MALLOC CALL
	sub  rsp, 8
	push rdi
	push rsi
	push rcx
	push rdx

	mov edi, ecx
	sal edi, 1		;edi = dataSize * 2

	call malloc		;short* newData = (short*)malloc(dataSize * 2);

	pop rdx
	pop rcx
	pop rsi
	pop rdi
	add rsp, 8

	mov r14d, [rdi+IMAGE_OS_WIDTH]
	mov r15d, [rdi+IMAGE_OS_HEIGHT]

	xor r13d, r13d
	mov r13b, [rsi+MAT_OS_SIZE] ; r13 = mat->size
	sar r13b, 1					; r13 = mat->size / 2

	mov r10d, r13d		;y = mat->size / 2

	xor r12, r12	; srcIndex = (y * src->width + x)
	mov r12d, r10d
	imul r12d, r14d
	add r12d, r13d
	sal r12d, 1

loop_y:				;for (y = 0; y < src->height; y++)
	add r10d, r13d
	cmp r10d, r15d
	je	end_loop_y
	sub r10d, r13d

	;xor r11d, r11d
	mov r11d, r13d	;x = mat->size / 2

loop_x:				;for (x = 0; x < src->width; x++)
	add r11d, r13d
	cmp r11d, r14d
	jg	end_loop_x
	sub r11d, r13d

	push rax
	push rdi
	push rsi
	push rcx
	push rdx
	push r10
	push r11

	; rdi = src
	; rsi = mat
	mov edx, r11d ;x
	mov ecx, r10d ;y
	;lea r8, [rax+r12]
	mov r8,	 rax
	add r8,  r12 ;newData + srcIndex

	call performConvolutionStep_1bpp ;performConvolutionStep(src, mat, x, y, newData + srcIndex);

	pop r11
	pop r10
	pop rdx
	pop rcx
	pop rsi
	pop rdi
	pop rax

	add r12, 2
	inc r11d
	jmp loop_x
end_loop_x:
	inc r10d
	sub r12, 2

	add r12d, r13d
	add r12d, r13d
	add r12d, r13d
	add r12d, r13d
	jmp loop_y
end_loop_y:


	; RESTORE STACK
	pop	r15
	pop r14
	pop	r13
	pop	r12
	pop rbx
	pop rbp

	ret			; return


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; PIXEL CONVOLUTION
performConvolutionStep_1bpp: ;const Image* image, const SquareMatrix* mat, const unsigned int x, const unsigned int y, short* dst
	; PREPARE STACK
	push rbp
	mov rbp, rsp
	push rbx
	push r12
	push r13
	push r14
	push r15

	mov r14, r8  ; r14 = dst

	mov r8b, [rsi+MAT_OS_SIZE]
	and r8d, 0x000000FF

	mov r9d, 0

	xor	r15, r15
	mov r15d, r8d		; r15d = matSize
	sar r15d, 2			; r15d = matSize / 4
	inc r15d			; r15d = (matSize / 4) + 1
	sal r15d, 4			; r15d = ((mSize / 4) + 1) * 16 = matSizeAligned * (sizeOf(float));

	;movsxd	r15, r15d	; Sign extend to r15

	; Calculate end of top line (y - matSize/2)
	mov r13d, r8d
	sar r13d, 1

	xor		r10, r10
	mov 	r10d, ecx 	;r10d = y
	sub		r10d, r13d	;r10d = y - matSize/2

	xor		r11,  r11
	mov		r11d, [rdi+IMAGE_OS_WIDTH]
	imul 	r10d, r11d	; r10d = y * src->width

	;movsxd	r11, r11d	; Sign extend to r15

	 mov	ecx, r8d	; restore ecx
	add rsi, MAT_OS_DATA

	; Set up Src and Mat Pointers
	;add		rdi, IMAGE_OS_DATA
	mov  	rdi, [rdi+IMAGE_OS_DATA]
	add		rdi, r10	; rdi = startOfLine

	mov  	rsi, [rsi]

	; result is stored in xmm1
	pxor xmm8, xmm8


convolution_line_loop:
	cmp r9d, r8d
	je 	end_convolution_line_loop

	push r8
	push r9
	push r11
	push rdi
	push rsi
	push rdx
	push rcx

	; rdi = start of src line
	; rsi = start of mat line
	; edx = x
	; ecx = matSize
	call performConvolutionLine_1bpp ;(const char* line, float* matLine, const unsigned int x, int matSize)

	addss xmm8, xmm0

	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop r11
	pop r9
	pop r8

	add rsi, r15	;	matLine + alignedSize * sizeof(float)
	add	rdi, r11	;	srcLine + src->width
	inc r9d
	jmp convolution_line_loop
end_convolution_line_loop:

	; cvt result to integer
	cvtss2si eax, xmm8

	; result is a short, must convert to it. If its positive, do nothing. If its negative, must add sign
	cmp eax, 0
	jge end_cvt_to_short

cvt_to_short_negative:
	mov ecx, eax
	xor eax, eax
	add ax, cx

end_cvt_to_short:

	; Write result (in ax)
	mov [r14], ax

	; RESTORE STACK
	pop	r15
	pop r14
	pop	r13
	pop	r12
	pop rbx
	pop rbp

	ret			; return


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; SINGLE MATRIX LINE
performConvolutionLine_1bpp: ;float (const char* line, float* matLine, const unsigned int x, int matSize)
	push rbp
	mov rbp, rsp
	push rbx
	push r12
	push r13
	push r14
	push r15


	xor 	r8d, 	r8d
	xor  	r13d, 	r13d

	add rdi, rdx
	; Calculate number of sections
 	mov	eax, ecx
	mov	edx, ecx
	sar eax, 2					; 	eax = matSize / 4
	and	edx, 0x03				;	edx = matSize % 4

	inc eax

	mov     r14d, edx		;   r14d = matSize % 4

	xor		r10, r10
	mov     r10d, ecx
	sar     r10d, 1			; r10d = matSize / 2

	; set rdi to correct position

	sub rdi, r10
	; initizalize result
	pxor xmm0, xmm0

loop_sections:
	cmp	r8d, eax
	je	end_loop_sections


	movdqa	xmm1, [rsi]		; load matrix line section
	movdqu  xmm2, [rdi]		; load image line section

continue_line:

	; cvt to float
	pshufb xmm2, [shuffle_packed_4_byte_to_float]
	cvtdq2ps xmm2, xmm2

	; increase loop data
	add	rsi,	16
	add rdi,	4
	inc	r8d

check_last_section:
	cmp r8d, eax
	je  is_last_section

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;jmp continue_overflow_data
	; Multiply and sum
	nop
	mulps  	xmm1, xmm2

	;; OPTMIZATION
	haddps	xmm1, xmm1
	haddps	xmm1, xmm1



	; Add to result
	addss	xmm0, xmm1

	jmp loop_sections

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

is_last_section: ; Zero overflowing data (outside matrix convolution)
	cmp r14d, 1
	je keep_1_float					; if (matSize % 4 == 1)
	pand xmm2, [and_keep_3_float] 	; if matSize % 4 != 1 then its 3

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;jmp continue_overflow_data
	; Multiply and sum
	mulps  	xmm1, xmm2

	;; OPTMIZATION
	haddps	xmm1, xmm1
	haddps	xmm1, xmm1



	; Add to result
	addss	xmm0, xmm1

	jmp loop_sections

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
keep_1_float:
	pand xmm2, [and_keep_1_float]

	; Multiply and sum
	mulps  	xmm1, xmm2

	;; OPTMIZATION
	haddps	xmm1, xmm1
	haddps	xmm1, xmm1



	; Add to result
	addss	xmm0, xmm1

	jmp loop_sections

end_loop_sections:

	pop	r15
	pop r14
	pop	r13
	pop	r12
	pop rbx
	pop rbp

	ret			; return
