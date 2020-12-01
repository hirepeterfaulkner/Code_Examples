; Insertion Sort in Assembly
; n = 4

 .main:
mov r13, 0
mov r0, 9
st r0, [r13]
mov r0, 2
st r0, 4[r13]
mov r0, 3
st r0, 8[r13]
mov r0, 5
st r0, 12[r13]
mov r1, 0
b .for


.for:
mul r4, r1, 4
add r4, r4, r13
ld r3, [r4]
sub r2, r1, 1
b .whilecheck

.whilecheck:
cmp r2, 0
bgt .whilecheck2
beq .whilecheck2
b .afterfor


.whilecheck2:
mul r5, r2, 4 
add r5, r5, r13
ld r6, [r5]
cmp r6, r3
bgt .while
b .afterfor


.while:
mul r8, r2, 4
ld r9, [r8]
add r8, r8, 4
st r9, [r8]
sub r2, r2, 1
b .whilecheck


.afterfor:
add r7, r2, 1
mul r7, r7, 4
add r7, r7, r13
st r3, [r7] 
mov r10, 4
add r1, r1, 1
cmp r10, r1
bgt .for