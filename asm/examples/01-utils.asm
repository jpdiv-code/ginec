.macro BEGIN
    PUSH.w R0
    PUSH.w R1
    PUSH.w R2
    PUSH.w R3
.endmacro

.macro RET_R0
    POP.w R3
    POP.w R2
    POP.w R1
    PUSH.w R0
    SWP.w
    POP.w R0
    SWP.w
    RET
.endmacro

.equ SCREEN_WIDTH, 180
.equ SCREEN_HEIGHT, 136
.equ FRAMEBUFFER, 0x0100

.equ INPUT_PRESSED, 0x0002
.equ BTN_MASK_A, 0x0010

.section romb

EXAMPLE_STRING: .org 0x0090
.string "Hello, World!"
