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

#ifndef SCEPARTICLEMODIFIER_H
#define SCEPARTICLEMODIFIER_H

#include "SCE/core/SCEParticle.h"
#include "SCE/core/SCEParticleEmitter.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SCE_FParticleModifierEmitFunc)(SCE_SParticle*,
                                              SCE_SParticleEmitter*, void*);
typedef void (*SCE_FParticleModifierFunc)(SCE_SParticle*, float, void*);

typedef struct sce_sparticlemodifier SCE_SParticleModifier;
struct sce_sparticlemodifier {
    SCE_FParticleModifierEmitFunc emit;
    SCE_FParticleModifierFunc update;
    size_t size;                /* size of \c data */
    void *data;                 /* additionnal data */
};

void SCE_ParticleMod_Init (SCE_SParticleModifier*);
SCE_SParticleModifier* SCE_ParticleMod_Create (void*, size_t);
void SCE_ParticleMod_Delete (SCE_SParticleModifier*);

void SCE_ParticleMod_SetData (SCE_SParticleModifier*, void*);
void SCE_ParticleMod_SetEmitCallback (SCE_SParticleModifier*,
                                      SCE_FParticleModifierEmitFunc);
void SCE_ParticleMod_SetUpdateCallback (SCE_SParticleModifier*,
                                        SCE_FParticleModifierFunc);

void SCE_ParticleMod_Emit (SCE_SParticleModifier*, SCE_SParticleEmitter*,
                           SCE_SParticle*);
void SCE_ParticleMod_Apply (SCE_SParticleModifier*, SCE_SParticle*, float);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */
