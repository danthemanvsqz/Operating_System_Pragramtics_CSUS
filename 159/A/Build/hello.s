# hello.s
# as -o hello.o hello.s && link386 -nostartfiles -userapp -o hello.out hello.o && ./bin2hex.pl hello.out 
# link386 -nostartfiles -userapp -o hello.out hello.o
# ./bin2hex.pl hello.out 
# (to generate hello.out.hex)
#
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#       stdout PID (6) is HARD-CODED (see below)
# !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
# GNU assembler syntax, usage, etc. shell> as --help
# http://www.cs.utah.edu/dept/old/texinfo/as/as_toc.html

.data			# data section in memory is after .text
msg:			# 1st is our local msg_t msg
   .rept 8		# repeat 8 times (8 integers 1st in msg)
   .long 0		# int msg.sender, msg.send_tick, msg.numbers[6]
   .endr		# end repetition
   #.ascii "Hallo, mein name ist a.out!\n\0" # msg.bytes (29 chars)
   .rept 101		# repeat 101 times (msg.bytes = 101 Bytes)
   .ascii "\0"		# padding 101 nulls
   .endr		# end repetition
msg2:			# 1st is our local msg_t msg
   .rept 8		# repeat 8 times (8 integers 1st in msg)
   .long 0		# int msg.sender, msg.send_tick, msg.numbers[6]
   .endr		# end repetition
   .ascii "Hallo, mein name ist a.out!\n\0" # msg.bytes (29 chars)
   .rept 72		# repeat 72 times (29+72=101 msg.bytes)
   .ascii "\0"		# padding 72 nulls
   .endr		# end repetition

.text      		# text/code section of program
.global _start  	# declare that _start is our main()
_start:            	# instructions starts here

# calc my msg addr and save copies to pop later when MsgSnd/MsgRcv
   pushl %esp		# esp is 4KB from beginning
   popl	%edx		# pop it into edx to subtract
   subl $0x20000000, %edx
   movl	$msg, %ecx	# addr of msg is 0x80000080+y since
   subl	$0x80000080, %ecx # 2G set by link386, skip 128 B header
   addl	%edx, %ecx	# add 14M+ is actual (e.g., 0xe00000+y)
   pushl %ecx		# save a copy
   pushl %ecx		# save another copy
   pushl %ecx		# save another copy
   pushl %ecx		# save another copy
   
# Stdout in irq34.c needs to reply this proc (use msg.sender)
   popl	%eax		# actual addr of msg (14M+)
   int	$55		# MSGRCV_INTR
   movl 8(%eax), %eax # num[0]

# stdout PID is hard-coded, need to check with its Spawn order
   #movl	$6, %eax	# mboxes ID (mid) is stdout
   popl	%ebx		# actual addr of msg (14M+)
   addl  $133, %ebx
   int	$54		# MSGSND_INTR

# Stdout in irq34.c needs to reply this proc (use msg.sender)
   popl	%eax		# actual addr of msg (14M+)
   int	$55		# MSGRCV_INTR

# exit with msg addr
   movl $99, %eax
   int	$58		# EXIT_INTR


