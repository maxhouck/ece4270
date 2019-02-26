addiu $a1, $zero, 0x1001
sll $a1, $a1, 16
addiu $a1, $a1, 0x0000
addiu $a0, $zero, 5
sb $a0, 0($a1)
addiu $a0, $zero, 3
sb $a0, 1($a1)
addiu $a0, $zero, 6
sb $a0, 2($a1)
addiu $a0, $zero, 8
sb $a0, 3($a1)
addiu $a0, $zero, 9
sb $a0, 4($a1)
addiu $a0, $zero, 1
sb $a0, 5($a1)
addiu $a0, $zero, 4
sb $a0, 6($a1)
addiu $a0, $zero, 7
sb $a0, 7($a1)
addiu $a0, $zero, 2
sb $a0, 8($a1)
addiu $a0, $zero, A
sb $a0, 9($a1)

LUI $t0, 0x1001
LUI $t3, 0x1001
ADDIU, $a1, $zero, 0xA
ADDIU $a2, $zero, 0
SUB $s0, $a1, 0x1
ADDIU $s1, $zero, 0
ADDIU, $t3, $t3, 0x4 //L1
LW $t1, 0($t0)
LW $t2, 0($t3)
SUB $s2, $t1, $t2
BLTZ $s2, 12
SW $t2, 0($t0)
SW $t1, 0($t3)
ADDIU $s1, $s1, 0x1 //l2
bne $s0, $s1, -32
addiu $t0, $t0, 0x4
addiu $a2, $a2, 0x1
addiu $s1, $zero, 0
add $t3, $zero, $t0
sub $s0, $s0, 0x1
bne $s0, $zero, -56
addiu $v0, $zero, 0xA
syscall
