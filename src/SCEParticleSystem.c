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
   updated: 26/08/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEParticleSystem.h"

void SCE_ParticleSys_Init (SCE_SParticleSystem *psys)
{
    psys->box = NULL;
    psys->sphere = NULL;
    SCE_ParticleProc_Init (&psys->proc);
    SCE_Particle_InitBuffer (&psys->buf);
    SCE_ParticleEmit_Init (&psys->emitter);
    psys->n_particles = 0.0f;
    psys->pps = 10.0f;
}
SCE_SParticleSystem* SCE_ParticleSys_Create (void)
{
    SCE_SParticleSystem *psys = NULL;
    if (!(psys = SCE_malloc (sizeof *psys)))
        SCEE_LogSrc ();
    else
        SCE_ParticleSys_Init (psys);
    return psys;
}
void SCE_ParticleSys_Delete (SCE_SParticleSystem *psys)
{
    if (psys) {
        SCE_free (psys);
    }
}


/**
 * \brief Sets the bounding box of a particle system
 * \sa SCE_ParticleSys_GetBoundingbox()
 */
void SCE_ParticleSys_SetBoundingBox (SCE_SParticleSystem *psys,
                                     SCE_SBoundingBox *box)
{
    psys->box = box;
    if (box) {
        SCE_Particle_ActivateBoxUpdate (&psys->buf, SCE_TRUE);
        SCE_Particle_SetBox (&psys->buf, SCE_BoundingBox_GetBox (box));
    } else {
        SCE_Particle_ActivateBoxUpdate (&psys->buf, SCE_FALSE);
        SCE_Particle_SetBox (&psys->buf, NULL);
    }
}
/**
 * \brief Gets the bounding box of a particle system
 * \sa SCE_ParticleSys_SetBoundingbox()
 */
SCE_SBoundingBox* SCE_ParticleSys_GetBoundingBox (SCE_SParticleSystem *psys)
{
    return psys->box;
}

/**
 * \brief Sets emission frequency of a particle system, ie emitted particles
 * per second
 */
void SCE_ParticleSys_SetEmissionFrequency (SCE_SParticleSystem *psys, float f)
{
    psys->pps = f;
}

/**
 * \brief Prepares a particle system for use it
 */
void SCE_ParticleSys_Build (SCE_SParticleSystem *psys)
{
    SCE_ParticleProc_SetToBuffer (&psys->proc, &psys->buf);
    SCE_ParticleProc_SetEmitter (&psys->proc, &psys->emitter);
}

/**
 * \brief Updates a particle system
 * \param sec elapsed seconds
 */
void SCE_ParticleSys_Update (SCE_SParticleSystem *psys, float sec)
{
    size_t n;

    /* remove diying particles */
    SCE_Particle_RemoveDead (&psys->buf, sec);

    /* create new particles from emitters */
    psys->n_particles += psys->pps * sec;
    n = psys->n_particles + 0.5f;
    psys->n_particles -= n;
    SCE_Particle_AddParticles (&psys->buf, n);
    /*SCE_ParticleEmit_GetNumGenerated(&psys->emitter));*/

    /* update particles */
    SCE_Particle_UpdateParticles (&psys->buf, sec);

    /* update renderable buffer */
    SCE_Particle_UpdateArrays (&psys->buf);

    /* done! */
}
