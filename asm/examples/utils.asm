.macro STORE
    PUSH.w R0
    PUSH.w R1
    PUSH.w R2
    PUSH.w R3
.endmacro

.macro RESTORE
    POP.w R3
    POP.w R2
    POP.w R1
    POP.w R0
.endmacro

.equ SCREEN_WIDTH, 180
.equ SCREEN_HEIGHT, 136
.equ FRAMEBUFFER, 0x0100

.equ INPUT_DOWN, 0x0000
.equ INPUT_PRESSED, 0x0002
.equ INPUT_RELEASED, 0x0004
.equ BTN_MASK_A, 0x0010
