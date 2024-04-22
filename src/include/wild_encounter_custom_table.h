#ifndef WILD_ENC_CUST_TBL_H
#define WILD_ENC_CUST_TBL_H
#include <pokeagb/pokeagb.h>

struct Encounter{
	u16 species;
	u8 lvl;
    u8 filler;
};

struct Wild_enocunter_tbl{
	u16 entriesNo;
	struct Encounter encounters[];
};
#endif