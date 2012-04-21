/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2012  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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

/* created: 19/04/2012
   updated: 21/04/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEForestTree.h"

void SCE_FTree_InitNode (SCE_SForestTreeNode *node)
{
    int i;

    node->parent = NULL;
    SCE_Matrix4x3_Identity (node->matrix);
    node->radius = 1.0;
    node->distance = 1.0;
    node->n_polygons = 8;

    node->index = 0;
    for (i = 0; i < SCE_MAX_FTREE_DEGREE; i++)
        node->children[i] = NULL;
    node->n_children = 0;
    node->n_nodes = 0;

    node->n_vertices1 = node->n_indices1 = 0;
    node->n_vertices2 = node->n_indices2 = 0;
    SCE_List_InitIt (&node->it);
    SCE_List_SetData (&node->it, node);
}
void SCE_FTree_ClearNode (SCE_SForestTreeNode *node)
{
    int i;
    for (i = 0; i < SCE_MAX_FTREE_DEGREE; i++)
        SCE_FTree_DeleteNode (node->children[i]);
}
SCE_SForestTreeNode* SCE_FTree_CreateNode (void)
{
    SCE_SForestTreeNode *node = NULL;
    if (!(node = SCE_malloc (sizeof *node)))
        SCEE_LogSrc ();
    else {
        SCE_FTree_InitNode (node);
    }
    return node;
}
void SCE_FTree_DeleteNode (SCE_SForestTreeNode *node)
{
    if (node) {
        SCE_FTree_ClearNode (node);
        SCE_free (node);
    }
}


void SCE_FTree_Init (SCE_SForestTree *ft)
{
    SCE_FTree_InitNode (&ft->root);

    SCE_Geometry_Init (&ft->tree_geom);
    ft->matrix_data = NULL;
    ft->drad_data = NULL;
    ft->npoly_data = NULL;
    ft->indices_data = NULL;
    SCE_Geometry_InitArray (&ft->ar1);
    SCE_Geometry_InitArray (&ft->ar2);
    SCE_Geometry_InitArray (&ft->ar3);
    SCE_Geometry_InitArray (&ft->ar4);
    SCE_Geometry_InitArray (&ft->ar5);
    SCE_Geometry_InitArray (&ft->ar6);
    SCE_Geometry_InitArray (&ft->ind);
    SCE_Geometry_AttachArray (&ft->ar1, &ft->ar2);
    SCE_Geometry_AttachArray (&ft->ar2, &ft->ar3);
    SCE_Geometry_AttachArray (&ft->ar3, &ft->ar4);
    SCE_Geometry_AttachArray (&ft->ar4, &ft->ar5);
    /* TODO: 'attach' 6th array */

    SCE_Geometry_Init (&ft->final_geom);
    ft->vertices = NULL;
    ft->indices = NULL;
    SCE_Geometry_InitArray (&ft->pos);
    SCE_Geometry_InitArray (&ft->nor);
    SCE_Geometry_InitArray (&ft->tc);
    SCE_Geometry_InitArray (&ft->idx);
    SCE_Geometry_AttachArray (&ft->pos, &ft->nor);
#if 0
    SCE_Geometry_AttachArray (&ft->nor, &ft->tc);
#endif

    ft->index_counter = ft->vertex_counter = 0;
}
void SCE_FTree_Clear (SCE_SForestTree *ft)
{
    SCE_FTree_ClearNode (&ft->root);
    SCE_Geometry_Clear (&ft->tree_geom);
    SCE_Geometry_Clear (&ft->final_geom);
    SCE_free (ft->matrix_data);
    SCE_free (ft->drad_data);
    SCE_free (ft->npoly_data);
    SCE_free (ft->indices_data);
    SCE_free (ft->vindex);
    SCE_free (ft->vertices);
    SCE_free (ft->indices);
}
SCE_SForestTree* SCE_FTree_Create (void)
{
    SCE_SForestTree *ft = NULL;
    if (!(ft = SCE_malloc (sizeof *ft)))
        SCEE_LogSrc ();
    else {
        SCE_FTree_Init (ft);
    }
    return ft;
}
void SCE_FTree_Delete (SCE_SForestTree *ft)
{
    if (ft) {
        SCE_FTree_Clear (ft);
        SCE_free (ft);
    }
}

/* bruteforce copy */
void SCE_FTree_CopyNode (SCE_SForestTreeNode *dst,
                         const SCE_SForestTreeNode *src)
{
    size_t i;
    dst->parent = src->parent;
    SCE_Matrix4x3_Copy (dst->matrix, src->matrix);
    SCE_Vector3_Copy (dst->plane, src->plane);
    dst->distance = src->distance;
    dst->radius = src->radius;
    dst->n_polygons = src->n_polygons;

    dst->index = src->index;
    for (i = 0; i < src->n_children; i++)
        dst->children[i] = src->children[i];
    dst->n_children = src->n_children;
    dst->n_nodes = src->n_nodes;

    dst->n_vertices1 = src->n_vertices1;
    dst->n_indices1 = src->n_indices1;
    dst->n_vertices2 = src->n_vertices2;
    dst->n_indices2 = src->n_indices2;
}


SCE_SForestTreeNode* SCE_FTree_GetRootNode (SCE_SForestTree *ft)
{
    return &ft->root;
}
void SCE_FTree_AddNode (SCE_SForestTreeNode *parent, SCE_SForestTreeNode *node)
{
    if (parent->n_children < SCE_MAX_FTREE_DEGREE) {
        parent->children[parent->n_children] = node;
        parent->n_children++;
        node->parent = parent;
        /* TODO: compute distance */
    }
}
void SCE_FTree_RemoveNode (SCE_SForestTreeNode *node)
{
    SCE_SForestTreeNode *p = node->parent;

    if (p) {
        size_t i;

        for (i = 0; i < p->n_children; i++) {
            if (node->parent->children[i] == node) {
                p->n_children--;
                p->children[i] = p->children[p->n_children];
                p->children[p->n_children] = NULL;
                break;
            }
        }

        node->parent = NULL;
        node->distance = 0.0;   /* TODO: lol */
    }
}

void SCE_FTree_SetNodeRadius (SCE_SForestTreeNode *node, float r)
{
    node->radius = r;
}
float SCE_FTree_GetNodeRadius (const SCE_SForestTreeNode *node)
{
    return node->radius;
}
float SCE_FTree_GetNodeDistance (const SCE_SForestTreeNode *node)
{
    return node->distance;
}
SCEuint SCE_FTree_GetNodeNumVertices (const SCE_SForestTreeNode *node)
{
    return node->n_polygons;
}
void SCE_FTree_SetNodeNumVertices (SCE_SForestTreeNode *node, SCEuint n)
{
    node->n_polygons = n;
}


void SCE_FTree_SetNodeMatrix (SCE_SForestTreeNode *node,
                              const SCE_TMatrix4x3 matrix)
{
    SCE_Matrix4x3_Copy (node->matrix, matrix);
    /* TODO: compute new distance */
}
float* SCE_FTree_GetNodeMatrix (SCE_SForestTreeNode *node)
{
    /* TODO: dont allow the user to modify the matrix */
    return node->matrix;
}
void SCE_FTree_GetNodeMatrixv (SCE_SForestTreeNode *node, SCE_TMatrix4x3 mat)
{
    SCE_Matrix4x3_Copy (mat, node->matrix);
}
size_t SCE_FTree_GetNumNodeChildren (const SCE_SForestTreeNode *node)
{
    return node->n_children;
}


/* counts the number of nodes */
static void
SCE_FTree_Count /* Dooku */ (SCE_SForestTreeNode *node)
{
    size_t i;
    size_t n_nodes = 0, n_vertices1 = 0, n_vertices2 = 0, n_indices2 = 0;
    size_t n = node->n_children;

    for (i = 0; i < n; i++) {
        SCE_FTree_Count (node->children[i]);
        n_nodes += node->children[i]->n_nodes;
        n_vertices1 += node->children[i]->n_vertices1;
        n_vertices2 += node->children[i]->n_vertices2;
        n_indices2 += node->children[i]->n_indices2;
        n_indices2 += 3 * (node->n_polygons + node->children[i]->n_polygons);
    }

    node->n_nodes = n_nodes + 1;
    node->n_vertices1 = n_vertices1 + 1 + (n > 1 ? n - 1 : 0);
    node->n_vertices2 = n_vertices2 + node->n_polygons;
    if (n > 1)
        node->n_vertices2 += (n - 1) * node->n_polygons;

    node->n_indices1 = n_nodes * 2;
    node->n_indices2 = n_indices2;
}


static void
SCE_FTree_ComputePlanesAux (SCE_SForestTreeNode *node)
{
    size_t i;
    SCE_TVector3 u, v, pos;

    /* vector parent -> node */
    SCE_Matrix4x3_GetTranslation (node->parent->matrix, v);
    SCE_Matrix4x3_GetTranslation (node->matrix, pos);
    SCE_Vector3_Operator2v (u, =, pos, -, v);
    SCE_Vector3_Normalize (u);

    if (node->n_children > 0) {
        /* vector node -> child0 */
        SCE_Matrix4x3_GetTranslation (node->children[0]->matrix, v);
        SCE_Vector3_Operator1v (v, -=, pos);
        SCE_Vector3_Normalize (v);
        /* average */
        SCE_Vector3_Operator1v (u, +=, v);
        SCE_Vector3_Normalize (u);
    }

    SCE_Vector3_Copy (node->plane, u);

    for (i = 0; i < node->n_children; i++)
        SCE_FTree_ComputePlanesAux (node->children[i]);
}

static void
SCE_FTree_ComputePlanes (SCE_SForestTreeNode *node)
{
    size_t i;
    SCE_TVector3 x, y, z;

    /* root plane is determined by the direction vector */
    SCE_Matrix4x3_GetBase (node->matrix, x, y, z);
    SCE_Vector3_Copy (node->plane, z);
    SCE_Vector3_Normalize (node->plane);

    for (i = 0; i < node->n_children; i++)
        SCE_FTree_ComputePlanesAux (node->children[i]);
}

static int SCE_FTree_BuildTreeGeom (SCE_SForestTree *ft)
{
    /* allocate data */
    if (!(ft->matrix_data = SCE_malloc (15 * ft->root.n_vertices1 *
                                        sizeof *ft->matrix_data)))
        goto fail;
    if (!(ft->drad_data = SCE_malloc (2 * ft->root.n_vertices1 *
                                      sizeof *ft->drad_data)))
        goto fail;
    if (!(ft->npoly_data = SCE_malloc (ft->root.n_vertices1 *
                                       sizeof *ft->npoly_data)))
        goto fail;
    if (!(ft->indices_data = SCE_malloc (ft->root.n_indices1 *
                                         sizeof *ft->indices_data)))
        goto fail;

    /* position */
    SCE_Geometry_SetArrayData (&ft->ar1, SCE_POSITION, SCE_FLOAT, 0, 3,
                               ft->matrix_data, SCE_FALSE);
    /* base matrix */
    SCE_Geometry_SetArrayData (&ft->ar2, SCE_TEXCOORD0, SCE_FLOAT, 0, 3,
                               &ft->matrix_data[3], SCE_FALSE);
    SCE_Geometry_SetArrayData (&ft->ar3, SCE_TEXCOORD1, SCE_FLOAT, 0, 3,
                               &ft->matrix_data[6], SCE_FALSE);
    SCE_Geometry_SetArrayData (&ft->ar4, SCE_TEXCOORD2, SCE_FLOAT, 0, 3,
                               &ft->matrix_data[9], SCE_FALSE);
    /* plane normal */
    SCE_Geometry_SetArrayData (&ft->ar5, SCE_TEXCOORD3, SCE_FLOAT, 0, 3,
                               &ft->matrix_data[12], SCE_FALSE);
    /* TODO: if interleaved, use the same pointer for drad */
    /* distance and radius */
    SCE_Geometry_SetArrayData (&ft->ar6, SCE_TEXCOORD4, SCE_FLOAT, 0, 2,
                               ft->drad_data, SCE_FALSE);

    SCE_Geometry_SetArrayIndices (&ft->ind, SCE_INDICES_TYPE, ft->indices_data,
                                  SCE_FALSE);

    SCE_Geometry_AddArrayRecDup (&ft->tree_geom, &ft->ar1, SCE_FALSE);
    SCE_Geometry_SetIndexArray (&ft->tree_geom, &ft->ind, SCE_FALSE);
    SCE_Geometry_SetPrimitiveType (&ft->tree_geom, SCE_LINES);

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

static int SCE_FTree_BuildFinalGeom (SCE_SForestTree *ft)
{
#define V_SIZE 6
    /* allocate data */
    if (!(ft->vertices = SCE_malloc (V_SIZE * ft->root.n_vertices2 *
                                     sizeof *ft->vertices)))
        goto fail;
    if (!(ft->indices = SCE_malloc (ft->root.n_indices2 * sizeof *ft->indices)))
        goto fail;

    SCE_Geometry_SetArrayData (&ft->pos, SCE_POSITION, SCE_FLOAT, 0, 3,
                               ft->vertices, SCE_FALSE);
    SCE_Geometry_SetArrayData (&ft->nor, SCE_NORMAL, SCE_FLOAT, 0, 3,
                               &ft->vertices[3], SCE_FALSE);
#if 0
    SCE_Geometry_SetArrayData (&ft->tc, SCE_TEXCOORD0, SCE_FLOAT, 0, 2,
                               &ft->vertices[5], SCE_FALSE);
#endif

    SCE_Geometry_SetArrayIndices (&ft->idx, SCE_INDICES_TYPE, ft->indices,
                                  SCE_FALSE);

    SCE_Geometry_AddArrayRecDup (&ft->final_geom, &ft->pos, SCE_FALSE);
    SCE_Geometry_SetIndexArray (&ft->final_geom, &ft->idx, SCE_FALSE);
    SCE_Geometry_SetPrimitiveType (&ft->final_geom, SCE_TRIANGLES);

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}

int SCE_FTree_Build (SCE_SForestTree *ft)
{
    SCE_FTree_Count (&ft->root);

    if (ft->root.n_vertices1 >= 256 * 256 ||
        ft->root.n_vertices2 >= 256 * 256) {
        /* the tree is too fucking big */
        return SCE_ERROR;
    }

    SCE_FTree_ComputePlanes (&ft->root);

    if (SCE_FTree_BuildTreeGeom (ft) < 0) goto fail;
    if (SCE_FTree_BuildFinalGeom (ft) < 0) goto fail;

    if (!(ft->vindex = SCE_malloc (ft->root.n_vertices1 * sizeof *ft->vindex)))
        goto fail;

    /* fillup vertex arrays */
    SCE_FTree_UpdateTreeGeometry (ft);
    SCE_FTree_UpdateFinalGeometry (ft);

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


static void SCE_FTree_OutputVertex (SCE_SForestTree *ft,
                                    SCE_SForestTreeNode *node)
{
    SCE_TVector3 x, y, z;
    SCEuint index;

    /* copy matrix */
    node->index = index = ft->vertex_counter;
    /* TODO: SCEvertices and SCE_TVector3 types may conflict */
    SCE_Matrix4x3_GetTranslation (node->matrix, x);
    SCE_Vector3_Copy (&ft->matrix_data[index * 15 + 0], x);
    SCE_Matrix4x3_GetBase (node->matrix, x, y, z);
    SCE_Vector3_Copy (&ft->matrix_data[index * 15 + 3], x);
    SCE_Vector3_Copy (&ft->matrix_data[index * 15 + 6], y);
    SCE_Vector3_Copy (&ft->matrix_data[index * 15 + 9], z);
    SCE_Vector3_Copy (&ft->matrix_data[index * 15 + 12], node->plane);

    /* copy distance, radius & stuff */
    ft->drad_data[index * 2 + 0] = node->radius;
    ft->drad_data[index * 2 + 1] = node->distance;
    ft->npoly_data[index] = node->n_polygons;

    ft->vertex_counter++;
}

static void
SCE_FTree_UpdateNode (SCE_SForestTree *ft, SCE_SForestTreeNode *node)
{
    size_t i;
    SCE_SForestTreeNode tmp;
    SCE_TVector3 pos;

    /* main branch */
    /* output vertex and index */
    SCE_FTree_OutputVertex (ft, node);
    if (node->parent) {
        ft->indices_data[ft->index_counter + 0] = (SCEindices)node->parent->index;
        ft->indices_data[ft->index_counter + 1] = (SCEindices)node->index;
        ft->index_counter += 2;
    }
    if (node->n_children > 0)
        SCE_FTree_UpdateNode (ft, node->children[0]);

    SCE_Matrix4x3_GetTranslation (node->matrix, pos);
    SCE_FTree_CopyNode (&tmp, node);

    for (i = 1; i < node->n_children; i++) {
        /* construct new vertex */
        SCE_Matrix4x3_GetTranslation (node->children[i]->matrix, tmp.plane);
        SCE_Vector3_Operator2v (tmp.plane, =, tmp.plane, -, pos);
        SCE_Vector3_Normalize (tmp.plane);
        tmp.radius = MIN (node->children[i]->radius, node->radius);

        /* output vertex */
        SCE_FTree_OutputVertex (ft, &tmp);

        /* hax */
        node->children[i]->parent = &tmp;
        SCE_FTree_UpdateNode (ft, node->children[i]);
        node->children[i]->parent = node;
    }
}

/**
 * \brief Update the vertex arrays
 * \param ft a tree
 * \return
 */
void SCE_FTree_UpdateTreeGeometry (SCE_SForestTree *ft)
{
    /* DFS: keep vertex locality for optimal vertex caching */
    ft->root.index = 0;
    ft->index_counter = 0;
    ft->vertex_counter = 0;
    SCE_FTree_UpdateNode (ft, &ft->root);

    SCE_FTree_Count (&ft->root);
    SCE_Geometry_SetNumVertices (&ft->tree_geom, ft->root.n_vertices1);
    SCE_Geometry_SetNumIndices (&ft->tree_geom, ft->root.n_indices1);
}



void SCE_FTree_UpdateFinalGeometry (SCE_SForestTree *ft)
{
    size_t i, j;
    size_t previous_index = 0;

    /* NOTE: pray for ft->tree_geom to be up-to-date */

    /* step1: generate vertices */
    for (i = 0; i < ft->root.n_vertices1; i++) {
        SCE_SPlane p;
        SCE_SLine3 l;
        SCE_TVector3 x, y, z;

        /* sum up the total number of output vertices to get indices */
        ft->vindex[i] = previous_index;

        SCE_Vector3_Copy (x, &ft->matrix_data[i * 15 + 3]);
        SCE_Vector3_Copy (y, &ft->matrix_data[i * 15 + 6]);
        SCE_Vector3_Normalize (x);
        SCE_Vector3_Normalize (y);

        SCE_Vector3_Copy (z, &ft->matrix_data[i * 15 + 12]);
        SCE_Vector3_Normalize (z);
        SCE_Plane_SetFromPointv (&p, z, &ft->matrix_data[i * 15]);
        SCE_Vector3_Copy (z, &ft->matrix_data[i * 15 + 9]);
        SCE_Vector3_Normalize (z);
        SCE_Line3_SetNormal (&l, z);

        for (j = 0; j < ft->npoly_data[i]; j++) {
            SCE_TVector3 pos;
            float c, s;
            float angle = M_PI * 2.0 * j / (float)ft->npoly_data[i];

            /* compute position */
            c = cos (angle) * ft->drad_data[i * 2];
            s = sin (angle) * ft->drad_data[i * 2];
            SCE_Vector3_Operator2v (pos, = c *, x, + s *, y);
            SCE_Vector3_Operator1v (pos, +=, &ft->matrix_data[i * 15]);

            SCE_Line3_SetOrigin (&l, pos);
            /* project the position on the node plane */
            SCE_Plane_LineIntersection (&p, &l,
                                        &ft->vertices[previous_index * V_SIZE]);

            /* compute normal: vector from node's position to current vertex */
            SCE_Vector3_Operator2v (&ft->vertices[previous_index * V_SIZE + 3], =,
                                    &ft->vertices[previous_index * V_SIZE], -,
                                    &ft->matrix_data[i * 15]);
            SCE_Vector3_Normalize (&ft->vertices[previous_index * V_SIZE + 3]);

            /* compute texture coordinates */
            /* TODO: compute distance */
            /* lol mingface todo. */

            previous_index++;
        }
    }

    previous_index = 0;         /* used to count position into index array */

    /* step2: generate indices */
    for (i = 0; i < ft->root.n_indices1; i += 2) {
        /* indices of vertices from each side of the branch */
        size_t index1 = ft->vindex[ft->indices_data[i + 0]];
        size_t index2 = ft->vindex[ft->indices_data[i + 1]];
        /* number of vertices on each side */
        size_t n1 = ft->npoly_data[ft->indices_data[i + 0]];
        size_t n2 = ft->npoly_data[ft->indices_data[i + 1]];

        /* generate triangles from each side */
        for (j = 0; j < n1; j++) {
            /* I know what I'm doing. */
            float offset = ((float)j + 0.0) / n1;
            size_t index = offset * n2 + 0.5;

            ft->indices[previous_index * 3 + 1] = index1 + j;
            if (index >= n2)
                ft->indices[previous_index * 3 + 0] = index2;
            else
                ft->indices[previous_index * 3 + 0] = index2 + index;
            if (j >= n1 - 1)
                ft->indices[previous_index * 3 + 2] = index1;
            else
                ft->indices[previous_index * 3 + 2] = index1 + j + 1;

            previous_index++;
        }

        /* repeat for the other side */
        for (j = 0; j < n2; j++) {
            /* I STILL know what I'm doing. */
            float offset = ((float)j + 0.5) / n2;
            size_t index = offset * n1 + 0.5;

            ft->indices[previous_index * 3 + 0] = index2 + j;
            if (index >= n1)
                ft->indices[previous_index * 3 + 1] = index1;
            else
                ft->indices[previous_index * 3 + 1] = index1 + index;
            if (j >= n2 - 1)
                ft->indices[previous_index * 3 + 2] = index2;
            else
                ft->indices[previous_index * 3 + 2] = index2 + j + 1;

            previous_index++;
        }
    }

    SCE_Geometry_SetNumVertices (&ft->final_geom, ft->root.n_vertices2);
    SCE_Geometry_SetNumIndices (&ft->final_geom, ft->root.n_indices2);
}


SCE_SGeometry* SCE_FTree_GetTreeGeometry (SCE_SForestTree *ft)
{
    return &ft->tree_geom;
}

SCE_SGeometry* SCE_FTree_GetFinalGeometry (SCE_SForestTree *ft)
{
    return &ft->final_geom;
}
