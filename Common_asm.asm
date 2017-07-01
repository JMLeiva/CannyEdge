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


section .data
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
	sal edi, 1		;r8d = dataSize * 2

	call malloc		;short* newData = (short*)malloc(dataSize * 2);

	pop rdx
	pop rcx
	pop rsi
	pop rdi
	add rsp, 8

	mov r8d, [rdi+IMAGE_OS_WIDTH]
	mov r9d, [rdi+IMAGE_OS_HEIGHT]

	xor r13d, r13d
	mov r13b, [rsi+MAT_OS_SIZE] ; r13 = mat->size
	sar r13b, 1					; r13 = mat->size / 2

	mov r10d, r13d		;y = mat->size / 2

loop_y:				;for (y = 0; y < src->height; y++)
	add r10d, r13d
	cmp r10d, r9d
	je	end_loop_y
	sub r10d, r13d

	xor r11d, r11d
	mov r11d, r13d	;x = mat->size / 2

	xor r12, r12	; srcIndex = (y * src->width + x)
	mov r12d, r10d
	imul r12d, r8d
	add r12d, r11d
	sal r12d, 1		; is a short array, index must be multiplied by 2

loop_x:				;for (x = 0; x < src->width; x++)
	add r11d, r13d
	cmp r11d, r8d
	jg	end_loop_x
	sub r11d, r13d

	push rax
	push rdi
	push rsi
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11

	; rdi = src
	; rsi = mat
	mov edx, r11d ;x
	mov ecx, r10d ;y
	mov r8,	 rax
	add r8,  r12 ;newData + srcIndex

	call performConvolutionStep_1bpp ;performConvolutionStep(src, mat, x, y, newData + srcIndex);

	pop r11
	pop r10
	pop r9
	pop r8
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

	mov r15d, r8d		; r15d = matSize
	sar r15d, 2			; r15d = matSize / 4
	inc r15d			; r15d = (matSize / 4) + 1
	sal r15d, 4			; r15d = ((mSize / 4) + 1) * 16 = matSizeAligned * (sizeOf(float));

	mov eax, r15d
	mov r12d, edx
	cdq					; this shit writes edx
	mov edx, r12d
	mov r15, rax		; Extend r15d to r15

	; Calculate end of top line (y - matSize/2)
	mov r13d, r8d
	sar r13d, 1

	xor		r10, r10
	mov 	r10d, ecx 	;r10d = y
	sub		r10d, r13d	;r10d = y - matSize/2

	mov		r11d, [rdi+IMAGE_OS_WIDTH]
	imul 	r10d, r11d	; r10d = y * src->width

	mov eax, r11d
	mov r12d, edx
	cdq					; this shit writes edx
	mov edx, r12d
	mov r11, rax		; Extend r11d to r11


	push r10
	push rdx
	push rcx
	sub rsp, 8

	mov	ecx, r8d	; restore ecx
	add rsi, MAT_OS_DATA

	; Set up Src and Mat Pointers
	add		rdi, IMAGE_OS_DATA
	mov  	rdi, [rdi]
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
	jmp cvt_to_short_negative

cvt_to_short_negative:
	mov ecx, eax
	mov eax, 0
	add ax, cx

end_cvt_to_short:

	; Write result (in ax)
	mov [r14], ax


	add rsp, 8
	pop rcx
	pop rdx
	pop r10

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

	mov     r9d, edx		;	r9d = x

	; Correct Line 16-Byte Alignment
		;mov		rax, rdi
		;mov 	edx, 0
		;mov		r8, 16
		;idiv 	r8					; eax = line % 16
	;; OPTIMIZATION
	mov	 rdx,	rdi
	and	 rdx,	0x0F			; eax = line % 16


	and		rdx,	0x0000FFFF	; rdx -> edx

	sub		rdi, rdx
	add		r9d, edx



	; correct x to be x - (matSize / 2)
	mov     r10d, ecx
	sar     r10d, 1			; r10d = matSize / 2
	sub		r9d,  r10d		; r9d = x - matSize / 2

	; Calculate x_aligned
		;mov 	eax, r9d
		;mov 	edx, 0
		;mov	r8, 16
		;idiv 	r8				;	eax = x / 16 |||| edx = x % 16
		;sal	eax, 4			;   eax = x * 16 (aligned)
	;; OPTIMIZATION
	mov	edx,	r9d
	mov	eax,	r9d
	and	edx,	0x0F
	and	eax,	0xFFFFFFF0	; eax / 16  then eax * 16

	xor		r10,  r10
	mov		r10d, eax		;	r10d = x_aligned

	mov 	r12d, edx		;	r12d = x % 16

; Calculate number of sections
		;mov 	eax, ecx		;	eax = matSize
		;mov 	edx, 0
		;mov		r8, 4
		;idiv 	r8				;	eax = matSize / 4 |||| edx = matSize % 4
	;; OPTIMIZATION
	mov	eax, ecx
	sar eax, 2					; 	eax = matSize / 4
	mov	edx, ecx
	and	edx, 0x03


	inc		eax				;	eax = nSections

	mov     r14d, edx		;   r14d = matSize % 4

; Iterate sections
	mov 	r11d, ecx		;  r11d = matSize
	add		rdi, r10        ;  alignedLine
	mov		r8d, 0

	push  rdi
	push  rsi


	mov  r13d, 0 			; r13d = line_x_offset

	; initizalize result
	pxor xmm0, xmm0

loop_sections:
	cmp	r8d, eax
	je	end_loop_sections

	movdqa	xmm1, [rsi]		; load matrix line section
	movdqa  xmm2, [rdi]		; load image line section

	mov ecx, r12d ; <- line_x
	add ecx, r13d ; <- x_offset

	cmp	ecx, 0
	je	check_double_line
	jmp	line_unaligned

line_aligned:
	; Do nothing, all data is in in our registers
	jmp	continue_line

line_unaligned:
line_unaligned_loop:
	call shift_xmm2_right

check_double_line:
; If data is really separated by 16bytes alignment, must do another read, algin and join
	mov	ecx, r9d 	; x
	add ecx, r13d   ; x + offset
	add ecx, r11d	; x + offset + matSize

	cmp ecx, 16		; if(x + matSize > 16) then it is separated
	jle continue_line

	add		rdi, 16
	movdqa	xmm3, [rdi] ; <- Second part of line

	mov ecx, 16
	sub ecx, r12d		; Inverse Alignment
	sub ecx, r13d

	cmp ecx, 0			; if ecx is zero, loop never ends
	je  line_unaligned_loop_2_end
	jg  line_unaligned_loop_2_left
	; ecx is neither zero nor positive
	; ecx = -ecx
	neg ecx

	jmp line_unaligned_loop_2_right


line_unaligned_loop_2_left:
	call shift_xmm3_left
	jmp  line_unaligned_loop_2_end

line_unaligned_loop_2_right:
	call shift_xmm3_right

line_unaligned_loop_2_end:
	por xmm2, xmm3 ;

continue_line:

	; cvt to float
	pshufb xmm2, [shuffle_packed_4_byte_to_float]
	cvtdq2ps xmm2, xmm2

	; increase loop data
	add	rsi,	16
	inc	r8d

	add	r13d,	4
	cmp r13d,   16
	jge increase_line_x

	jmp check_last_section

increase_line_x:		; if more than 16 pixel has been readed, increase line position and reset offset
	mov r13d, 0
	add rdi, 16

check_last_section:
	cmp r8d, eax
	je  is_last_section
	jmp continue_overflow_data

is_last_section: ; Zero overflowing data (outside matrix convolution)
	cmp r14d, 1
	je keep_1_float					; if (matSize % 4 == 1)
	pand xmm2, [and_keep_3_float] 	; if matSize % 4 != 1 then its 3
	jmp continue_overflow_data

keep_1_float:
	pand xmm2, [and_keep_1_float]

continue_overflow_data:

	; Multiply and sum
	mulps  	xmm1, xmm2

	;; OPTMIZATION
	haddps	xmm1, xmm1
	haddps	xmm1, xmm1

		;	movdqa	xmm2, xmm1
		;	movdqa  xmm3, xmm1
		;	movdqa  xmm4, xmm1



		;	pshufb  xmm2, [shuffle_get_2nd_float]
		;	pshufb  xmm3, [shuffle_get_3rd_float]
		;	pshufb  xmm4, [shuffle_get_4th_float]

		;	addss   xmm1, xmm2
		;	addss   xmm1, xmm3
		;	addss	xmm1, xmm4

	; Add to result
	addss	xmm0, xmm1

	jmp loop_sections

end_loop_sections:

	pop  rsi
	pop  rdi

	pop	r15
	pop r14
	pop	r13
	pop	r12
	pop rbx
	pop rbp

	ret			; return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; SHIFTING FUNCTIONS

shift_xmm2_right:	; (ecx times)
	cmp	ecx, 0
	je	shift_xmm2_right_0
	cmp ecx, 1
	je	shift_xmm2_right_1
	cmp ecx, 2
	je	shift_xmm2_right_2
	cmp ecx, 3
	je	shift_xmm2_right_3
	cmp ecx, 4
	je	shift_xmm2_right_4
	cmp ecx, 5
	je	shift_xmm2_right_5
	cmp ecx, 6
	je	shift_xmm2_right_6
	cmp ecx, 7
	je	shift_xmm2_right_7
	cmp ecx, 8
	je	shift_xmm2_right_8
	cmp ecx, 9
	je	shift_xmm2_right_9
	cmp ecx, 10
	je	shift_xmm2_right_10
	cmp ecx, 11
	je	shift_xmm2_right_11
	cmp ecx, 12
	je	shift_xmm2_right_12
	cmp ecx, 13
	je	shift_xmm2_right_13
	cmp ecx, 14
	je	shift_xmm2_right_14
	cmp ecx, 15
	je	shift_xmm2_right_15
	; greater than 16
	pxor xmm2, xmm2
	ret	;

shift_xmm2_right_0:
	ret
shift_xmm2_right_1:
	psrldq xmm2, 1
	ret
shift_xmm2_right_2:
	psrldq xmm2, 2
	ret
shift_xmm2_right_3:
	psrldq xmm2, 3
	ret
shift_xmm2_right_4:
	psrldq xmm2, 4
	ret
shift_xmm2_right_5:
	psrldq xmm2, 5
	ret
shift_xmm2_right_6:
	psrldq xmm2, 6
	ret
shift_xmm2_right_7:
	psrldq xmm2, 7
	ret
shift_xmm2_right_8:
	psrldq xmm2, 8
	ret
shift_xmm2_right_9:
	psrldq xmm2, 9
	ret
shift_xmm2_right_10:
	psrldq xmm2, 10
	ret
shift_xmm2_right_11:
	psrldq xmm2, 11
	ret
shift_xmm2_right_12:
	psrldq xmm2, 12
	ret
shift_xmm2_right_13:
	psrldq xmm2, 13
	ret
shift_xmm2_right_14:
	psrldq xmm2, 14
	ret
shift_xmm2_right_15:
	psrldq xmm2, 15
	ret




shift_xmm3_right:	; (ecx times)
	cmp	ecx, 0
	je	shift_xmm3_right_0
	cmp ecx, 1
	je	shift_xmm3_right_1
	cmp ecx, 2
	je	shift_xmm3_right_2
	cmp ecx, 3
	je	shift_xmm3_right_3
	cmp ecx, 4
	je	shift_xmm3_right_4
	cmp ecx, 5
	je	shift_xmm3_right_5
	cmp ecx, 6
	je	shift_xmm3_right_6
	cmp ecx, 7
	je	shift_xmm3_right_7
	cmp ecx, 8
	je	shift_xmm3_right_8
	cmp ecx, 9
	je	shift_xmm3_right_9
	cmp ecx, 10
	je	shift_xmm3_right_10
	cmp ecx, 11
	je	shift_xmm3_right_11
	cmp ecx, 12
	je	shift_xmm3_right_12
	cmp ecx, 13
	je	shift_xmm3_right_13
	cmp ecx, 14
	je	shift_xmm3_right_14
	cmp ecx, 15
	je	shift_xmm3_right_15
	; greater than 16
	pxor xmm3, xmm3
	ret	;

shift_xmm3_right_0:
	ret
shift_xmm3_right_1:
	psrldq xmm3, 1
	ret
shift_xmm3_right_2:
	psrldq xmm3, 2
	ret
shift_xmm3_right_3:
	psrldq xmm3, 3
	ret
shift_xmm3_right_4:
	psrldq xmm3, 4
	ret
shift_xmm3_right_5:
	psrldq xmm3, 5
	ret
shift_xmm3_right_6:
	psrldq xmm3, 6
	ret
shift_xmm3_right_7:
	psrldq xmm3, 7
	ret
shift_xmm3_right_8:
	psrldq xmm3, 8
	ret
shift_xmm3_right_9:
	psrldq xmm3, 9
	ret
shift_xmm3_right_10:
	psrldq xmm3, 10
	ret
shift_xmm3_right_11:
	psrldq xmm3, 11
	ret
shift_xmm3_right_12:
	psrldq xmm3, 12
	ret
shift_xmm3_right_13:
	psrldq xmm3, 13
	ret
shift_xmm3_right_14:
	psrldq xmm3, 14
	ret
shift_xmm3_right_15:
	psrldq xmm3, 15
	ret

shift_xmm3_left:	; (ecx times)
	cmp	ecx, 0
	je	shift_xmm3_left_0
	cmp ecx, 1
	je	shift_xmm3_left_1
	cmp ecx, 2
	je	shift_xmm3_left_2
	cmp ecx, 3
	je	shift_xmm3_left_3
	cmp ecx, 4
	je	shift_xmm3_left_4
	cmp ecx, 5
	je	shift_xmm3_left_5
	cmp ecx, 6
	je	shift_xmm3_left_6
	cmp ecx, 7
	je	shift_xmm3_left_7
	cmp ecx, 8
	je	shift_xmm3_left_8
	cmp ecx, 9
	je	shift_xmm3_left_9
	cmp ecx, 10
	je	shift_xmm3_left_10
	cmp ecx, 11
	je	shift_xmm3_left_11
	cmp ecx, 12
	je	shift_xmm3_left_12
	cmp ecx, 13
	je	shift_xmm3_left_13
	cmp ecx, 14
	je	shift_xmm3_left_14
	cmp ecx, 15
	je	shift_xmm3_left_15
	; greater than 16
	pxor xmm3, xmm3
	ret	;

shift_xmm3_left_0:
	ret
shift_xmm3_left_1:
	pslldq xmm3, 1
	ret
shift_xmm3_left_2:
	pslldq xmm3, 2
	ret
shift_xmm3_left_3:
	pslldq xmm3, 3
	ret
shift_xmm3_left_4:
	pslldq xmm3, 4
	ret
shift_xmm3_left_5:
	pslldq xmm3, 5
	ret
shift_xmm3_left_6:
	pslldq xmm3, 6
	ret
shift_xmm3_left_7:
	pslldq xmm3, 7
	ret
shift_xmm3_left_8:
	pslldq xmm3, 8
	ret
shift_xmm3_left_9:
	pslldq xmm3, 9
	ret
shift_xmm3_left_10:
	pslldq xmm3, 10
	ret
shift_xmm3_left_11:
	pslldq xmm3, 11
	ret
shift_xmm3_left_12:
	pslldq xmm3, 12
	ret
shift_xmm3_left_13:
	pslldq xmm3, 13
	ret
shift_xmm3_left_14:
	pslldq xmm3, 14
	ret
shift_xmm3_left_15:
	pslldq xmm3, 15
	ret
