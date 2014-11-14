# PFtest.s
# PageFault test, used in OS project phase 0xA
#
# as -o PFtest.o PFtest.s
# link386 -nostartfiles -userapp -o PFtest.out PFtest.o
# bin2hex.pl PFtest.out

# data section has msg0, msg1, msg2, and msg3
.data

msg0: 
      .rept 8				# repeat 8 times (8 integers 1st in msg)
      .long 0				# int msg.sender, msg.send_tick, msg.numbers[6]
      .endr				# end repetition
      .rept 101				# repeat 101 times
      .ascii "\0"			# padding 101 nulls
      .endr				# end repetition
msg1: # "Writing what parent gives to addr 2G+12K, reading it back.\n"
      .rept 8				# repeat 8 times (8 integers 1st in msg)
      .long 0				# int msg.sender, msg.send_tick, msg.numbers[6]
      .endr				# end repetition
      .rept 101				# repeat 101 times
      .ascii "\0"			# padding 101 nulls
      .endr				# end repetition
msg2: #"Adding 100 to what's read back, writing it to addr 2.1G, reading it back.\n"
      .rept 8				# repeat 8 times (8 integers 1st in msg)
      .long 0				# int msg.sender, msg.send_tick, msg.numbers[6]
      .endr				# end repetition
      .rept 101				# repeat 101 times
      .ascii "\0"			# padding 101 nulls
      .endr				# end repetition
msg3: #"Exiting with what's read back as the exit code.\n"
      .rept 8				# repeat 8 times (8 integers 1st in msg)
      .long 0				# int msg.sender, msg.send_tick, msg.numbers[6]
      .endr				# end repetition
      .rept 101				# repeat 101 times
      .ascii "\0"			# padding 101 nulls
      .endr				# end repetition

.text   # text section contains code
.global _start

_start:                         # equivalent to main()

# 1st get the _info_ from the parent process

# sleep for a number of seconds, the number matches the 'info'

# show msg1 to the terminal

# write the info to virtual location 2G+12K

# read it back at virtual location 2G+12K

# show msg2 to the terminal

# add 100 to what's read back

# write the info to virtual location 2.1G

# read it back at virtual location 2.1G

# show msg3 to the terminal

# exit with code being what's read back
movl $99, %eax
int $58

