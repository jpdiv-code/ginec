.inc utils.asm

.section romb

main: .org 0x0000

LDI.w R0, 0
SEED R0

loop:
    CALL clear_on_pressed
    CALL gen_addr
    POP.w R0
    RAND R1
    STR R0, R1
    SYNC
    JMP loop

clear_on_pressed:
    LD.w R0, INPUT_PRESSED
    AND.w R0, BTN_MASK_A
    CMP.w R0, 0
    JZ clear_on_pressed_done
    LDI.w R0, FRAMEBUFFER
    LDI.w R1, 0
    LDI.w R2, SCREEN_WIDTH * SCREEN_HEIGHT
    FILL.w R0, R1, R2
clear_on_pressed_done:
    RET

gen_x:
gen_x_loop:
    LDI.w R0, 0
    RAND R0
    CMP R0, SCREEN_WIDTH
    JNC gen_x_loop
    PUSH.w R0
    RET

gen_y:
gen_y_loop:
    LDI.w R0, 0
    RAND R0
    CMP R0, SCREEN_HEIGHT
    JNC gen_y_loop
    PUSH.w R0
    RET

gen_addr:
    CALL gen_x
    CALL gen_y
    POP.w R0
    LDI.w R2, SCREEN_WIDTH
    MULR.w R0, R2
    POP.w R1
    ADDR.w R0, R1
    ADD.w R0, 0x0100
    PUSH.w R0
    RET
