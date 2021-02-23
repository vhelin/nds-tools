@---------------------------------------------------------------------------------
        .text
        .align 4
        .thumb
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global swiDelay
        .thumb_func
@---------------------------------------------------------------------------------
swiDelay:
@---------------------------------------------------------------------------------
        swi     0x03
        bx      lr
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global swiIntrWait
	.thumb_func
@---------------------------------------------------------------------------------
swiIntrWait:
@---------------------------------------------------------------------------------
        swi     0x04
        bx      lr
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global swiWaitForVBlank
	.thumb_func
@---------------------------------------------------------------------------------
swiWaitForVBlank:
@---------------------------------------------------------------------------------
        swi     0x05
        bx      lr
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global swiDivide
	.thumb_func
@---------------------------------------------------------------------------------
swiDivide:
@---------------------------------------------------------------------------------
        swi     0x09
        bx      lr
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global swiRemainder
	.thumb_func
@---------------------------------------------------------------------------------
swiRemainder:
@---------------------------------------------------------------------------------
        swi     0x09
        mov     r0, r1
        bx      lr
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global swiDivMod
	.thumb_func
@---------------------------------------------------------------------------------
swiDivMod:
@---------------------------------------------------------------------------------
        push    { r2, r3 }
        swi     0x09
        pop     { r2, r3 }
        str     r0, [r2]
        str     r1, [r3]
        bx      lr
@---------------------------------------------------------------------------------

@---------------------------------------------------------------------------------
        .global swiSqrt
	.thumb_func
@---------------------------------------------------------------------------------
swiSqrt:
@---------------------------------------------------------------------------------
        swi     0x0D
        bx      lr
@---------------------------------------------------------------------------------
