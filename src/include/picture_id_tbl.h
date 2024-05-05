#ifndef PICTURE_ID_TBL_H
#define PICTURE_ID_TBL_H

#include <pokeagb/pokeagb.h>

struct PictureData{
    u16 tilesTag;
    u16 pal_num;
    u16 pal_num2;
    u16 len; //of what?
    u16 width;
    u16 height;
    u8 pal_slot;
    u8 footprint;
    u8 unused;
    void *distribution; //IDK
    void *sizedraw; //IDK
    void *animation; //IDK
    void *spritetable;
    void *ramstore; //IDK
};

//0x083A3BB0
extern struct PictureData game_pic_data_tbl[150]; //this is probably more than 150, but this works for now

#endif