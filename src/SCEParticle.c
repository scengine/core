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

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEParticle.h"

/**
 * \ingroup interface
 * @{
 */

/**
 * \brief Initializes a particle structure
 */
void SCE_Particle_Init (SCE_SParticle *part)
{
    SCE_Vector3_Set (part->position, 0.0f, 0.0f, 0.0f);
    SCE_Vector3_Set (part->velocity, 0.0f, 0.0f, 0.0f);
    part->age = 0.0f;
}

void SCE_Particle_InitBuffer (SCE_SParticleBuffer *pb)
{
    pb->vertices = NULL;
    pb->v_stride = 0;
    pb->vpp = 1;
    pb->array = NULL;
    pb->geom = NULL;
    pb->prim = SCE_POINTS;
    pb->particles = NULL;
    pb->n_particles = pb->max_particles = 0;
    pb->size = 0;
    pb->p_stride = sizeof (SCE_SParticle);
    pb->update = NULL;
    pb->init = NULL;
    pb->update_p = pb->update_array_p = pb->init_p = pb->init_array_p = NULL;
    pb->update_array = NULL;
    pb->init_array = NULL;
    pb->box = NULL;
    pb->update_aabb = SCE_FALSE;
}
/**
 * \brief 
 */
SCE_SParticleBuffer* SCE_Particle_CreateBuffer (void)
{
    SCE_SParticleBuffer *pb = NULL;
    if (!(pb = SCE_malloc (sizeof *pb)))
        SCEE_LogSrc ();
    else
        SCE_Particle_InitBuffer (pb);
    return pb;
}
/**
 * \brief 
 */
void SCE_Particle_DeleteBuffer (SCE_SParticleBuffer *pb)
{
    if (pb) {
        SCE_free (pb->particles);
        SCE_free (pb);
    }
}

/**
 * \brief Gets the pointer to the additionnal part allocated for a particle
 * \sa SCE_SParticleBuffer::size
 */
void* SCE_Particle_GetData (SCE_SParticle *part)
{
    return &part[1];
}

/**
 * \brief Sets the size of the additionnal part allocated for a particle
 *
 * Advice: call this function before allocate the particles into a
 * particle buffer (see SCE_Particle_SetMaxParticles()).
 * \sa SCE_Particle_GetData(), SCE_Particle_SetMaxParticles()
 */
void SCE_Particle_SetAdditionnalDataSize (SCE_SParticleBuffer *pb, size_t size)
{
    pb->size = size;
    pb->p_stride = size + sizeof (SCE_SParticle);
}
/**
 * \brief Defines the maximum capacity of a particle buffer
 * \param n maximum number of particles
 *
 * This function allocates memory to store the particles into a buffer.
 * It frees the previous pointer returned by SCE_Particle_GetParticles().
 * The number of used particles is unchanged unless \p n is lesser than it,
 * this case the number of used particles is set to \p n.
 * \sa SCE_Particle_GetParticles(), SCE_Particle_GetMaxParticles(),
 * SCE_Particle_GetNumParticles()
 */
int SCE_Particle_SetMaxParticles (SCE_SParticleBuffer *pb, size_t n)
{
    SCE_free (pb->particles);
    pb->particles = NULL;
    if (n > 0) {
        if (!(pb->particles = SCE_malloc (n * pb->p_stride))) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }
    pb->max_particles = n;
    if (n < pb->n_particles)
        pb->n_particles = n;
    return SCE_OK;
}
/**
 * \brief 
 */
size_t SCE_Particle_GetNumParticles (SCE_SParticleBuffer *pb)
{
    return pb->n_particles;
}
/**
 * \brief 
 */
size_t SCE_Particle_GetMaxParticles (SCE_SParticleBuffer *pb)
{
    return pb->max_particles;
}
/**
 * \brief 
 */
size_t SCE_Particle_GetAvailableParticles (SCE_SParticleBuffer *pb)
{
    return pb->max_particles - pb->n_particles;
}

/**
 * \brief 
 */
void SCE_Particle_SetInitArraysCallback (SCE_SParticleBuffer *pb,
                                         SCE_FParticleArrayFunc f, void *p)
{
    pb->init_array = f;
    pb->init_array_p = p;
}
/**
 * \brief 
 */
void SCE_Particle_SetInitParticlesCallback (SCE_SParticleBuffer *pb,
                                            SCE_FParticleFunc f, void *p)
{
    pb->init = f;
    pb->init_p = p;
}

/**
 * \brief Gets the pointer to the particles of a buffer
 * \sa SCE_Particle_AddParticles(), SCE_Particle_SetMaxParticles()
 */
SCE_SParticle* SCE_Particle_GetParticles (SCE_SParticleBuffer *pb)
{
    return (SCE_SParticle*)pb->particles;
}
/**
 * \brief Increase the number of active particles by \p n
 * \returns the offset of the first added particle
 * 
 * Note that you have to check if you have exceed the maximum buffer capacity.
 * \sa SCE_Particle_GetMaxParticles(), SCE_Particle_SetMaxParticles(),
 * SCE_Particle_GetParticles(), SCE_Particle_RemoveParticle()
 */
size_t SCE_Particle_AddParticles (SCE_SParticleBuffer *pb, size_t n)
{
    size_t offset, i;
    offset = SCE_Particle_GetAvailableParticles (pb);
    n = MIN (n, offset);
    i = offset = pb->n_particles;
    pb->n_particles += n;
    /* init content */
    for (; i < pb->n_particles; i++) {
        SCE_Particle_Init ((SCE_SParticle*)&pb->particles[i * pb->p_stride]);
        pb->init ((SCE_SParticle*)&pb->particles[i * pb->p_stride], 0.0f,
                  pb->init_p);
        pb->init_array ((SCE_SParticle*)&pb->particles[i * pb->p_stride],
                        &pb->vertices[i * pb->v_stride], pb->init_array_p);
    }
    return offset;
}
/* copies particle b into a, abstract and GL data */
static void SCE_Particle_CopyParticle (SCE_SParticleBuffer *pb,
                                       size_t a, size_t b)
{
    memcpy (&pb->vertices[a * pb->v_stride], &pb->vertices[b * pb->v_stride],
            pb->v_stride);
    memcpy (&pb->particles[a * pb->p_stride], &pb->particles[b * pb->p_stride],
            pb->p_stride);
}
/**
 * \brief Removes the particle number \p n
 * \sa SCE_Particle_AddParticles(), SCE_Particle_GetAvailableParticles()
 */
void SCE_Particle_RemoveParticle (SCE_SParticleBuffer *pb, size_t n)
{
    pb->n_particles--;
    SCE_Particle_CopyParticle (pb, n, pb->n_particles);
}
void SCE_Particle_RemoveLesserThan (SCE_SParticleBuffer *pb, float age)
{
    size_t i;
    for (i = 0; i < pb->n_particles; i++) {
        SCE_SParticle *part = (SCE_SParticle*)&pb->particles[i * pb->p_stride];
        if (part->age < age) {
            SCE_Particle_RemoveParticle (pb, i);
            i--;
        }
    }
}
void SCE_Particle_RemoveGreaterThan (SCE_SParticleBuffer *pb, float age)
{
    size_t i;
    for (i = 0; i < pb->n_particles; i++) {
        SCE_SParticle *part = (SCE_SParticle*)&pb->particles[i * pb->p_stride];
        if (part->age > age) {
            SCE_Particle_RemoveParticle (pb, i);
            i--;
        }
    }
}
/**
 * \brief Removes the dead particles (lesser or equal to \p sec)
 * \sa SCE_Particle_RemoveLesserThan(), SCE_Particle_RemoveGreaterThan()
 */
void SCE_Particle_RemoveDead (SCE_SParticleBuffer *pb, float sec)
{
    SCE_Particle_RemoveLesserThan (pb, sec);
}

/**
 * \brief Builds the geometry array of a particle buffer
 * \returns SCE_ERROR on error, SCE_OK otherwise
 *
 * The first value into \p attribs is the number of vertices per particle.
 * Then, the structure of \p attribs parameter is, for each attribute you
 * want for particle vertices:
 * attribute type, data type, size.
 * \sa SCE_Particle_GetArray()
 */
int SCE_Particle_BuildArrays (SCE_SParticleBuffer *pb, SCE_EPrimitiveType prim,
                              const int *attribs, size_t n_attrib)
{
    size_t i, vpp, stride;
    const int *a;

    vpp = attribs[0];
    a = &attribs[1];

    for (i = 0; i < n_attrib; i++) {
        pb->v_stride += a[2] * SCE_Type_Sizeof (a[1]);
        a = &a[3];
    }
    pb->v_stride *= vpp;

    pb->array = NULL; /* the previous one will be removed by its geometry */
    /* create and initialize vertices array */
    if (!(pb->vertices = SCE_malloc (pb->max_particles * pb->v_stride)))
        goto fail;
    {
        SCE_SParticle model;
        SCE_Particle_Init (&model);
        for (i = 0; i < pb->max_particles; i++) {
            pb->init_array (&model, &pb->vertices[i * pb->v_stride],
                            pb->init_array_p);
        }
    }

    stride = 0;
    a = &attribs[1];
    for (i = 0; i < n_attrib; i++) {
        SCE_SGeometryArray *array = NULL;
        if (!(array = SCE_Geometry_CreateArray ()))
            goto fail;
        SCE_Geometry_SetArrayData (array, a[0], a[1], 0, a[2],
                                   &pb->vertices[stride], SCE_FALSE);
        stride += a[2] * SCE_Type_Sizeof (a[1]);
        if (!pb->array)
            pb->array = array;
        else
            SCE_Geometry_AttachArray (pb->array, array);
        a = &a[3];
    }

    pb->vpp = vpp;
    pb->prim = prim;

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}
SCE_SGeometryArray* SCE_Particle_GetArray (SCE_SParticleBuffer *pb)
{
    return pb->array;
}
static void SCE_Particle_UpdateGeometry (SCE_SParticleBuffer *pb)
{
    SCE_Geometry_SetNumVertices (pb->geom, pb->n_particles * pb->vpp);
    SCE_Geometry_Modified (pb->array, NULL);
}
/**
 * \brief Builds a geometry from a particle buffer
 *
 * To work fine, this function requires that the geometry array of the particle
 * buffer to be already built by SCE_Particle_BuildArrays().
 * \sa SCE_Particle_BuildArrays()
 */
void SCE_Particle_BuildGeometry (SCE_SParticleBuffer *pb, SCE_SGeometry *geom)
{
    SCE_Geometry_SetPrimitiveType (geom, pb->prim);
    SCE_Geometry_AddArrayRec (geom, pb->array);
    pb->geom = geom;
    SCE_Particle_UpdateGeometry (pb);
    /* set max number, because the geometry will probably be used to
       construct a mesh just right now */
    SCE_Geometry_SetNumVertices (geom, pb->max_particles * pb->vpp);
}
/**
 * \brief Creates a new geometry and call SCE_Particle_BuildGeometry() on it
 * \returns a new geometry of which you should manage the memory (ie. call
 * SCE_Geometry_Delete() on it or will never happen)
 * \sa SCE_Particle_BuildGeometry(), SCE_Particle_BuildArrays()
 */
SCE_SGeometry* SCE_Particle_CreateGeometry (SCE_SParticleBuffer *pb)
{
    SCE_SGeometry *geom = NULL;
    if (!(geom = SCE_Geometry_Create ())) {
        SCEE_LogSrc ();
        return NULL;
    }
    SCE_Particle_BuildGeometry (pb, geom);
    return geom;
}

/**
 * \brief 
 */
void SCE_Particle_SetUpdateArraysCallback (SCE_SParticleBuffer *pb,
                                           SCE_FParticleArrayFunc f, void *p)
{
    pb->update_array = f;
    pb->update_array_p = p;
}
/**
 * \brief 
 */
void SCE_Particle_SetUpdateParticlesCallback (SCE_SParticleBuffer *pb,
                                              SCE_FParticleFunc f, void *p)
{
    pb->update = f;
    pb->update_p = p;
}

/**
 * \brief Enables/disables automatic update of the box of a particle buffer
 * when updating it (using SCE_Particle_UpdateParticles())
 * \sa SCE_Particle_SetBox()
 */
void SCE_Particle_ActivateBoxUpdate (SCE_SParticleBuffer *pb, int a)
{
    pb->update_aabb = a;
}
/**
 * \brief Sets the bounding box where write out
 *
 * By default a particle buffer doesn't have a box, so enabling
 * automatic bounding box update requires that you specify a box.
 * \sa SCE_Particle_ActivateBoxUpdate(), SCE_Particle_GetBox()
 */
void SCE_Particle_SetBox (SCE_SParticleBuffer *pb, SCE_SBox *box)
{
    pb->box = box;
    if (!box)
        pb->update_aabb = SCE_FALSE;
}
/**
 * \brief Gets the bounding box of a particle buffer set with
 * SCE_Particle_SetBox()
 * \sa SCE_Particle_SetBox()
 */
SCE_SBox* SCE_Particle_GetBox (SCE_SParticleBuffer *pb)
{
    return pb->box;
}

static void SCE_Particle_GetMinMaxPoints (SCE_TVector3 minp, SCE_TVector3 maxp,
                                          const SCE_SParticle *p)
{
    SCE_Vector3_GetMin (minp, minp, p->position);
    SCE_Vector3_GetMax (maxp, maxp, p->position);
}
/**
 * \brief Updates the geometry array using the user-defined callback
 * \param sec elapsed seconds
 * \sa SCE_Particle_SetUpdateParticlesCallback(),
 * SCE_Particle_UpdateArrays()
 */
void SCE_Particle_UpdateParticles (SCE_SParticleBuffer *pb, float sec)
{
    size_t i;
    if (!pb->update_aabb) {
        for (i = 0; i < pb->n_particles; i++) {
            pb->update ((SCE_SParticle*)&pb->particles[i * pb->p_stride],
                        sec, pb->update_p);
        }
    } else {
        SCE_TVector3 minp = {0.0f, 0.0f, 0.0f};
        SCE_TVector3 maxp = {0.0f, 0.0f, 0.0f};
        for (i = 0; i < pb->n_particles; i++) {
            pb->update ((SCE_SParticle*)&pb->particles[i * pb->p_stride],
                        sec, pb->update_p);
            SCE_Particle_GetMinMaxPoints
                (minp, maxp, (SCE_SParticle*)&pb->particles[i * pb->p_stride]);
        }
        SCE_Box_SetFromMinMax (pb->box, minp, maxp);
    }
}
/**
 * \brief Updates the geometry array using the user-defined callback
 * \sa SCE_Particle_SetUpdateArraysCallback(),
 * SCE_Particle_UpdateParticles()
 */
void SCE_Particle_UpdateArrays (SCE_SParticleBuffer *pb)
{
    size_t i;
    for (i = 0; i < pb->n_particles; i++) {
        pb->update_array ((SCE_SParticle*)&pb->particles[i * pb->p_stride],
                          &pb->vertices[i * pb->v_stride],
                          pb->update_array_p);
    }
    SCE_Particle_UpdateGeometry (pb);
}

/** @} */
