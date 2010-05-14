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

/* created: 06/04/2009
   updated: 04/06/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEAnimatedGeometry.h"

/**
 * \defgroup animgeom Animated geometry
 * \ingroup interface
 * \internal
 * \brief Skeletal animated geometry
 * @{
 */

#define SCE_ANIMGEOM_ENABLE_QUAT_TRANSFORM 0

static int is_init = SCE_FALSE;
static int resource_type = 0;

/**
 * \internal
 * \brief Initializes the animated geometry manager
 */
int SCE_Init_AnimGeom (void)
{
    if (is_init)
        return SCE_OK;
    resource_type = SCE_Resource_RegisterType (SCE_TRUE, NULL, NULL);
    if (resource_type < 0)
        goto fail;
    is_init = SCE_TRUE;
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    SCEE_LogSrcMsg ("failed to initialize animated geometry module");
    return SCE_ERROR;
}
void SCE_Quit_AnimGeom (void)
{
    is_init = SCE_FALSE;
}


int SCE_AnimGeom_GetResourceType (void)
{
    return resource_type;
}


static size_t SCE_AnimGeom_GetVerticesID (SCE_EVertexAttribute attrib)
{
    size_t id = 0;
    switch (attrib)
    {
    /*case SCE_POSITION: id = 0; break;*/
    case SCE_NORMAL:   id = 1; break;
    case SCE_TANGENT:  id = 2; break;
    case SCE_BINORMAL: id = 3;
    }
    return id;
}
static SCE_EVertexAttribute SCE_AnimGeom_GetVerticesAttrib (size_t id)
{
    SCE_EVertexAttribute attribs[] = {SCE_POSITION, SCE_NORMAL,
                                          SCE_TANGENT,  SCE_BINORMAL};
    return attribs[id];
}


static void SCE_AnimGeom_InitVertex (SCE_SVertex *vert)
{
    vert->weight_id = 0;
    vert->weight_count = 0;
}
static void SCE_AnimGeom_InitWeight (SCE_SVertexWeight *weight)
{
    weight->weight = 0.0;
    weight->joint_id = 0;
    weight->next_vertex_id = 0;
}

static void SCE_AnimGeom_ApplySkeletonP (SCE_SAnimatedGeometry*, SCE_SSkeleton*);

/**
 * \internal
 * \brief Initializes an animated geometry structure
 */
static void SCE_AnimGeom_Init (SCE_SAnimatedGeometry *ageom)
{
    size_t i;
    ageom->geom = NULL;
    ageom->animskel = ageom->baseskel = NULL;
    ageom->canfree_animskel = ageom->canfree_baseskel =
        ageom->canfree_indices = SCE_FALSE;
    ageom->vertices = NULL;
    ageom->weights = NULL;
    ageom->indices = NULL;
    ageom->n_vertices = 0;
    ageom->n_weights = 0;
    ageom->n_indices = 0;
    for (i = 0; i < SCE_MAX_ANIMATED_VERTEX_ATTRIBUTES; i++) {
        ageom->base[i] = NULL;
        ageom->local[i] = SCE_FALSE;
        ageom->output[i] = NULL;
        ageom->arrays[i] = NULL;
    }
    ageom->applyskel = SCE_AnimGeom_ApplySkeletonP;
}
/**
 * \brief Creates a new animated geometry
 */
SCE_SAnimatedGeometry* SCE_AnimGeom_Create (void)
{
    SCE_SAnimatedGeometry *ageom = NULL;
    if (!(ageom = SCE_malloc (sizeof *ageom)))
        SCEE_LogSrc ();
    else {
        SCE_AnimGeom_Init (ageom);
        if (!(ageom->geom = SCE_Geometry_Create ())) {
            SCE_free (ageom), ageom = NULL;
            SCEE_LogSrc ();
        }
    }
    return ageom;
}
/**
 * \brief Deletes an animated geometry
 */
void SCE_AnimGeom_Delete (SCE_SAnimatedGeometry *ageom)
{
    if (ageom) {
        size_t i;
        SCE_Geometry_Delete (ageom->geom);
        for (i = 0; i < SCE_MAX_ANIMATED_VERTEX_ATTRIBUTES; i++)
            SCE_free (ageom->base[i]);
        SCE_free (ageom->indices);
        if (ageom->canfree_baseskel)
            SCE_Skeleton_Delete (ageom->baseskel);
        if (ageom->canfree_animskel)
            SCE_Skeleton_Delete (ageom->animskel);
        SCE_free (ageom);
    }
}

/**
 * \brief Gets the geometry of an animated geometry
 * \sa SCE_AnimGeom_BuildGeometry()
 */
SCE_SGeometry* SCE_AnimGeom_GetGeometry (SCE_SAnimatedGeometry *ageom)
{
    return ageom->geom;
}

/**
 * \brief Sets the bind pose skeleton of an animated geometry
 */
void SCE_AnimGeom_SetBaseSkeleton (SCE_SAnimatedGeometry *ageom,
                                   SCE_SSkeleton *skel, int canfree)
{
    if (ageom->canfree_baseskel)
        SCE_Skeleton_Delete (ageom->baseskel);
    ageom->baseskel = skel;
    ageom->canfree_baseskel = canfree;
}
/**
 * \brief Gets the bind pose skeleton of an animated geometry
 */
SCE_SSkeleton* SCE_AnimGeom_GetBaseSkeleton (SCE_SAnimatedGeometry *ageom)
{
    return ageom->baseskel;
}
/**
 * \brief Sets the animtation skeleton of an animated geometry
 */
void SCE_AnimGeom_SetAnimSkeleton (SCE_SAnimatedGeometry *ageom,
                                   SCE_SSkeleton *skel, int canfree)
{
    if (ageom->canfree_animskel)
        SCE_Skeleton_Delete (ageom->animskel);
    ageom->animskel = skel;
    ageom->canfree_animskel = canfree;
}
/**
 * \brief Gets the animation skeleton of an animated geometry
 */
SCE_SSkeleton* SCE_AnimGeom_GetAnimSkeleton (SCE_SAnimatedGeometry *ageom)
{
    return ageom->animskel;
}
/**
 * \brief Creates and sets a new skeleton as the animation skeleton
 * \sa SCE_AnimGeom_GetAnimSkeleton(), SCE_AnimGeom_SetAnimSkeleton()
 */
int SCE_AnimGeom_AllocateAnimSkeleton (SCE_SAnimatedGeometry *ageom)
{
    SCE_SSkeleton *skel = NULL;
    size_t n;

#ifdef SCE_DEBUG
    if (!ageom->baseskel) {
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("this animated has not bind pose skeleton: animation "
                     "skeleton can't be created");
        return SCE_ERROR;
    }
#endif
    if (!(skel = SCE_Skeleton_Create ()))
        goto fail;
    n = SCE_Skeleton_GetNumJoints (ageom->baseskel);
    if (SCE_Skeleton_AllocateJoints (skel, n) < 0)
        goto fail;
    SCE_AnimGeom_SetAnimSkeleton (ageom, skel, SCE_TRUE);
    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


/**
 * \brief Allocates memory for vertices
 * \returns SCE_ERROR on error, SCE_OK otherwise
 * \sa SCE_AnimGeom_AllocateWeights()
 */
int SCE_AnimGeom_AllocateVertices (SCE_SAnimatedGeometry *ageom, size_t n)
{
    size_t i;
    SCE_free (ageom->vertices);
    if (!(ageom->vertices = SCE_malloc (n * sizeof *ageom->vertices))) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    ageom->n_vertices = n;
    for (i = 0; i < n; i++)
        SCE_AnimGeom_InitVertex (&ageom->vertices[i]);
    return SCE_OK;
}
/**
 * \brief Allocates memory for vertex weights
 * \returns SCE_ERROR on error, SCE_OK otherwise
 * \sa SCE_AnimGeom_SetWeights(), SCE_AnimGeom_AllocateVertices()
 */
int SCE_AnimGeom_AllocateWeights (SCE_SAnimatedGeometry *ageom, size_t n)
{
    size_t i;
    SCE_free (ageom->weights);
    if (!(ageom->weights = SCE_malloc (n * sizeof *ageom->weights))) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    ageom->n_weights = n;
    for (i = 0; i < n; i++)
        SCE_AnimGeom_InitWeight (&ageom->weights[i]);
    return SCE_OK;
}
/**
 * \brief Allocates memory for the base vertices (4D vectors)
 * \param attrib vertex attribute of the vectors
 * \param local are the vectors in local position from their joints?
 * \returns SCE_ERROR on error, SCE_OK otherwise
 */
int SCE_AnimGeom_AllocateBaseVertices (SCE_SAnimatedGeometry *ageom,
                                       SCE_EVertexAttribute attrib,
                                       int local)
{
    float *data = NULL;
    size_t i, n, id;
    if (local)
        n = ageom->n_weights;
    else
        n = ageom->n_vertices;
#ifdef SCE_DEBUG
    if (ageom->n_vertices == 0) {
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("you must call SCE_AnimGeom_AllocateVertices() "
                     "before calling this function");
        return SCE_ERROR;
    } else if (n == 0) {
        SCEE_Log (SCE_INVALID_OPERATION);
        SCEE_LogMsg ("you must call SCE_AnimGeom_AllocateWeights() "
                     "before calling this function");
        return SCE_ERROR;
    }
#endif
    id = SCE_AnimGeom_GetVerticesID (attrib);
    if (!(data = SCE_malloc (n * 4 * sizeof *data))) {
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    ageom->output[id] = SCE_malloc (ageom->n_vertices * 3 * sizeof *data);
    if (!ageom->output[id]) {
        SCE_free (data);
        SCEE_LogSrc ();
        return SCE_ERROR;
    }
    for (i = 0; i < n; i++) {
        data[i]     = ageom->output[id][i]     = 0.0f;
        data[i + 1] = ageom->output[id][i + 1] = 0.0f;
        data[i + 2] = ageom->output[id][i + 2] = 0.0f;
        data[i + 3] = 0.0f;
    }
    SCE_free (ageom->base[id]);
    ageom->base[id] = data;
    ageom->local[id] = local;
    return SCE_OK;
}

/**
 * \brief Gets the vertices pointer of an animated geometry
 * \sa SCE_AnimGeom_GetWeights()
 */
SCE_SVertex* SCE_AnimGeom_GetVertices (SCE_SAnimatedGeometry *ageom)
{
    return ageom->vertices;
}
/**
 * \brief Gets the vertices pointer of an animated geometry
 * \sa SCE_AnimGeom_GetVertices()
 */
SCE_SVertexWeight* SCE_AnimGeom_GetWeights (SCE_SAnimatedGeometry *ageom)
{
    return ageom->weights;
}
/**
 * \brief Gets the base vertices data of an animated geometry
 * \param attrib the vertex attribute of the queried data
 */
SCEvertices* SCE_AnimGeom_GetBaseVertices (SCE_SAnimatedGeometry *ageom,
                                           int attrib)
{
    unsigned int id = SCE_AnimGeom_GetVerticesID (attrib);
    return ageom->base[id];
}

/**
 * \brief Sets the vertices indices of an animated geometry
 * \sa SCE_AnimGeom_SetVertices(), SCE_Mesh_SetIndices()
 */
void SCE_AnimGeom_SetIndices (SCE_SAnimatedGeometry *ageom, size_t n,
                              SCEindices *indices, int canfree)
{
    if (ageom->canfree_indices)
        SCE_free (ageom->indices);
    ageom->indices = indices;
    ageom->n_indices = n;
}



void MulMatScalar (SCE_TMatrix4x3 m, float s, SCE_TMatrix4x3 n)
{
    n[0] = s * m[0];
    n[1] = s * m[1];
    n[2] = s * m[2];
    n[3] = s * m[3];
    n[4] = s * m[4];
    n[5] = s * m[5];
    n[6] = s * m[6];
    n[7] = s * m[7];
    n[8] = s * m[8];
    n[9] = s * m[9];
    n[10] = s * m[10];
    n[11] = s * m[11];
}
void MadMatScalar (SCE_TMatrix4x3 m, float s, SCE_TMatrix4x3 n)
{
    n[0] += s * m[0];
    n[1] += s * m[1];
    n[2] += s * m[2];
    n[3] += s * m[3];
    n[4] += s * m[4];
    n[5] += s * m[5];
    n[6] += s * m[6];
    n[7] += s * m[7];
    n[8] += s * m[8];
    n[9] += s * m[9];
    n[10] += s * m[10];
    n[11] += s * m[11];
}

#if 0
static void SCE_AnimGeom_ApplySkeleton (SCE_SAnimatedGeometry *ageom,
                                        unsigned int n,
                                        SCE_SSkeleton *skel)
{
    unsigned int i, j;

    for (i = 0; i < ageom->n_vertices; i++) {
        SCE_TMatrix4x3 mat;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        weight = &ageom->weights[vert->weight_id];
        MulMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                      mat);
        for (j = 1; j < vert->weight_count; j++) {
            weight = &ageom->weights[j + vert->weight_id];
            MadMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                          mat);
        }

        SCE_Matrix4x3_MulV4 (mat, &ageom->base[n][i * 4],
                             &ageom->output[n][i * ageom->size[0]]);
    }
}
#endif
static void SCE_AnimGeom_ApplySkeletonLocalP (SCE_SAnimatedGeometry*,
                                              SCE_SSkeleton*);
#if SCE_ANIMGEOM_ENABLE_QUAT_TRANSFORM
static void SCE_AnimGeom_ApplySkeletonLocalPQuat (SCE_SAnimatedGeometry*,
                                              SCE_SSkeleton*);
#endif


static void SCE_AnimGeom_ApplySkeletonP (SCE_SAnimatedGeometry *ageom,
                                         SCE_SSkeleton *skel)
{
    size_t i, j;

    for (i = 0; i < ageom->n_vertices; i++) {
        SCE_TMatrix4x3 mat;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        weight = &ageom->weights[vert->weight_id];
        MulMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                      mat);
        for (j = 1; j < vert->weight_count; j++) {
            weight = &ageom->weights[j + vert->weight_id];
            MadMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                          mat);
        }

        SCE_Matrix4x3_MulV4 (mat, &ageom->base[0][i*4], &ageom->output[0][i*3]);
    }
    /* TODO: how to estimate modified vertices range...?
       use joint-ranged skeletons? */
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
}
static void SCE_AnimGeom_ApplySkeletonPN (SCE_SAnimatedGeometry *ageom,
                                          SCE_SSkeleton *skel)
{
    size_t i, j;

    for (i = 0; i < ageom->n_vertices; i++) {
        SCE_TMatrix4x3 mat;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        weight = &ageom->weights[vert->weight_id];
        MulMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                      mat);
        for (j = 1; j < vert->weight_count; j++) {
            weight = &ageom->weights[j + vert->weight_id];
            MadMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                          mat);
        }

        SCE_Matrix4x3_MulV4 (mat, &ageom->base[0][i*4], &ageom->output[0][i*3]);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[1][i*4], &ageom->output[1][i*3]);
    }
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
    if (ageom->arrays[1])
        SCE_Geometry_Modified (ageom->arrays[1], NULL);
}
static void SCE_AnimGeom_ApplySkeletonPNT (SCE_SAnimatedGeometry *ageom,
                                           SCE_SSkeleton *skel)
{
    size_t i, j;

    for (i = 0; i < ageom->n_vertices; i++) {
        SCE_TMatrix4x3 mat;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        weight = &ageom->weights[vert->weight_id];
        MulMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                      mat);
        for (j = 1; j < vert->weight_count; j++) {
            weight = &ageom->weights[j + vert->weight_id];
            MadMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                          mat);
        }

        SCE_Matrix4x3_MulV4 (mat, &ageom->base[0][i*4], &ageom->output[0][i*3]);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[1][i*4], &ageom->output[1][i*3]);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[2][i*4], &ageom->output[2][i*3]);
    }
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
    if (ageom->arrays[1])
        SCE_Geometry_Modified (ageom->arrays[1], NULL);
    if (ageom->arrays[2])
        SCE_Geometry_Modified (ageom->arrays[2], NULL);
}
static void SCE_AnimGeom_ApplySkeletonPNTB (SCE_SAnimatedGeometry *ageom,
                                            SCE_SSkeleton *skel)
{
    size_t i, j;

    for (i = 0; i < ageom->n_vertices; i++) {
        SCE_TMatrix4x3 mat;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        weight = &ageom->weights[vert->weight_id];
        MulMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                      mat);
        for (j = 1; j < vert->weight_count; j++) {
            weight = &ageom->weights[j + vert->weight_id];
            MadMatScalar (&skel->mat[0][weight->joint_id * 12], weight->weight,
                          mat);
        }

        SCE_Matrix4x3_MulV4 (mat, &ageom->base[0][i*4], &ageom->output[0][i*3]);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[1][i*4], &ageom->output[1][i*3]);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[2][i*4], &ageom->output[2][i*3]);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[3][i*4], &ageom->output[3][i*3]);
    }
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
    if (ageom->arrays[1])
        SCE_Geometry_Modified (ageom->arrays[1], NULL);
    if (ageom->arrays[2])
        SCE_Geometry_Modified (ageom->arrays[2], NULL);
    if (ageom->arrays[3])
        SCE_Geometry_Modified (ageom->arrays[3], NULL);
}

static void SCE_AnimGeom_ApplySkeletonLocal (SCE_SAnimatedGeometry *ageom,
                                             unsigned int n,
                                             SCE_SSkeleton *skel)
{
    size_t i, j, index;
    float *out = NULL;

    for (i = 0; i < ageom->n_vertices; i++) {
        float *mat = NULL;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        index = vert->weight_id;
        out = &ageom->output[n][i * 3];

        weight = &ageom->weights[index];
        mat = &skel->mat[0][weight->joint_id * 12];
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[n][index * 4], out);
        for (j = 1; j < vert->weight_count; j++) {
            index++;
            weight = &ageom->weights[index];
            mat = &skel->mat[0][weight->joint_id * 12];
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[n][index * 4], out);
        }
    }
    /* TODO: range problem */
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
}
static void SCE_AnimGeom_ApplySkeletonLocalP (SCE_SAnimatedGeometry *ageom,
                                              SCE_SSkeleton *skel)
{
    size_t i, j, index;
    float *out = NULL;

    for (i = 0; i < ageom->n_vertices; i++) {
        float *mat = NULL;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        index = vert->weight_id;
        out = &ageom->output[0][i * 3];

        weight = &ageom->weights[index];
        mat = &skel->mat[0][weight->joint_id * 12];
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[0][index * 4], out);
        for (j = 1; j < vert->weight_count; j++) {
            index++;
            weight = &ageom->weights[index];
            mat = &skel->mat[0][weight->joint_id * 12];
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[0][index * 4], out);
        }
    }
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
}
#if SCE_ANIMGEOM_ENABLE_QUAT_TRANSFORM
static void ApplyJointToVector (SCE_SJoint *j, float bias, SCE_TVector3 in,
                                SCE_TVector3 out)
{
    /* calculate transformed vertex for this weight */
    SCE_TVector3 wv;
    SCE_Quaternion_RotateV3 (j->orientation, in, wv);

    /* the sum of all weight->bias should be 1.0 */
    out[0] += (j->position[0] + wv[0]) * bias;
    out[1] += (j->position[1] + wv[1]) * bias;
    out[2] += (j->position[2] + wv[2]) * bias;
}
static void SCE_AnimGeom_ApplySkeletonLocalPQuat (SCE_SAnimatedGeometry *ageom,
                                                  SCE_SSkeleton *skel)
{
    size_t i, j, index;
    float *out = NULL;
    SCE_SJoint *joints = NULL;

    joints = skel->joints;

    for (i = 0; i < ageom->n_vertices; i++) {
        float *mat = NULL;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];
        SCE_SJoint *joint = NULL;

        index = vert->weight_id;
        out = &ageom->output[0][i * 3];
        SCE_Vector3_Set (out, 0.0, 0.0, 0.0);
        for (j = 0; j < vert->weight_count; j++, index++) {
            weight = &ageom->weights[index];
            joint = &joints[weight->joint_id];
            ApplyJointToVector (joint, weight->weight,
                                &ageom->base[0][index * 4], out);
        }
    }
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
}
#endif
static void SCE_AnimGeom_ApplySkeletonLocalPN (SCE_SAnimatedGeometry *ageom,
                                               SCE_SSkeleton *skel)
{
    size_t i, j, index;
    float *out = NULL, *out2 = NULL;

    for (i = 0; i < ageom->n_vertices; i++) {
        float *mat = NULL;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        index = vert->weight_id;
        out = &ageom->output[0][i * 3];
        out2 = &ageom->output[1][i * 3];

        weight = &ageom->weights[index];
        mat = &skel->mat[0][weight->joint_id * 12];
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[0][index * 4], out);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[1][index * 4], out2);
        for (j = 1; j < vert->weight_count; j++) {
            index++;
            weight = &ageom->weights[index];
            mat = &skel->mat[0][weight->joint_id * 12];
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[0][index * 4], out);
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[1][index * 4], out2);
        }
    }
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
    if (ageom->arrays[1])
        SCE_Geometry_Modified (ageom->arrays[1], NULL);
}
static void SCE_AnimGeom_ApplySkeletonLocalPNT (SCE_SAnimatedGeometry *ageom,
                                                SCE_SSkeleton *skel)
{
    size_t i, j, index;
    float *out = NULL, *out2 = NULL, *out3 = NULL;

    for (i = 0; i < ageom->n_vertices; i++) {
        float *mat = NULL;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        index = vert->weight_id;
        out = &ageom->output[0][i * 3];
        out2 = &ageom->output[1][i * 3];
        out3 = &ageom->output[2][i * 3];

        weight = &ageom->weights[index];
        mat = &skel->mat[0][weight->joint_id * 12];
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[0][index * 4], out);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[1][index * 4], out2);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[2][index * 4], out3);
        for (j = 1; j < vert->weight_count; j++) {
            index++;
            weight = &ageom->weights[index];
            mat = &skel->mat[0][weight->joint_id * 12];
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[0][index * 4], out);
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[1][index * 4], out2);
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[2][index * 4], out3);
        }
    }
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
    if (ageom->arrays[1])
        SCE_Geometry_Modified (ageom->arrays[1], NULL);
    if (ageom->arrays[2])
        SCE_Geometry_Modified (ageom->arrays[2], NULL);
}
static void SCE_AnimGeom_ApplySkeletonLocalPNTB (SCE_SAnimatedGeometry *ageom,
                                                 SCE_SSkeleton *skel)
{
    size_t i, j, index;
    float *out = NULL, *out2 = NULL, *out3 = NULL, *out4 = NULL;

    for (i = 0; i < ageom->n_vertices; i++) {
        float *mat = NULL;
        SCE_SVertexWeight *weight;
        SCE_SVertex *vert = &ageom->vertices[i];

        index = vert->weight_id;
        out = &ageom->output[0][i * 3];
        out2 = &ageom->output[1][i * 3];
        out3 = &ageom->output[2][i * 3];
        out4 = &ageom->output[3][i * 3];

        weight = &ageom->weights[index];
        mat = &skel->mat[0][weight->joint_id * 12];
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[0][index * 4], out);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[1][index * 4], out2);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[2][index * 4], out3);
        SCE_Matrix4x3_MulV4 (mat, &ageom->base[3][index * 4], out4);
        for (j = 1; j < vert->weight_count; j++) {
            index++;
            weight = &ageom->weights[index];
            mat = &skel->mat[0][weight->joint_id * 12];
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[0][index * 4], out);
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[1][index * 4], out2);
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[2][index * 4], out3);
            SCE_Matrix4x3_MulV4Add (mat, &ageom->base[3][index * 4], out4);
        }
    }
    if (ageom->arrays[0])
        SCE_Geometry_Modified (ageom->arrays[0], NULL);
    if (ageom->arrays[1])
        SCE_Geometry_Modified (ageom->arrays[1], NULL);
    if (ageom->arrays[2])
        SCE_Geometry_Modified (ageom->arrays[2], NULL);
    if (ageom->arrays[3])
        SCE_Geometry_Modified (ageom->arrays[3], NULL);
}

/**
 * \brief Applies a skeleton to an animated geometry
 *
 * This function does not update the internal SCE_SMesh mesh of \p ageom,
 * so call SCE_AnimGeom_Prout() for that.
 * \sa SCE_AnimGeom_Prout()
 */
void SCE_AnimGeom_ApplySkeleton (SCE_SAnimatedGeometry *ageom, SCE_SSkeleton *skel)
{
    ageom->applyskel (ageom, skel);
}

/**
 * \brief Applies the bind pose skeleton of an animated geometry
 * \sa SCE_AnimGeom_ApplySkeleton()
 */
void SCE_AnimGeom_ApplyBaseSkeleton (SCE_SAnimatedGeometry *ageom)
{
#ifdef SCE_DEBUG
    if (!ageom->baseskel) {
        SCEE_SendMsg ("this animated geometry has not bind pose skeleton, SCE_"
                      "AnimGeom_ApplyBaseSkeleton() aborted");
    }
    else
#endif
    SCE_AnimGeom_ApplySkeleton (ageom, ageom->baseskel);
}
/**
 * \brief Applies the bind pose skeleton of an animated geometry
 * \sa SCE_AnimGeom_ApplySkeleton()
 */
void SCE_AnimGeom_ApplyAnimSkeleton (SCE_SAnimatedGeometry *ageom)
{
#ifdef SCE_DEBUG
    if (!ageom->animskel) {
        SCEE_SendMsg ("this animated geometry has not animation skeleton, SCE_"
                      "AnimGeom_ApplyBaseSkeleton() aborted");
    }
    else
#endif
    SCE_AnimGeom_ApplySkeleton (ageom, ageom->animskel);
}

#if 0
/**
 * \brief Sets the vertices in local position of the joints (so duplicate
 * vertices if necessary)
 * \param bpose the bind pose skeleton
 * \sa SCE_AnimGeom_SetGlobal()
 */
void SCE_AnimGeom_SetLocal (SCE_SAnimatedGeometry *ageom, SCE_SSkeleton *bpose)
{
    SCEvertices *vert = NULL;
}
#endif
/**
 * \brief Set the vertices in global position
 * \param bpose the bind pose skeleton
 * \sa SCE_AnimGeom_SetGlobal()
 * \todo manage the w component
 */
int SCE_AnimGeom_SetGlobal (SCE_SAnimatedGeometry *ageom)
{
    size_t i;

    for (i = 0; i < SCE_MAX_ANIMATED_VERTEX_ATTRIBUTES; i++) {
        if (ageom->local[i] && ageom->base[i]) {
            SCEvertices *vert = NULL;
            size_t j;
            /* alloc new base */
            if (!(vert = SCE_malloc (ageom->n_vertices * 4 * sizeof *vert))) {
                SCEE_LogSrc ();
                return SCE_ERROR;
            }
            SCE_AnimGeom_ApplySkeletonLocal (ageom, i, ageom->baseskel);
            for (j = 0; j < ageom->n_vertices; j++) {
                SCE_Vector3_Copy (&vert[j * 4], &ageom->output[i][j * 3]);
            }
            SCE_free (ageom->base[i]);
            ageom->base[i] = vert;
            ageom->local[i] = SCE_FALSE;
        }
    }
    return SCE_OK;
}

/**
 * \brief Builds the internal SCE_SGeometry of an animated geometry
 * \sa SCE_AnimGeom_GetGeometry()
 */
int SCE_AnimGeom_BuildGeometry (SCE_SAnimatedGeometry *ageom)
{
    if (ageom->base[0]) {
        ageom->arrays[0] =
            SCE_Geometry_AddNewArray (ageom->geom, SCE_POSITION,
                                      SCE_VERTICES_TYPE, 0, 3,
                                      ageom->output[0], SCE_FALSE);
                                      
        if (!ageom->arrays[0])
            goto fail;
    }
    if (ageom->base[1]) {
        ageom->arrays[1] =
            SCE_Geometry_AddNewArray (ageom->geom, SCE_NORMAL,
                                      SCE_VERTICES_TYPE, 0, 3,
                                      ageom->output[1], SCE_FALSE);
        if (!ageom->arrays[1])
            goto fail;
    }
    if (ageom->base[2]) {
        ageom->arrays[2] =
            SCE_Geometry_AddNewArray (ageom->geom, SCE_TANGENT,
                                      SCE_VERTICES_TYPE, 0, 3,
                                      ageom->output[2], SCE_FALSE);
        if (!ageom->arrays[2])
            goto fail;
    }
    if (ageom->base[3]) {
        ageom->arrays[3] =
            SCE_Geometry_AddNewArray (ageom->geom, SCE_BINORMAL,
                                      SCE_VERTICES_TYPE, 0, 3,
                                      ageom->output[3], SCE_FALSE);
        if (!ageom->arrays[3])
            goto fail;
    }
    if (ageom->indices) {
        SCE_SGeometryArray array;
        SCE_Geometry_InitArray (&array);
        SCE_Geometry_SetArrayIndices (&array, SCE_INDICES_TYPE,
                                      ageom->indices, SCE_FALSE);
        if (SCE_Geometry_SetIndexArrayDup (ageom->geom, &array, SCE_FALSE) < 0)
            goto fail;
        SCE_Geometry_SetNumIndices (ageom->geom, ageom->n_indices);
    }
    SCE_Geometry_SetNumVertices (ageom->geom, ageom->n_vertices);
    SCE_Geometry_SetPrimitiveType (ageom->geom, SCE_TRIANGLES);

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

/**
 * \brief Loads an animated geometry from a file
 * \param fmesh the file of the mesh
 * \param force force reload of the resource
 * \returns a new animated geometry based on the given files or NULL on error
 * \sa SCE_Resource_Load()
 */
SCE_SAnimatedGeometry* SCE_AnimGeom_Load (const char *fmesh, int force)
{
    SCE_SAnimatedGeometry *ageom = NULL;
    if (!(ageom = SCE_Resource_Load (resource_type, fmesh, force, NULL)))
        SCEE_LogSrc ();
    return ageom;
}

/* @} */
