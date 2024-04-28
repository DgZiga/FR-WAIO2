#include <pokeagb/pokeagb.h>
#include "config.h"
#include "include/agb_debug/debug.c"
#include "include/wild_encounter_custom_table.h"
#include "include/npc_show_countdown.h"

POKEAGB_EXTERN void flag_clear(u16 flag);
extern u32 mod(u32 x, u32 y);
extern u16 person_id_by_npc_id_and_map_and_bank(u8 npc_id, u8 map, u8 bank);
void load_script(u16 person_id, struct Encounter selected_encounter, u8 npcId);
void load_wildbattle_script(struct Wild_enocunter_tbl *encounters, u8 npcId);


void callasm_entrypoint(){
    u8 *current_ptr_script = script_env_1.pointer_script;
    struct Wild_enocunter_tbl *encounters_tbl = (struct Wild_enocunter_tbl *)(current_ptr_script+6); //skip 5 bytes: (GOTO) and filler 0xF
    u8 id = 0xFF;
    for (u8 i = 0; i < sizeof(npc_states)/ sizeof(npc_states[0]); i++) {
        if(npc_states[i].local_id == var_800F){
            id = i;
        }
    }
    if(id == 0xFF){
        return;
    }
    load_wildbattle_script(encounters_tbl, id);
}

void load_wildbattle_script(struct Wild_enocunter_tbl *encounters, u8 npcId) {
    struct NpcState npc = *(npc_states+npcId);
    u16 person_id = person_id_by_npc_id_and_map_and_bank(npc.local_id, npc.local_map_number, npc.local_map_bank);

    struct Encounter *selected_encounter;
    //Choose a random encounter
    u8 selected_pkmn = mod(rand(), encounters->entriesNo);
    selected_encounter = &((encounters->encounters)[selected_pkmn]);

    
	struct Npc_show_countdown *npc_show_ctdwn = (struct Npc_show_countdown *)NPC_CTDWN_ADDR;
    //Look for a free spot to add the npc countdown
    for(u8 i = 0; i<10; i++){
        if(npc_show_ctdwn[i].person_id != 0){
            continue;
        }
        npc_show_ctdwn[i] = (struct Npc_show_countdown) {person_id, STEPS_BEFORE_RESPAWN-1}; //Clearflag shows the sprite on the step AFTER it's executed, so we insert x-1 so that clearflag will be executed on the x-1 frame and at frame x the sprite will be displayed 
        break;
    }
    
    //Load hidesprite and wildbattle script into memory and exec it
    void *script_slot = (void *)0x0202D4B4; //TODO refactor
	void *script_slot_iterable = script_slot;
    
    *((u8 *)script_slot_iterable) = 0x29; //setflag 
    script_slot_iterable++;
    *((u16 *)script_slot_iterable) = person_id; 
    script_slot_iterable+= 2;
    *((u8 *)script_slot_iterable) = 0x53; //hidesprite 
    script_slot_iterable++;
    *((u16 *)script_slot_iterable) = npc.local_id;  
    script_slot_iterable += 2;
    *((u8 *)script_slot_iterable) = 0xB6; //wildbattle 
    script_slot_iterable++;
    *((u16 *)script_slot_iterable) = selected_encounter->species;  //wildbattle species
    script_slot_iterable+= 2;
    *((u8 *)script_slot_iterable) = selected_encounter->lvl; //wildbattle lvl
    script_slot_iterable++;
    *((u16*)script_slot_iterable) = 0x0000; //wildbattle item
    script_slot_iterable+= 2;
    *((u8 *)script_slot_iterable) = 0xB7; //dowildbattle
    script_slot_iterable++;
    *((u8 *)script_slot_iterable) = 0x02; //end
    
}

//To be executed at every step (see /src/main.s), checks if any npc had its countdown expire and, should that be the case, execs a clearflag, which will show NPC on next step
void check_showsprite_every_step(){
	struct Npc_show_countdown *npc_show_ctdwn = (struct Npc_show_countdown *)NPC_CTDWN_ADDR;
	for(u8 i = 0; i<10; i++){	//For every countdown position
		if(npc_show_ctdwn[i].person_id == 0){ //Avoid empty countdown positions
			continue;
		}
		npc_show_ctdwn[i].steps_no--;	//Decrease the number of steps needed
		
		if(npc_show_ctdwn[i].steps_no == 0){	

            struct MapHeader *mapHeader = mapheader_by_mapnumbers(npc_states[0].local_map_bank, npc_states[0].local_map_number);
            struct RomNpc* romNpcs = mapHeader->events->npcs;
            u8 romNpcsNo = mapHeader->events->num_npcs;
            u8 id = 0xFF;
            for (u8 j = 0; j < romNpcsNo; j++) {
                if (romNpcs[j].local_id == npc_show_ctdwn[i].person_id) {
                    id = j;
                }
            }
            if(id == 0xFF){
                flag_clear(npc_show_ctdwn[i].person_id); 
                npc_show_ctdwn[i].person_id = 0;
                continue;
            }

            struct RomNpc romNpc = romNpcs[id];
            u16 npcX = (romNpc.x) +7; //+7 because coords are translated once in game (black padding is added around the map)
            u16 npcY = (romNpc.y) +7; //+7 because coords are translated once in game (black padding is added around the map)
            u16 playerX = npc_states[0].to.x;
            u16 playerY = npc_states[0].to.y;
            u16 xDelta = (playerX > npcX ? (playerX-npcX) : (npcX-playerX));
            u16 yDelta = (playerY > npcY ? (playerY-npcY) : (npcY-playerY));

            //dprintf("npcX : 0x%x, npcY: 0x%x, playerX: 0x%x, playerY: 0x%x, romNpcs: 0x%x, id: 0x%x\n", npcX, npcY, playerX, playerY, romNpcs, id);

            if(xDelta >= 8 || yDelta >= 6){ //out of sight: from my research, 8 tiles away on the horizontal axis and 6 tiles away on the vertical axis is the minimum delta required for the npc to be out of screen
                flag_clear(npc_show_ctdwn[i].person_id); 
                npc_show_ctdwn[i].person_id = 0;
            } else {
        		npc_show_ctdwn[i].steps_no++;
            }
		} 
	}
}
