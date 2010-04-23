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

/* created: 23/08/2009
   updated: 25/08/2009 */

#ifndef SCEPARTICLESYSTEM_H
#define SCEPARTICLESYSTEM_H

#include "SCE/core/SCEBoundingBox.h"
#include "SCE/core/SCEBoundingSphere.h"
#include "SCE/core/SCEParticle.h"
#include "SCE/core/SCEParticleEmitter.h"
#include "SCE/core/SCEParticleProcessor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_sparticlesystem SCE_SParticleSystem;
struct sce_sparticlesystem {
    SCE_SBoundingBox *box;      /* bounding volumes */
    SCE_SBoundingSphere *sphere;

    SCE_SParticleProcessor proc;
    SCE_SParticleBuffer buf;
    SCE_SParticleEmitter emitter;
    float n_particles;          /* number of particles */
    float pps;                  /* emitted particles per second */
};

void SCE_ParticleSys_Init (SCE_SParticleSystem*);
SCE_SParticleSystem* SCE_ParticleSys_Create (void);
void SCE_ParticleSys_Delete (SCE_SParticleSystem*);

void SCE_ParticleSys_SetBoundingBox (SCE_SParticleSystem*, SCE_SBoundingBox*);
SCE_SBoundingBox* SCE_ParticleSys_GetBoundingBox (SCE_SParticleSystem*);
void SCE_ParticleSys_SetEmissionFrequency (SCE_SParticleSystem*, float);

void SCE_ParticleSys_Build (SCE_SParticleSystem*);

void SCE_ParticleSys_Update (SCE_SParticleSystem*, float);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */
