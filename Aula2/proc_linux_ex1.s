	.file	"proc_linux_ex1.c"
	.text
	.globl	value
	.data
	.align 4
	.type	value, @object
	.size	value, 4
value:
	.long	5
	.section	.rodata
.LC0:
	.string	"PARENT: value = %d\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB6:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	call	fork@PLT
	movl	%eax, -4(%rbp)
	cmpl	$0, -4(%rbp)
	jne	.L2
	movl	value(%rip), %eax
	addl	$15, %eax
	movl	%eax, value(%rip)
	movl	$0, %eax
	jmp	.L3
.L2:
	cmpl	$0, -4(%rbp)
	jle	.L4
	movl	$0, %edi
	call	wait@PLT
	movl	value(%rip), %eax
	movl	%eax, %esi
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	$0, %eax
	jmp	.L3
.L4:
	movl	$0, %eax
.L3:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
