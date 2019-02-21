addiu $a0, $zero, 10 //store 10 in register 0
addiu $a1, $zero, 0 //initialize out the general purpose registers
addiu $a2, $zero, 1 //
addiu $a0, $a0, -1 //decrement counter
beq $a0, $zero, 5
add $a3, $a1, $a2 //add r1 and r2 and store in 3
addiu $a1, $a2, 0 //move r2 to r1
addiu $a2, $a3, 0 //move r3 to r2
beq $zero, $zero, -5
addiu $v0, $zero, 0xA //end program
syscall //result is in $a3 or r6
