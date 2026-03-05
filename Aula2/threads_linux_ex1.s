	.file	"threads_linux_ex1.c"
	.text
	.globl	valor_total
	.bss
	.align 4
	.type	valor_total, @object
	.size	valor_total, 4
valor_total:
	.zero	4
	.globl	em
	.align 32
	.type	em, @object
	.size	em, 40
em:
	.zero	40
	.text
	.globl	alteraValor
	.type	alteraValor, @function
alteraValor:
.LFB6:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	leaq	em(%rip), %rax
	movq	%rax, %rdi
	call	pthread_mutex_lock@PLT
	movl	valor_total(%rip), %edx
	movl	-20(%rbp), %eax
	addl	%edx, %eax
	movl	%eax, valor_total(%rip)
	movl	valor_total(%rip), %eax
	movl	%eax, -4(%rbp)
	leaq	em(%rip), %rax
	movq	%rax, %rdi
	call	pthread_mutex_unlock@PLT
	movl	-4(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	alteraValor, .-alteraValor
	.globl	defineValor
	.type	defineValor, @function
defineValor:
.LFB7:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	leaq	em(%rip), %rax
	movq	%rax, %rdi
	call	pthread_mutex_lock@PLT
	movl	-4(%rbp), %eax
	movl	%eax, valor_total(%rip)
	leaq	em(%rip), %rax
	movq	%rax, %rdi
	call	pthread_mutex_unlock@PLT
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	defineValor, .-defineValor
	.globl	tela
	.bss
	.align 32
	.type	tela, @object
	.size	tela, 40
tela:
	.zero	40
	.text
	.globl	aloca_tela
	.type	aloca_tela, @function
aloca_tela:
.LFB8:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	tela(%rip), %rax
	movq	%rax, %rdi
	call	pthread_mutex_lock@PLT
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	aloca_tela, .-aloca_tela
	.globl	libera_tela
	.type	libera_tela, @function
libera_tela:
.LFB9:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	leaq	tela(%rip), %rax
	movq	%rax, %rdi
	call	pthread_mutex_unlock@PLT
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	libera_tela, .-libera_tela
	.section	.rodata
	.align 8
.LC0:
	.string	"Ultimo lido foi %d, digite <enter> para alterar\n"
	.text
	.globl	thread_mostra_status
	.type	thread_mostra_status, @function
thread_mostra_status:
.LFB10:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
.L7:
	movl	$1, %edi
	call	sleep@PLT
	call	aloca_tela
	movl	$1, %edi
	call	alteraValor
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
	call	libera_tela
	nop
	jmp	.L7
	.cfi_endproc
.LFE10:
	.size	thread_mostra_status, .-thread_mostra_status
	.section	.rodata
.LC1:
	.string	"Digite novo valor:"
	.text
	.globl	thread_le_teclado
	.type	thread_le_teclado, @function
thread_le_teclado:
.LFB11:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$1008, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
.L9:
	movq	stdin(%rip), %rdx
	leaq	-1008(%rbp), %rax
	movl	$1000, %esi
	movq	%rax, %rdi
	call	fgets@PLT
	call	aloca_tela
	leaq	.LC1(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movq	stdin(%rip), %rdx
	leaq	-1008(%rbp), %rax
	movl	$1000, %esi
	movq	%rax, %rdi
	call	fgets@PLT
	leaq	-1008(%rbp), %rax
	movq	%rax, %rdi
	call	atoi@PLT
	movl	%eax, %edi
	call	defineValor
	call	libera_tela
	nop
	jmp	.L9
	.cfi_endproc
.LFE11:
	.size	thread_le_teclado, .-thread_le_teclado
	.globl	main
	.type	main, @function
main:
.LFB12:
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	%edi, -36(%rbp)
	movq	%rsi, -48(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	leaq	-24(%rbp), %rax
	movl	$0, %ecx
	leaq	thread_mostra_status(%rip), %rdx
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_create@PLT
	leaq	-16(%rbp), %rax
	movl	$0, %ecx
	leaq	thread_le_teclado(%rip), %rdx
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_create@PLT
	movq	-24(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_join@PLT
	movq	-16(%rbp), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	pthread_join@PLT
	movl	$0, %eax
	movq	-8(%rbp), %rdx
	subq	%fs:40, %rdx
	je	.L13
	call	__stack_chk_fail@PLT
.L13:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
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
