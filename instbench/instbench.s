	.file	"instbench.cpp"
	.text
	.local	_ZStL8__ioinit
	.comm	_ZStL8__ioinit,1,1
	.section	.rodata
	.align 4
	.type	_ZL1N, @object
	.size	_ZL1N, 4
_ZL1N:
	.long	100000
	.text
	.globl	_Z5rdtscv
	.type	_Z5rdtscv, @function
_Z5rdtscv:
.LFB1791:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
#APP
# 12 "instbench.cpp" 1
	rdtsc
# 0 "" 2
#NO_APP
	movl	%eax, -8(%rbp)
	movl	%edx, -4(%rbp)
	movl	-4(%rbp), %eax
	salq	$32, %rax
	movq	%rax, %rdx
	movl	-8(%rbp), %eax
	orq	%rdx, %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1791:
	.size	_Z5rdtscv, .-_Z5rdtscv
	.globl	_Z5rdpmcj
	.type	_Z5rdpmcj, @function
_Z5rdpmcj:
.LFB1792:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)
	movl	-20(%rbp), %eax
	movl	%eax, %ecx
#APP
# 19 "instbench.cpp" 1
	rdpmc
# 0 "" 2
#NO_APP
	movl	%eax, -8(%rbp)
	movl	%edx, -4(%rbp)
	movl	-4(%rbp), %eax
	salq	$32, %rax
	movq	%rax, %rdx
	movl	-8(%rbp), %eax
	orq	%rdx, %rax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1792:
	.size	_Z5rdpmcj, .-_Z5rdpmcj
	.globl	perf_fd
	.data
	.align 4
	.type	perf_fd, @object
	.size	perf_fd, 4
perf_fd:
	.long	-1
	.text
	.type	_ZL15perf_event_openP15perf_event_attriiim, @function
_ZL15perf_event_openP15perf_event_attriiim:
.LFB1793:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movl	%edx, -16(%rbp)
	movl	%ecx, -20(%rbp)
	movq	%r8, -32(%rbp)
	movq	-32(%rbp), %rdi
	movl	-20(%rbp), %esi
	movl	-16(%rbp), %ecx
	movl	-12(%rbp), %edx
	movq	-8(%rbp), %rax
	movq	%rdi, %r9
	movl	%esi, %r8d
	movq	%rax, %rsi
	movl	$298, %edi
	movl	$0, %eax
	call	syscall@PLT
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1793:
	.size	_ZL15perf_event_openP15perf_event_attriiim, .-_ZL15perf_event_openP15perf_event_attriiim
	.section	.rodata
.LC0:
	.string	"perf_event_open"
	.text
	.globl	_Z18init_cycle_counterv
	.type	_Z18init_cycle_counterv, @function
_Z18init_cycle_counterv:
.LFB1794:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$144, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	-144(%rbp), %rax
	movl	$128, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	memset@PLT
	movl	$0, -144(%rbp)
	movl	$128, -140(%rbp)
	movq	$0, -136(%rbp)
	movzbl	-104(%rbp), %eax
	orl	$32, %eax
	movb	%al, -104(%rbp)
	leaq	-144(%rbp), %rax
	movl	$0, %r8d
	movl	$-1, %ecx
	movl	$-1, %edx
	movl	$0, %esi
	movq	%rax, %rdi
	call	_ZL15perf_event_openP15perf_event_attriiim
	movl	%eax, perf_fd(%rip)
	movl	perf_fd(%rip), %eax
	cmpl	$-1, %eax
	jne	.L10
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	call	perror@PLT
	movl	$1, %edi
	call	exit@PLT
.L10:
	nop
	movq	-8(%rbp), %rax
	subq	%fs:40, %rax
	je	.L9
	call	__stack_chk_fail@PLT
.L9:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1794:
	.size	_Z18init_cycle_counterv, .-_Z18init_cycle_counterv
	.globl	main
	.type	main, @function
main:
.LFB1795:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	call	_Z18init_cycle_counterv
#APP
# 51 "instbench.cpp" 1
	pxor %xmm0, %xmm0
	pxor %xmm1, %xmm1
	pxor %xmm2, %xmm2
	pxor %xmm3, %xmm3
	pxor %xmm4, %xmm4
	pxor %xmm5, %xmm5
	pxor %xmm6, %xmm6
	pxor %xmm7, %xmm7
	pxor %xmm8, %xmm8
	pxor %xmm9, %xmm9
	pxor %xmm10, %xmm10
	pxor %xmm11, %xmm11
	pxor %xmm12, %xmm12
	
# 0 "" 2
#NO_APP
	call	_Z5rdtscv
	movq	%rax, -24(%rbp)
	movl	perf_fd(%rip), %eax
	leaq	-40(%rbp), %rcx
	movl	$8, %edx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	read@PLT
	movl	$0, -44(%rbp)
	jmp	.L12
.L13:
#APP
# 73 "instbench.cpp" 1
	.rept 1000
	sha256rnds2 %xmm1, %xmm2
	sha256rnds2 %xmm3, %xmm2
	sha256rnds2 %xmm4, %xmm2
	sha256rnds2 %xmm5, %xmm2
	sha256rnds2 %xmm6, %xmm2
	sha256rnds2 %xmm7, %xmm2
	sha256rnds2 %xmm8, %xmm2
	sha256rnds2 %xmm9, %xmm2
	sha256rnds2 %xmm10, %xmm2
	sha256rnds2 %xmm11, %xmm2
	sha256rnds2 %xmm12, %xmm2
	.endr
	
# 0 "" 2
#NO_APP
	addl	$1, -44(%rbp)
.L12:
	cmpl	$99999, -44(%rbp)
	jle	.L13
	call	_Z5rdtscv
	movq	%rax, -16(%rbp)
	movl	perf_fd(%rip), %eax
	leaq	-32(%rbp), %rcx
	movl	$8, %edx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	read@PLT
	movq	-16(%rbp), %rax
	subq	-24(%rbp), %rax
	testq	%rax, %rax
	js	.L14
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
	jmp	.L15
.L14:
	movq	%rax, %rdx
	shrq	%rdx
	andl	$1, %eax
	orq	%rax, %rdx
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
.L15:
	movsd	.LC1(%rip), %xmm1
	divsd	%xmm1, %xmm0
	movsd	.LC2(%rip), %xmm1
	divsd	%xmm1, %xmm0
	movsd	.LC3(%rip), %xmm1
	divsd	%xmm1, %xmm0
	movq	%xmm0, %rax
	movq	%rax, %xmm0
	leaq	_ZSt4cout(%rip), %rax
	movq	%rax, %rdi
	call	_ZNSolsEd@PLT
	movq	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_@GOTPCREL(%rip), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZNSolsEPFRSoS_E@PLT
	movq	-32(%rbp), %rax
	movq	-40(%rbp), %rdx
	subq	%rdx, %rax
	testq	%rax, %rax
	js	.L16
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rax, %xmm0
	jmp	.L17
.L16:
	movq	%rax, %rdx
	shrq	%rdx
	andl	$1, %eax
	orq	%rax, %rdx
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rdx, %xmm0
	addsd	%xmm0, %xmm0
.L17:
	movsd	.LC1(%rip), %xmm1
	divsd	%xmm1, %xmm0
	movsd	.LC2(%rip), %xmm1
	divsd	%xmm1, %xmm0
	movsd	.LC3(%rip), %xmm1
	divsd	%xmm1, %xmm0
	movq	%xmm0, %rax
	movq	%rax, %xmm0
	leaq	_ZSt4cout(%rip), %rax
	movq	%rax, %rdi
	call	_ZNSolsEd@PLT
	movq	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_@GOTPCREL(%rip), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	_ZNSolsEPFRSoS_E@PLT
	movl	$0, %eax
	movq	-8(%rbp), %rdx
	subq	%fs:40, %rdx
	je	.L19
	call	__stack_chk_fail@PLT
.L19:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1795:
	.size	main, .-main
	.type	_Z41__static_initialization_and_destruction_0ii, @function
_Z41__static_initialization_and_destruction_0ii:
.LFB2325:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	cmpl	$1, -4(%rbp)
	jne	.L22
	cmpl	$65535, -8(%rbp)
	jne	.L22
	leaq	_ZStL8__ioinit(%rip), %rax
	movq	%rax, %rdi
	call	_ZNSt8ios_base4InitC1Ev@PLT
	leaq	__dso_handle(%rip), %rax
	movq	%rax, %rdx
	leaq	_ZStL8__ioinit(%rip), %rax
	movq	%rax, %rsi
	movq	_ZNSt8ios_base4InitD1Ev@GOTPCREL(%rip), %rax
	movq	%rax, %rdi
	call	__cxa_atexit@PLT
.L22:
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2325:
	.size	_Z41__static_initialization_and_destruction_0ii, .-_Z41__static_initialization_and_destruction_0ii
	.type	_GLOBAL__sub_I__Z5rdtscv, @function
_GLOBAL__sub_I__Z5rdtscv:
.LFB2326:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$65535, %esi
	movl	$1, %edi
	call	_Z41__static_initialization_and_destruction_0ii
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2326:
	.size	_GLOBAL__sub_I__Z5rdtscv, .-_GLOBAL__sub_I__Z5rdtscv
	.section	.init_array,"aw"
	.align 8
	.quad	_GLOBAL__sub_I__Z5rdtscv
	.section	.rodata
	.align 8
.LC1:
	.long	0
	.long	1090021888
	.align 8
.LC2:
	.long	0
	.long	1083129856
	.align 8
.LC3:
	.long	0
	.long	1076232192
	.hidden	__dso_handle
	.ident	"GCC: (GNU) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
