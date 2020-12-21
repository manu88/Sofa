#include "IONode.h"

int IONodeGetEISAID(const IONode*n, char eisaID[9])
{
    if(n->hid.type == IOVariantType_STRING)
    {
        strncpy(eisaID, n->hid.value.s, 8);
        return 0;
    }
    else if(n->hid.type == IOVariantType_UINT64 && isEisaId(n->hid.value.v))
    {
        getEisaidString(n->hid.value.v, eisaID);
        return 0;
    }
    return -1;
}

int IONodeEISAIDIs(const IONode*n, const char* eisaID)
{
    char eisaID_[9] = "";
    if(IONodeGetEISAID(n, eisaID_) == 0)
    {
        if(strcmp(eisaID_, eisaID) == 0)
        {
            return 0;
        }
    }
    return -1;
}