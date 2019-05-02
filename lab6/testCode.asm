addiu $a1, $zero, 4
lui $a3, 0x1001
sw $a1, 0($a3)
lw $a2, 0($a3)
addiu $v0, $zero, 0xA
syscall
