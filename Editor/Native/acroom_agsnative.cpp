//
// Implementation from acroom.cpp specific to AGS.Native library
//

#include "util/wgt2allg.h"
#include "ac/roomstruct.h"

//=============================================================================
// AGS.Native-specific implementation split out of acroom.cpp
//=============================================================================

bool load_room_is_version_bad(RoomStruct *rstruc)
{
    return ((rstruc->wasversion < kRoomVersion_3404) || (rstruc->wasversion > kRoomVersion_Current));
}
