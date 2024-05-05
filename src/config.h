#ifndef CONFIG_H
#define CONFIG_H

#include <pokeagb/pokeagb.h>

#define NPC_CTDWN_ADDR       0x0202DE14 //0x28 free bytes in RAM, 0x0202DE14 is actually box #8
#define STEPS_BEFORE_RESPAWN 10         //After this amount of steps the NPCs will respawn
#define TARGET_OAM_ID        110 

//                           NORMAL, FIGHTING, FLYING, POISON, GROUND, ROCK, BUG, GHOST, STEEL, NONE, FIRE, WATER, GRASS, ELECTRIC, PSYCHIC, ICE, DRAGON, DARK  
const u8 OAM_ID_BY_TYPE[] = {110   , 111     , 112   , 113   , 114   , 115 , 116, 117  , 118  , 119 , 120 , 121  , 122  , 123     , 124    , 125, 126   , 127};


/*
const unsigned short SharedPal[16] __attribute__((aligned(4)))=
{
	0x5134,0x56B5,0x18C6,0x7FFF,0x11A0,0x055A,0x7C20,0x001F,
	0x0000,0x20C0,0x7331,0x2FE0,0x5E64,0x046B,0x2407,0x069D,

    
    0x5134, // #a349a4
    0x7FFF, // #f8f8f8
    0x56B5, // 
    0x18C6, //fine normali
    0x7331,
    0x5e64,
    0x20c0, //fine blu
    0x069D,
    0x055A,
    0x046B //fine rosse


};*/
#endif