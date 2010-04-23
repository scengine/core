/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2010  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 -----------------------------------------------------------------------------*/

/* created: 25/08/2009
   updated: 25/08/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEParticleEmitter.h"

void SCE_ParticleEmit_Init (SCE_SParticleEmitter *emit)
{
    emit->node = NULL;
    SCE_Vector3_Set (emit->velocity, 0.0f, 0.0f, 0.0f);
    emit->auto_vel = SCE_FALSE;
    emit->min_life = 1.0f;
    emit->max_life = 1.0f;
    SCE_List_InitIt (&emit->it);
    SCE_List_SetData (&emit->it, emit);
}
SCE_SParticleEmitter* SCE_ParticleEmit_Create (void)
{
    SCE_SParticleEmitter *emit = NULL;
    if (!(emit = SCE_malloc (sizeof *emit)))
        SCEE_LogSrc ();
    else
        SCE_ParticleEmit_Init (emit);
    return emit;
}
void SCE_ParticleEmit_Delete (SCE_SParticleEmitter *emit)
{
    if (emit) {
        SCE_List_Remove (&emit->it);
        SCE_free (emit);
    }
}

SCE_SListIterator* SCE_ParticleEmit_GetIterator (SCE_SParticleEmitter *emit)
{
    return &emit->it;
}

void SCE_ParticleEmit_SetNode (SCE_SParticleEmitter *emit, SCE_SNode *node)
{
    emit->node = node;
}
SCE_SNode* SCE_ParticleEmit_GetNode (SCE_SParticleEmitter *emit)
{
    return emit->node;
}

float* SCE_ParticleEmit_GetVelocity (SCE_SParticleEmitter *emit)
{
    return emit->velocity;
}
void SCE_ParticleEmit_GetVelocityv (SCE_SParticleEmitter *emit, SCE_TVector3 v)
{
    SCE_Vector3_Copy (v, emit->velocity);
}

void SCE_ParticleEmit_SetMinLife (SCE_SParticleEmitter *emit, float life)
{
    emit->min_life = life;
}
float SCE_ParticleEmit_GetMinLife (SCE_SParticleEmitter *emit)
{
    return emit->min_life;
}

void SCE_ParticleEmit_SetMaxLife (SCE_SParticleEmitter *emit, float life)
{
    emit->max_life = life;
}
float SCE_ParticleEmit_GetMaxLife (SCE_SParticleEmitter *emit)
{
    return emit->max_life;
}
