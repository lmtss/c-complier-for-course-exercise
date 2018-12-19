.data 
a:.word 1 
.text 
.globl main 
gcd: 
addiu $sp, $sp, -16
sw $ra, 12($sp)
sw $fp, 8($sp)
move $fp, $sp
beq $a1, $zero, L0
nop 
j L1
nop 
L0: 
move $v0, $a0
move $sp, $fp
lw $ra, 12($sp)
lw $fp, 8($sp)
addiu $sp, $sp, 16
jr $ra
nop 
L1: 
div $t0, $a0, $a1
mul $t0, $t0, $a1
sub $t0, $a0, $t0
sw $a0, 16($fp)
sw $a1, 20($fp)
move $a0, $a1
move $a1, $t0
jal gcd
nop 
move $t1, $v0
move $v0, $t1
move $sp, $fp
lw $ra, 12($sp)
lw $fp, 8($sp)
addiu $sp, $sp, 16
jr $ra
nop 
main: 
addiu $sp, $sp, -20
sw $ra, 16($sp)
sw $fp, 12($sp)
move $fp, $sp
li $s0, 30
li $v0, 1
li $a0, 2
syscall 
li $v0, 11
li $a0, 32
syscall 
li $v0, 1
li $a0, -30
syscall 
li $v0, 11
li $a0, 10
syscall 
li $s2, 123456
li $s1, 7890
move $s7, $0
L2: 
li $t9, 10
blt $s7, $t9, L3
nop 
j L4
nop 
L3: 
li $v0, 5
syscall 
move $s2, $v0
li $v0, 1
move $a0, $s2
syscall 
addi $s7, $s7, 1
j L2
nop 
L4: 
move $v0, $0
move $sp, $fp
lw $ra, 16($sp)
lw $fp, 12($sp)
addiu $sp, $sp, 20
jr $ra
nop 
