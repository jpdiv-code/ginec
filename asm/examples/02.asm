.inc utils.asm

; Game constants
.equ CHAR_SIZE, 8
.equ CHAR_COLOR, 0x15
.equ BG_COLOR, 0x00
.equ GRAVITY, 1
.equ JUMP_STRENGTH, 8
.equ MOVE_SPEED, 2
.equ MAX_FALL_SPEED, 8

; Input button masks (based on common game controller layout)
.equ BTN_MASK_UP, 0x0001
.equ BTN_MASK_DOWN, 0x0002
.equ BTN_MASK_LEFT, 0x0004
.equ BTN_MASK_RIGHT, 0x0008
.equ BTN_MASK_Z, 0x0020

; RAM addresses for game state
.equ CHAR_X, 0x0010
.equ CHAR_Y, 0x0012
.equ CHAR_VX, 0x0014
.equ CHAR_VY, 0x0016
.equ IS_GROUNDED, 0x0018

.section romb

main: .org 0x0000

; Initialize character position in center
LDI.w R0, 86  ; (180 - 8) / 2
ST.w CHAR_X, R0
LDI.w R0, 64
ST.w CHAR_Y, R0
LDI.w R0, 0
ST.w CHAR_VX, R0
ST.w CHAR_VY, R0
ST.w IS_GROUNDED, R0

game_loop:
    CALL clear_screen
    CALL handle_input
    CALL apply_gravity
    CALL update_position
    CALL check_collisions
    CALL draw_character
    SYNC
    JMP game_loop

; Clear the screen
clear_screen:
    LDI.w R0, FRAMEBUFFER
    LDI.w R1, BG_COLOR
    LDI.w R2, SCREEN_WIDTH * SCREEN_HEIGHT
    FILL R0, R1, R2
    RET

; Handle player input
handle_input:
    LD.w R0, INPUT_DOWN
    
    ; Check Z for jump (only if grounded)
    LDI.w R1, BTN_MASK_Z
    ANDR.w R1, R0
    CMP.w R1, 0
    JZ check_left
    LD.w R2, IS_GROUNDED
    CMP.w R2, 0
    JZ check_left
    ; Apply jump (use two's complement for negative)
    LDI.w R2, 0xFFF8  ; -8 in two's complement
    ST.w CHAR_VY, R2
    LDI.w R2, 0
    ST.w IS_GROUNDED, R2

check_left:
    ; Check left
    LDI.w R1, BTN_MASK_LEFT
    ANDR.w R1, R0
    CMP.w R1, 0
    JZ check_right
    LDI.w R2, 0xFFFE  ; -2 in two's complement
    ST.w CHAR_VX, R2
    JMP input_done

check_right:
    ; Check right
    LDI.w R1, BTN_MASK_RIGHT
    ANDR.w R1, R0
    CMP.w R1, 0
    JZ input_done
    LDI.w R2, MOVE_SPEED
    ST.w CHAR_VX, R2

input_done:
    RET

; Apply gravity to velocity
apply_gravity:
    LD.w R0, CHAR_VY
    INC.w R0
    ST.w CHAR_VY, R0
    ; Cap fall speed only if positive (check if >= 32768, which is negative in signed)
    CMP.w R0, 0x8000
    JNC gravity_done  ; If negative (bit 15 set), skip cap
    ; Now we know it's positive, cap it
    CMP.w R0, MAX_FALL_SPEED
    JNC gravity_cap
    JMP gravity_done
gravity_cap:
    LDI.w R0, MAX_FALL_SPEED
    ST.w CHAR_VY, R0
gravity_done:
    RET

; Update character position based on velocity
update_position:
    ; Update X position
    LD.w R0, CHAR_X
    LD.w R1, CHAR_VX
    ADDR.w R0, R1
    ST.w CHAR_X, R0
    ; Reset horizontal velocity
    LDI.w R0, 0
    ST.w CHAR_VX, R0
    
    ; Update Y position
    LD.w R0, CHAR_Y
    LD.w R1, CHAR_VY
    ADDR.w R0, R1
    ST.w CHAR_Y, R0
    RET

; Check collisions with screen borders
check_collisions:
    ; Check left wall (X < 0)
    LD.w R0, CHAR_X
    CMP.w R0, 0x8000  ; Check if negative (signed)
    JC check_right_wall  ; Jump if positive (< 0x8000)
    LDI.w R0, 0
    ST.w CHAR_X, R0

check_right_wall:
    ; Check right wall (X > SCREEN_WIDTH - CHAR_SIZE)
    LD.w R0, CHAR_X
    CMP.w R0, SCREEN_WIDTH - CHAR_SIZE
    JC check_floor  ; Jump if less than limit
    ; Also skip if negative
    CMP.w R0, 0x8000
    JNC check_floor
fix_right_wall:
    LDI.w R0, SCREEN_WIDTH - CHAR_SIZE
    ST.w CHAR_X, R0

check_floor:
    ; Check floor (Y >= SCREEN_HEIGHT - CHAR_SIZE)
    LD.w R0, CHAR_Y
    ; Skip if negative
    CMP.w R0, 0x8000
    JNC check_ceiling
    ; Check if hit floor
    CMP.w R0, SCREEN_HEIGHT - CHAR_SIZE
    JC air_state  ; Jump if less than floor
    ; Hit floor
fix_floor:
    LDI.w R0, SCREEN_HEIGHT - CHAR_SIZE
    ST.w CHAR_Y, R0
    LDI.w R0, 0
    ST.w CHAR_VY, R0
    LDI.w R0, 1
    ST.w IS_GROUNDED, R0
    JMP collision_done
air_state:
    ; In air
    LDI.w R0, 0
    ST.w IS_GROUNDED, R0
    JMP check_ceiling

check_ceiling:
    ; Check ceiling (Y < 0)
    LD.w R0, CHAR_Y
    CMP.w R0, 0x8000  ; Check if negative (signed)
    JC collision_done  ; Jump if positive
    LDI.w R0, 0
    ST.w CHAR_Y, R0
    ST.w CHAR_VY, R0

collision_done:
    RET

; Draw the character square
draw_character:
    LD.w R0, CHAR_Y
    ; Calculate starting screen address: FRAMEBUFFER + Y * SCREEN_WIDTH + X
    LDI.w R1, SCREEN_WIDTH
    MULR.w R0, R1
    LD.w R1, CHAR_X
    ADDR.w R0, R1
    ADD.w R0, FRAMEBUFFER
    ST.w 0x0020, R0  ; Store base address temporarily
    
    ; Draw CHAR_SIZE rows
    LDI.w R3, 0  ; Row counter
draw_row_loop:
    CMP.w R3, CHAR_SIZE
    JNC draw_done
    
    ; Get current row address
    LD.w R0, 0x0020
    ; Draw CHAR_SIZE pixels in this row
    LDI.w R1, CHAR_COLOR
    LDI.w R2, CHAR_SIZE
    FILL R0, R1, R2
    
    ; Move to next row (add SCREEN_WIDTH to address)
    LD.w R0, 0x0020
    ADD.w R0, SCREEN_WIDTH
    ST.w 0x0020, R0
    
    INC.w R3
    JMP draw_row_loop

draw_done:
    RET
