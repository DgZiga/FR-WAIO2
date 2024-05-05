#ifndef NPC_TINT_CALCULATOR_C
#define NPC_TINT_CALCULATOR_C

#include "include/npc_tint_calculator.h"

void npc_tint_asm_entrypoint(struct RomNpc* romNpcLoadedAddress){
    if (romNpcLoadedAddress->type_nr == TARGET_OAM_ID){
        u8 *script_addr = romNpcLoadedAddress->script;
        struct Wild_enocunter_tbl *tbl_addr = (struct Wild_enocunter_tbl *)(script_addr+11); //skip callasm, goto, and a 0xFF filler

        u8 *type_cnt = malloc(TYPE_MAX);
        memset(type_cnt, 0x0, TYPE_MAX);

        //count how many instances of a type there are in this encounter table
        for(u8 i=0; i<tbl_addr->entriesNo; i++){
            struct PokemonBaseStat species = pokemon_base_stats[tbl_addr->encounters[i].species];
            enum PokemonType type1 = species.type[0];
            enum PokemonType type2 = species.type[1];
            type_cnt[type1]++;
            if(type1 != type2){
                type_cnt[type2]++;
            }
        }

        //We could do something smart here, but for now just take the highest occourring type
        u8 maxI = 0;
        u8 maxV = 0;
        for (u8 i = 0; i < TYPE_MAX; i++) {
            if(type_cnt[i] > maxV){
                maxV = type_cnt[i];
                maxI = i;
            }
        }
        
        u8 oam_id = OAM_ID_BY_TYPE[maxI];

        free(type_cnt);
        ///3A5158 palette degli ow
        //3A3BB0 sprites table
       // struct PictureData asd = game_pic_data_tbl[0];

        romNpcLoadedAddress->type_nr = oam_id; //Override picture number
    }
}



#endif