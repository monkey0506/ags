#ifndef __CROOM_MOVE_H
#define __CROOM_MOVE_H

#ifndef ROOMEDIT
#define MAXNEEDSTAGES 40
struct MoveList {
    int   pos[MAXNEEDSTAGES];
    int   numstage;
    fixed xpermove[MAXNEEDSTAGES], ypermove[MAXNEEDSTAGES];
    int   fromx, fromy;
    int   onstage, onpart;
    int   lastx, lasty;
    char  doneflag;
    char  direct;  // MoveCharDirect was used or not
};
#endif


#endif // __CROOM_MOVE_H