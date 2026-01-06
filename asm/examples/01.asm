.inc 01-utils.asm

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
    BEGIN
    LD.w R0, INPUT_PRESSED
    AND.w R0, BTN_MASK_A
    CMP.w R0, 0
    JZ clear_on_pressed_done
    LDI.w R0, FRAMEBUFFER
    LDI.w R1, 0
    LDI.w R2, SCREEN_WIDTH * SCREEN_HEIGHT
    FILL.w R0, R1, R2
clear_on_pressed_done:
    RET_R0

gen_x:
    BEGIN
gen_x_loop:
    LDI.w R0, 0
    RAND R0
    CMP R0, SCREEN_WIDTH
    JNC gen_x_loop
    RET_R0

gen_y:
    BEGIN
gen_y_loop:
    LDI.w R0, 0
    RAND R0
    CMP R0, SCREEN_HEIGHT
    JNC gen_y_loop
    RET_R0

gen_addr:
    BEGIN
    CALL gen_x
    CALL gen_y
    POP.w R0
    LDI.w R2, SCREEN_WIDTH
    MULR.w R0, R2
    POP.w R1
    ADDR.w R0, R1
    ADD.w R0, 0x0100
    RET_R0
