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
   updated: 26/08/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEParticleModifier.h"

/* default callbacks */
static void SCE_ParticleMod_EmitCallback (SCE_SParticle *part,
                                          SCE_SParticleEmitter *emit, void *p)
{
    float *mat;                 /* TODO: float* for matrix is bad */
    (void)p;
    mat = SCE_Node_GetFinalMatrix (SCE_ParticleEmit_GetNode (emit));
    SCE_Matrix4_GetTranslation (mat, part->position);
    SCE_Vector3_Copy (part->velocity, SCE_ParticleEmit_GetVelocity (emit));
    part->velocity[1] += 10.0f;
    part->age = SCE_ParticleEmit_GetMinLife (emit);
}
static void SCE_ParticleMod_UpdateCallback (SCE_SParticle *part, float sec,
                                            void *p)
{
    (void)p;
    SCE_Vector3_Operator2 (part->position, +=, part->velocity, *, sec);
    part->age -= sec;
}

void SCE_ParticleMod_Init (SCE_SParticleModifier *mod)
{
    mod->emit = SCE_ParticleMod_EmitCallback;
    mod->update = SCE_ParticleMod_UpdateCallback;
    mod->size = 0;
    mod->data = &mod[1];
}
SCE_SParticleModifier* SCE_ParticleMod_Create (void *data, size_t size)
{
    SCE_SParticleModifier *mod = NULL;
    if (!(mod = SCE_malloc (sizeof *mod + size)))
        SCEE_LogSrc ();
    else {
        SCE_ParticleMod_Init (mod);
        mod->size = size;
        SCE_ParticleMod_SetData (mod, data);
    }
    return mod;
}
void SCE_ParticleMod_Delete (SCE_SParticleModifier *mod)
{
    if (mod) {
        SCE_free (mod);
    }
}

void SCE_ParticleMod_SetData (SCE_SParticleModifier *mod, void *data)
{
    if (mod->size)
        memcpy (mod->data, data, mod->size);
    else
        mod->data = data;
}
void SCE_ParticleMod_SetEmitCallback (SCE_SParticleModifier *mod,
                                      SCE_FParticleModifierEmitFunc f)
{
    mod->emit = f;
}
void SCE_ParticleMod_SetUpdateCallback (SCE_SParticleModifier *mod,
                                        SCE_FParticleModifierFunc f)
{
    mod->update = f;
}

void SCE_ParticleMod_Emit (SCE_SParticleModifier *mod, 
                           SCE_SParticleEmitter *emit, SCE_SParticle *part)
{
    mod->emit (part, emit, mod->data);
}
void SCE_ParticleMod_Apply (SCE_SParticleModifier *mod, SCE_SParticle *part,
                            float sec)
{
    mod->update (part, sec, mod->data);
}
