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
#include "ProcessList.h"
#include "utils.h"
#include "NameServer.h"


void ThreadCleanupTimer(Thread* t)
{
    KernelTaskContext* env = getKernelTaskContext();
    vka_t *mainVKA = getMainVKA();
    tm_deregister_cb(&env->tm, t->_base.timerID);
    tm_free_id(&env->tm, t->_base.timerID);
    cnode_delete(mainVKA, t->_base.replyCap);
    t->_base.replyCap = 0;
}

void ThreadBaseAddServiceClient(ThreadBase*t, ServiceClient* client)
{
    LL_APPEND(t->_clients, client);
}