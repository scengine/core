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

#ifndef SCEPARTICLE_H
#define SCEPARTICLE_H

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sce_sparticle SCE_SParticle;
struct sce_sparticle {
    SCE_TVector3 position;
    SCE_TVector3 velocity;
    float age;                  /* mass.. ? */
};

typedef void (*SCE_FParticleFunc)(SCE_SParticle*, float, void*);
typedef void (*SCE_FParticleArrayFunc)(const SCE_SParticle*, void*, void*);

typedef struct sce_sparticlebuffer SCE_SParticleBuffer;
struct sce_sparticlebuffer {
    /* renderable part */
    char *vertices;
    size_t v_stride;
    size_t vpp;                 /* vertices per particle */
    SCE_SGeometryArray *array;  /* root array */
    SCE_SGeometry *geom;        /* associated geometry */
    SCE_EPrimitiveType prim;

    /* abstract part */
    char *particles;            /* all particles */
    size_t n_particles;         /* used particles */
    size_t max_particles;       /* capacity of \c particles */
    size_t size;                /* size of additionnal part allocated for a
                                   particle */
    size_t p_stride;            /* stride between two consecutive particles */

    SCE_FParticleFunc update, init;
    void *update_p, *init_p;
    SCE_FParticleArrayFunc update_array, init_array;
    void *update_array_p, *init_array_p;

    SCE_SBox *box;
    int update_aabb;
};

void SCE_Particle_Init (SCE_SParticle*);

void SCE_Particle_InitBuffer (SCE_SParticleBuffer*);
SCE_SParticleBuffer* SCE_Particle_CreateBuffer (void);
void SCE_Particle_DeleteBuffer (SCE_SParticleBuffer*);

void* SCE_Particle_GetData (SCE_SParticle*);
void SCE_Particle_SetAdditionnalDataSize (SCE_SParticleBuffer*, size_t);

int SCE_Particle_SetMaxParticles (SCE_SParticleBuffer*, size_t);
size_t SCE_Particle_GetNumParticles (SCE_SParticleBuffer*);
size_t SCE_Particle_GetMaxParticles (SCE_SParticleBuffer*);
size_t SCE_Particle_GetAvailableParticles (SCE_SParticleBuffer*);

void SCE_Particle_SetInitArraysCallback (SCE_SParticleBuffer*,
                                         SCE_FParticleArrayFunc, void*);
void SCE_Particle_SetInitParticlesCallback (SCE_SParticleBuffer*,
                                            SCE_FParticleFunc, void*);

SCE_SParticle* SCE_Particle_GetParticles (SCE_SParticleBuffer*);
size_t SCE_Particle_AddParticles (SCE_SParticleBuffer*, size_t);
void SCE_Particle_RemoveParticle (SCE_SParticleBuffer*, size_t);
void SCE_Particle_RemoveLesserThan (SCE_SParticleBuffer*, float);
void SCE_Particle_RemoveGreaterThan (SCE_SParticleBuffer*, float);
void SCE_Particle_RemoveDead (SCE_SParticleBuffer*, float);

int SCE_Particle_BuildArrays (SCE_SParticleBuffer*, SCE_EPrimitiveType,
                              const int*, size_t);
SCE_SGeometryArray* SCE_Particle_GetArray (SCE_SParticleBuffer*);
void SCE_Particle_BuildGeometry (SCE_SParticleBuffer*, SCE_SGeometry*);
SCE_SGeometry* SCE_Particle_CreateGeometry (SCE_SParticleBuffer*);

void SCE_Particle_SetUpdateArraysCallback (SCE_SParticleBuffer*,
                                           SCE_FParticleArrayFunc, void*);
void SCE_Particle_SetUpdateParticlesCallback (SCE_SParticleBuffer*,
                                              SCE_FParticleFunc, void*);

void SCE_Particle_ActivateBoxUpdate (SCE_SParticleBuffer*, int);
void SCE_Particle_SetBox (SCE_SParticleBuffer*, SCE_SBox*);
SCE_SBox* SCE_Particle_GetBox (SCE_SParticleBuffer*);

void SCE_Particle_UpdateParticles (SCE_SParticleBuffer*, float);
void SCE_Particle_UpdateArrays (SCE_SParticleBuffer*);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */
