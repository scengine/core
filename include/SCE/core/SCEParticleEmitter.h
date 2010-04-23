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

/* created: 24/08/2009
   updated: 25/08/2009 */

#ifndef SCEPARTICLEEMITTER_H
#define SCEPARTICLEEMITTER_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCENode.h"

#ifdef __cplusplus
extern "C" {
#endif

/* an emitter has rules to create particles: base location and velocity, etc. */

typedef struct sce_sparticleemitter SCE_SParticleEmitter;
struct sce_sparticleemitter {
    SCE_SNode *node;
    SCE_TVector3 velocity;      /* can be given by the user or computed */
    int auto_vel;               /* is \c velocity computed or given? */
    float min_life, max_life;
    /* should be in "mod" module rather than here */
#if 0
    int give_velocity;          /* give the emitter's velocity to the created
                                   particles */
    int attached;               /* are particles attached to \c node ? */
#endif
    SCE_SListIterator it;
};

void SCE_ParticleEmit_Init (SCE_SParticleEmitter*);
SCE_SParticleEmitter* SCE_ParticleEmit_Create (void);
void SCE_ParticleEmit_Delete (SCE_SParticleEmitter*);

SCE_SListIterator* SCE_ParticleEmit_GetIterator (SCE_SParticleEmitter*);

void SCE_ParticleEmit_SetNode (SCE_SParticleEmitter*, SCE_SNode*);
SCE_SNode* SCE_ParticleEmit_GetNode (SCE_SParticleEmitter*);

float* SCE_ParticleEmit_GetVelocity (SCE_SParticleEmitter*);
void SCE_ParticleEmit_GetVelocityv (SCE_SParticleEmitter*, SCE_TVector3);

void SCE_ParticleEmit_SetMinLife (SCE_SParticleEmitter*, float);
float SCE_ParticleEmit_GetMinLife (SCE_SParticleEmitter*);

void SCE_ParticleEmit_SetMaxLife (SCE_SParticleEmitter*, float);
float SCE_ParticleEmit_GetMaxLife (SCE_SParticleEmitter*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */
