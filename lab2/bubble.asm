addiu $a1, $zero, 0x1001
sll $a1, $a1, 16
addiu $a1, $a1, 0x0000
addiu $a0, $zero, 5
sb $a0, 0($a1)
addiu $a0, $zero, 3
sb $a0, 4($a1)
addiu $a0, $zero, 6
sb $a0, 8($a1)
addiu $a0, $zero, 8
sb $a0, 12($a1)
addiu $a0, $zero, 9
sb $a0, 16($a1)
addiu $a0, $zero, 1
sb $a0, 20($a1)
addiu $a0, $zero, 4
sb $a0, 24($a1)
addiu $a0, $zero, 7
sb $a0, 28($a1)
addiu $a0, $zero, 2
sb $a0, 32($a1)
addiu $a0, $zero, 10
sb $a0, 36($a1)
addiu $s7, $zero, 1
addiu $t0, $zero, 0x1001
sll $t0, $t0, 16
addiu $t0, $t0, 0x0000
addiu $t3, $zero, 0x1001
sll $t3, $t3, 16
addiu $t3, $t3, 0x0000
ADDIU, $a1, $zero, 10
ADDIU $a2, $zero, 0
SUB $s0, $a1, $s7
ADDIU $s1, $zero, 0
ADDIU, $t3, $t3, 4 //L1
LW $t1, 0($t0)
LW $t2, 0($t3)
SUB $s2, $t1, $t2
BLTZ $s2, 3 //12
SW $t2, 0($t0)
SW $t1, 0($t3)
ADDIU $s1, $s1, 1 //l2
bne $s0, $s1, -8 //32
addiu $t0, $t0, 4
addiu $a2, $a2, 1
addiu $s1, $zero, 0
add $t3, $zero, $t0
sub $s0, $s0, $s7
bne $s0, $zero, -14 //56
addiu $v0, $zero, 10
syscall
