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
#include "SCE/core/SCEParticleProcessor.h"

void SCE_ParticleProc_Init (SCE_SParticleProcessor *proc)
{
    proc->emitter = NULL;
    SCE_List_Init (&proc->modifiers);
    SCE_List_CanDeleteIterators (&proc->modifiers, SCE_TRUE);
}
SCE_SParticleProcessor* SCE_ParticleProc_Create (void)
{
    SCE_SParticleProcessor *proc = NULL;
    if (!(proc = SCE_malloc (sizeof *proc)))
        SCEE_LogSrc ();
    else
        SCE_ParticleProc_Init (proc);
    return proc;
}
void SCE_ParticleProc_Delete (SCE_SParticleProcessor *proc)
{
    if (proc) {
        SCE_ParticleProc_RemoveModifiers (proc);
        SCE_free (proc);
    }
}

static void SCE_ParticleProc_EmitCallback (SCE_SParticle *part, float sec,
                                           void *p)
{
    SCE_SListIterator *it;
    SCE_SParticleProcessor *proc = p;
    (void)p;
    SCE_List_ForEach (it, &proc->modifiers) {
        SCE_SParticleModifier *mod = SCE_List_GetData (it);
        SCE_ParticleMod_Emit (mod, proc->emitter, part);
    }
}
static void SCE_ParticleProc_UpdateCallback (SCE_SParticle *part, float sec,
                                             void *p)
{
    SCE_SListIterator *it;
    SCE_SParticleProcessor *proc = p;
    SCE_List_ForEach (it, &proc->modifiers) {
        SCE_SParticleModifier *mod = SCE_List_GetData (it);
        SCE_ParticleMod_Apply (mod, part, sec);
    }
}

void SCE_ParticleProc_SetToBuffer (SCE_SParticleProcessor *proc,
                                   SCE_SParticleBuffer *pb)
{
    SCE_Particle_SetInitParticlesCallback (pb, SCE_ParticleProc_EmitCallback,
                                           proc);
    SCE_Particle_SetUpdateParticlesCallback (pb,SCE_ParticleProc_UpdateCallback,
                                             proc);
}

void SCE_ParticleProc_SetEmitter (SCE_SParticleProcessor *proc,
                                  SCE_SParticleEmitter *emitter)
{
    proc->emitter = emitter;
}
SCE_SParticleEmitter* SCE_ParticleProc_GetEmitter (SCE_SParticleProcessor *proc)
{
    return proc->emitter;
}

int SCE_ParticleProc_AddModifier (SCE_SParticleProcessor *proc,
                                  SCE_SParticleModifier *mod)
{
    if (SCE_List_AppendNewl (&proc->modifiers, mod) < 0) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    return SCE_OK;
}
void SCE_ParticleProc_RemoveModifiers (SCE_SParticleProcessor *proc)
{
    SCE_List_Clear (&proc->modifiers);
}
static void SCE_ParticleProc_FreeModifier (void *m)
{
    SCE_ParticleMod_Delete (m);
}
void SCE_ParticleProc_DeleteModifiers (SCE_SParticleProcessor *proc)
{
    SCE_List_SetFreeFunc (&proc->modifiers, SCE_ParticleProc_FreeModifier);
    SCE_List_Clear (&proc->modifiers);
    SCE_List_SetFreeFunc (&proc->modifiers, NULL);
}
