
@---------------------------------------------------------------------------------
        .text
        .align 4
        .arm
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global memorySet32bit32B
@---------------------------------------------------------------------------------
@	memorySet32bit32B() sets memory in 32B chunks to the given 32bit value
@	r0 ~ output
@	r1 ~ counter (write counter*8 times data -> output)
@	r2 ~ data (32bit data word)
@---------------------------------------------------------------------------------
memorySet32bit32B:
@---------------------------------------------------------------------------------
        stmfd  sp!, { r3 - r9 }

        mov    r3, r2 @ copy the data to seven other registers
        mov    r4, r2
        mov    r5, r2
        mov    r6, r2
        mov    r7, r2
        mov    r8, r2
        mov    r9, r2 @ now r2-r9 contain the data

loop1:  stmia  r0!, { r2, r3, r4, r5, r6, r7, r8, r9 }
        subs   r1, r1, #1
        bne    loop1

        ldmfd  sp!, { r3 - r9 }
        bx     lr
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global memoryCopy32bit32B
@---------------------------------------------------------------------------------
@	memoryCopy32bit32B() copies memory in 32B chunks
@	r0 ~ output
@	r1 ~ counter (copy counter*32 bytes from input to output)
@	r2 ~ input
@---------------------------------------------------------------------------------
memoryCopy32bit32B:
@---------------------------------------------------------------------------------
        stmfd  sp!, { r3 - r10 }

loop2:  ldmia  r2!, { r3, r4, r5, r6, r7, r8, r9, r10 }
        stmia  r0!, { r3, r4, r5, r6, r7, r8, r9, r10 }
	subs   r1, r1, #1
        bne    loop2

        ldmfd  sp!, { r3 - r10 }
        bx     lr
@---------------------------------------------------------------------------------
