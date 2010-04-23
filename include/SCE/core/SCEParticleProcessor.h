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

#ifndef SCEPARTICLEPROCESSOR_H
#define SCEPARTICLEPROCESSOR_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEParticle.h"
#include "SCE/core/SCEParticleEmitter.h"
#include "SCE/core/SCEParticleModifier.h"

#ifdef __cplusplus
extern "C" {
#endif

/* defines particles movement, updates abstract data providing callbacks */

typedef struct sce_sparticleprocessor SCE_SParticleProcessor;
struct sce_sparticleprocessor {
    SCE_SParticleEmitter *emitter;
    SCE_SList modifiers;
};

void SCE_ParticleProc_Init (SCE_SParticleProcessor*);
SCE_SParticleProcessor* SCE_ParticleProc_Create (void);
void SCE_ParticleProc_Delete (SCE_SParticleProcessor*);

void SCE_ParticleProc_SetToBuffer (SCE_SParticleProcessor*,
                                   SCE_SParticleBuffer*);

void SCE_ParticleProc_SetEmitter (SCE_SParticleProcessor*,
                                  SCE_SParticleEmitter*);
SCE_SParticleEmitter* SCE_ParticleProc_GetEmitter (SCE_SParticleProcessor*);

int SCE_ParticleProc_AddModifier (SCE_SParticleProcessor*,
                                  SCE_SParticleModifier*);
void SCE_ParticleProc_RemoveModifiers (SCE_SParticleProcessor*);
void SCE_ParticleProc_DeleteModifiers (SCE_SParticleProcessor*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */
