#include <pokeagb/pokeagb.h>
#include "config.h"
#include "include/agb_debug/debug.c"
#include "include/wild_encounter_custom_table.h"
#include "include/npc_show_countdown.h"

//extern struct Wild_enocunter_tbl encounter_tbl;
extern u32 mod(u32 x, u32 y);
extern u16 person_id_by_npc_id_and_map_and_bank(u8 npc_id, u8 map, u8 bank);
void load_script(u16 person_id, struct Encounter selected_encounter, u8 npcId);

void load_wildbattle_script(u8 npcId) {
	u16 steps = 10;
	
	struct NpcState npc = *(npc_states+npcId);			    		//Retrieve the NPC_STATE of the NPC the player is about to fight
	
	struct Wild_enocunter_tbl *encounter_tbl_ptr = (struct Wild_enocunter_tbl *)ENC_TBL_ADDR; //Start of encounter_tbl in ROM
	struct Npc_show_countdown *npc_show_ctdwn = (struct Npc_show_countdown *)NPC_CTDWN_ADDR; 

    //Lookup wild encounter tbl
    u16 person_id = person_id_by_npc_id_and_map_and_bank(npc.local_id, npc.local_map_number, npc.local_map_bank);

    //struct Wild_enocunter_tbl *encounter_tbl_ptr = &encounter_tbl; //For some reason this adds +1 to the address?
    while(encounter_tbl_ptr->person_id != person_id){
        encounter_tbl_ptr++;
    }
    
    struct Encounter selected_encounter;
    do{	//Choose a random encounter
        u8 selected_pkmn = mod(rand(), 4);
        dprintf("generated: 0x%x\n", selected_pkmn);
        selected_encounter = encounter_tbl_ptr->encounters[selected_pkmn];
    }while (selected_encounter.species == 0xFFFF);
    /*
    //Look for a free spot to add the npc countdown
    for(u8 i = 0; i<10; i++){
        if(npc_show_ctdwn[i].person_id != 0){
            continue;
        }
        npc_show_ctdwn[i] = (struct Npc_show_countdown) {person_id, steps};
        break;
    }*/
    
    //Load hidesprite and wildbattle script into memory and exec it
    load_script(person_id, selected_encounter, npcId);
}
void load_script(u16 person_id, struct Encounter selected_encounter, u8 npcId){
    void *script_slot = (void *)0x0202D4B4; //TODO refactor
	void *script_slot_iterable = script_slot;
	struct NpcState npc = *(npc_states+npcId); //Retrieve the NPC_STATE of the NPC the player is about to fight

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
    *((u16 *)script_slot_iterable) = selected_encounter.species; 
    script_slot_iterable+= 2;
    *((u8 *)script_slot_iterable) = selected_encounter.lvl; 
    script_slot_iterable++;
    *((u16*)script_slot_iterable) = 0x0000; 
    script_slot_iterable+= 2;
    *((u8 *)script_slot_iterable) = 0xB7; //dowildbattle
    script_slot_iterable++;
    *((u8 *)script_slot_iterable) = 0x02; //end
    
}
/*
//To be executed at every step (see /src/hooker.s and /hooks), checks if any npc had its countdown expire and, should that be the case, execs a showsprite
void check_showsprite_every_step(){
	struct Npc_show_countdown *npc_show_ctdwn = (struct Npc_show_countdown *)NPC_CTDWN_ADDR;
	void *precompiled_script_slot = (void *)PREC_SCRIPT_SLOT2;		
	void *script_slot_iterable = precompiled_script_slot;
	struct Script_state *env1 = (struct Script_state *)0x03000EB0;
	
	for(u8 i = 0; i<10; i++){	//For every countdown position
		if(npc_show_ctdwn[i].person_id == 0){ //Avoid empty countdown positions
			continue;
		}
		npc_show_ctdwn[i].steps_no--;	//Decrease the number of steps needed
		
		//Clearflag shows the sprite on the step AFTER it's executed, so execute it on the third-to-last step, so that on the second-to-last, the sprite will show itself and on the last step it will activate
		if(npc_show_ctdwn[i].steps_no == 2){	
		
			*((u8 *)script_slot_iterable) = 0x2A; //clearflag
			script_slot_iterable++;
			*((u16 *)script_slot_iterable) = npc_show_ctdwn[i].person_id; 
			script_slot_iterable += 2;
			*((u8 *)script_slot_iterable) = 0x02; //end
			
			env1->next_offset = (u32)precompiled_script_slot;
			env1->continue_exec = 1;
			script_exec_env_1(env1);
		
		} 
		if(npc_show_ctdwn[i].steps_no == 0){	//Remove the personId from the countdown section, so that it can battle again
			npc_show_ctdwn[i].person_id = 0;
		}
	}
}

//To be called by a callasm by every npc that is a wild pokémon, in case the player actively interacts with them
void wildbattle_on_a_press(){
	
	u16 steps = 10;
	u16 *current_npc  = (u16 *)0x03005074;
	u8  *current_bank  = (u8  *)0x02025530;
	u8  *current_map  = (u8  *)0x02025531;	
	struct Npc_state *npc_state = (struct Npc_state *)0x02036E38;	//Start of NPC_STATE
	struct Npc_state *npc = npc_state + *current_npc;

	struct Wild_enocunter_tbl *encounter_tbl = (struct Wild_enocunter_tbl *)ENC_TBL_ADDR; //Start of encounter_tbl in ROM
	struct Npc_show_countdown *npc_show_ctdwn = (struct Npc_show_countdown *)NPC_CTDWN_ADDR; 
	
	void *precompiled_script_slot = (void *)PREC_SCRIPT_SLOT;		
	void *script_slot_iterable = precompiled_script_slot;
	
	struct Script_state *env1 = (struct Script_state *)0x03000EB0;
	
	//Lookup wild encounter tbl
	u16 person_id = person_id_by_npc_id_and_map_and_bank(npc->local_id, *current_map, *current_bank);
	while(encounter_tbl->person_id != person_id){
		encounter_tbl++;
	}
	
	struct Encounter selected_encounter;
	do{	//Choose a random encounter
		u8 selected_pkmn = mod(rand(), 4);
		selected_encounter = encounter_tbl->encounters[selected_pkmn];
	}while (selected_encounter.species == 0xFFFF);
	
	//Look for a free spot to add the npc countdown
	for(u8 i = 0; i<10; i++){
		if(npc_show_ctdwn[i].person_id != 0){
			continue;
		}
		npc_show_ctdwn[i] = (struct Npc_show_countdown) {person_id, steps};
		break;
	}
	
	//Load "hidesprite" and wildbattle script into memory and exec it
	
	*((u8 *)script_slot_iterable) = 0x29; //setflag
	script_slot_iterable++;
	*((u16 *)script_slot_iterable) = person_id; 
	script_slot_iterable+= 2;
	*((u8 *)script_slot_iterable) = 0xB6; //wildbattle
	script_slot_iterable++;
	*((u16 *)script_slot_iterable) = selected_encounter.species; 
	script_slot_iterable+= 2;
	*((u8 *)script_slot_iterable) = selected_encounter.lvl; 
	script_slot_iterable++;
	*((u16 *)script_slot_iterable) = 0x0000; 
	script_slot_iterable+= 2;
	*((u8 *)script_slot_iterable) = 0x53; //hidesprite
	script_slot_iterable++;
	*((u16 *)script_slot_iterable) = npc->local_id; 
	script_slot_iterable += 2;
	*((u8 *)script_slot_iterable) = 0xB7; //dowildbattle
	script_slot_iterable++;
	*((u8 *)script_slot_iterable) = 0x02; //end
	
	env1->next_offset = (u32)precompiled_script_slot;
	env1->continue_exec = 1;
	script_exec_env_1(env1);
}*/