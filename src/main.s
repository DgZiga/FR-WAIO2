.gba
.thumb
.open "./BPRE0.gba","./multi.gba", 0x08000000

.org 0x08081FF8 ;insert hijack to customize npc approach
.thumb
.align 2
LDR r1, =custom_movement_hijack|1
BX r1
.pool

.org 0x08081BB0 ;hijack double battle check
.thumb
.align 2
LDR r0, =double_battle_check_hijack|1
BX r0
.pool

 .org 0x0804C064 ; disable DMA for box RAM
.align 2
.byte 00 
.byte 21
.pool

.org 0x0806D5F6 ; hijack step routine to check showsprites
.thumb
.align 2
LDR r0, =check_showsprite_every_step_hijack|1
BX r0
MOV r0, r0
.pool


.org 0x088000A4 ;debug, remove this if you see it
.thumb
.align 2
.word 0x88303a1
.pool

.org freespace
.thumb
.align 2
mod:
	SWI 0x6
    MOV r0, r1
    BX LR

.align 2
.importobj "./build/linked.o"
    
.align 2

; ------------------------- CUSTOM APOPLYMOVEMENT FOR APPROACH -------------------------

custom_movement_hijack:
    ;replaced code:
    LSL     R0, R0, #0x1C
    LSR     R0, R0, #0x1C
    
    ;check if npc script starts with 0x5C (trainerbattle)
    ;r4 here contains a pointer to the trainer npc entry in npc_states (starts at 0x02036E38)
    PUSH    {r0-r4}
    LDRB    r0, [r4, #0x8] ;npc_local_id
    LDRB    r1, [r4, #0x9] ;npc_local_mapno
    LDRB    r2, [r4, #0xA] ;npc_local_mapbankÃ¬
    LDR r3, =0x0805FC20|1 ;npc_script_by_local_id_and_map
    BL goto_r3
    LDRB    r0, [r0] ; load first byte of script
    MOV     r1, #0x5C ;byte indicating a trainerbattle
    CMP     r0, r1
    BEQ     original_routine
    
modified_routine:
    POP     {r0-r4}
    BL      copy_of_08063F84
    B       return_to_original_exec

original_routine:
    POP     {r0-r4}
    LDR     r1, = 0x08063F84|1 ; original routine
    BL      goto_r1
    
return_to_original_exec:
    ;continue r1 is not used after this
    LDR     R1, =0x08082000|1
    BX      r1

copy_of_08063F84:
    PUSH    {R4, R5, LR}
    SUB     SP, SP, #8
    LSL     R0, R0, #0x18
    LSR     R4, R0, #0x18
    LDR     R1, =custom_movement ;src
    MOV     R0, SP          ; dst
    MOV     R2, #5          ; size
    LDR     R5, =0x081E5E78|1 ;memcpy
    BL      goto_r5
    CMP     R4, #4
    BLS     copy_of_08063F84_continue
    MOV     R4, #0
copy_of_08063F84_continue:
    MOV     R1, SP
    ADD     R0, R1, R4
    LDRB    R0, [R0]
    ADD     SP, SP, #8
    POP     {R4, R5}
    POP     {R1}
    BX      R1
    



; ---------------------- HIJACK DOUBLE BATTLE CHECK TO CUSTOMIZE WHAT HAPPENS AFTER ENGAGEMENT -------------------------------

double_battle_check_hijack:
    PUSH    {r0-r1}
    LDRB    r0, [r6]
    MOV     r1, #0x5C
    CMP     r0, r1
    BEQ     double_battle_check_continue ;if it's a trainerbattle, continue as normal
    POP     {r0-r1}
    B        double_battle_check_return_skip

double_battle_check_continue:
    POP     {r0-r1}
    LDRB    R0, [R6,#1] ; load NPC script
    CMP     R0, #4
    BNE     double_battle_check_return_normal
    LDR        r1, =0x08040CC4|1
    BL        goto_r1
    LDR        r1, =0x08081BBA|1
    BX        r1

double_battle_check_return_normal:
    LDR        r0, =0x08081BC0|1
    BX        r0

double_battle_check_return_skip:
    mov r0, r6
    add r0, #0xB ;first 5 bytes are 'callasm', the 5 after are 'goto 0x0202D4B4', the one after is a padding 0xFF
    MOV r1, r5 ;call main with encounter table and npcId, this will put the script into script_slot
    BL load_wildbattle_script
    
    ;taken from 08080356 in order to correctly setup script environment
    PUSH {r0-r4}
    LSL    R0, R5, #0x18
    LSR    R0, R0, #0x18
    LDR     R2, =0x03005074 ; update scripting_npc
    STRB    R0, [R2]       
    LDR     R4, =0x020370D2 ;var_800F
    LDR     R3, =0x02036E38 ;npc_states
    LSL    R2, R0, #3
    ADD    R2, R2, R0
    LSL    R2, R2, #2
    ADD    R2, R2, R3
    LDRB    R0, [R2,#8] ; npc_state.local_id
    STRH    R0, [R4]

    LDR     R0, =custom_engagement_script ; orginally 0x081A4EB4
    LDR        r1, =0x08069AE4|1
    BL      goto_r1
    LDR        r1, =0x08069940|1
    BL      goto_r1
    POP {r0-r4}
    LDR     r0, =0x08081BC8|1
    BX        r0

.align 4
custom_engagement_script:
    .byte 0x69 ; lockall
    .byte 0x25
    .byte 0x38
    .byte 0x00 ; special 0x38
    .byte 0x25
    .byte 0x37
    .byte 0x00 ; special 0x37
    .byte 0x27 ; waitstate
    .byte 0x05 ; goto
    .word 0x0202D4B4





; ---------------------- CHECK SHOWSPRITE EVERY STEP -------------------------------

.align 4
.thumb
check_showsprite_every_step_hijack:
    PUSH {r0-r5}
    LDR r3, =check_showsprite_every_step|1
    BL goto_r3

END:
    POP {r0-r5}
    /*Instructions from 0x806D5F6*/
    LSL r0, r0, #0x18
    LSR r0, r0, #0x18
    CMP r0, #0x1
    BEQ BRANCHER
    MOV r0, r5
    MOV r1, r4
    LDR r3, =0x0806DA11
    BL goto_r3
    LSL r0, r0, #0x18
    LSR r0, r0, #0x18
    CMP r0, #0x1
    BEQ BRANCHER
    MOV r0, r4
    LDR r0, =0x0806D611
    BX r0

BRANCHER:
    LDR r0, =0x0806D651
    BX r0


.align 4
.thumb

goto_r1:
    BX r1;
    
goto_r3:
    BX r3
    
goto_r5:
    BX r5;
    
empty_text:
    .byte 0xFF

custom_movement:
    .byte 0x1D
    .byte 0x1D
    .byte 0x1E
    .byte 0x1F
    .byte 0x20
    
.pool

.close
