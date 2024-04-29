#ifndef CONFIG_H
#define CONFIG_H

#include <pokeagb/pokeagb.h>

#define NPC_CTDWN_ADDR       0x0202DE14 //0x28 free bytes in RAM, 0x0202DE14 is actually box #8
#define STEPS_BEFORE_RESPAWN 10         //After this amount of steps the NPCs will respawn
#define TARGET_OAM_ID        110 //110 on ow editor

//                           NORMAL, FIGHTING, FLYING, POISON, GROUND, ROCK, BUG, GHOST, STEEL, NONE, FIRE, WATER, GRASS, ELECTRIC, PSYCHIC, ICE, DRAGON, DARK  
const u8 OAM_ID_BY_TYPE[] = {110   , 111     , 112   , 113   , 114   , 115 , 116, 117  , 118  , 119 , 120 , 121  , 122  , 123     , 124    , 125, 126   , 127};

#endif