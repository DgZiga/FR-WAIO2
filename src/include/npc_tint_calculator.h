#ifndef NPC_TINT_CALCULATOR_H
#define NPC_TINT_CALCULATOR_H

/*somewhen (load map?) 

08054F68 copy_rom_npcs_to_ram is called when loading map, we should hijack here


R00=083b9708 R04=000008e0 R08=00000000 R12=08470d6c
R01=02025e88 R05=00000000 R09=00000000 R13=03007dd4
R02=083b9708 R06=00000000 R10=03005008 R14=0805592b
R03=00000000 R07=00000000 R11=00000000 R15=0805506c
CPSR=0000003f (......T Mode: 1f)
08055068  1c10 add r0, r2, #0x0
> 0805506a  c864 ldmia r0!, {r2,r5,r6}
0805506c  c164 stmia r1!, {r2,r5,r6}
debugger> c

at 0805506A we copy rom npc in ram, we can modify here



we scan the map npcs, if they have callasm 0x??????? goto 0x202D4B4 they are a custom npc
if they are a custom npc, personalize their picture number
*/
#include <pokeagb/pokeagb.h>
#include "../config.h"
#include "agb_debug/debug.c"
#include "wild_encounter_custom_table.h"

#include "picture_id_tbl.h"
#include "picture_pal_tbl.h"

void npc_tint_asm_entrypoint(struct RomNpc* romNpcLoadedAddress);

#endif