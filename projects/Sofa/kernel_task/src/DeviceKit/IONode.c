/*
 * This file is part of the Sofa project
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "IONode.h"
#include <EISAID.h>


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