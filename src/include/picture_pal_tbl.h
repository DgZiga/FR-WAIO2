#ifndef PICTURE_PAL_TBL_H
#define PICTURE_PAL_TBL_H

#include <pokeagb/pokeagb.h>

struct PicturePalData{
    u16 *pal; //array of 16 u16
    u16 tag;
    u16 filler_maybe;
};

#endif