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
   updated: 20/04/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEForestTree.h"

void SCE_FTree_InitNode (SCE_SForestTreeNode *node)
{
    int i;
    node->parent = NULL;
    SCE_Matrix4x3_Identity (node->matrix);
    node->radius = 1.0;
    node->distance = 1.0;
    for (i = 0; i < SCE_MAX_FTREE_DEGREE; i++)
        node->children[i] = NULL;
    node->n_children = 0;
    node->n_vertices = 0;
    node->n_indices = 0;
    node->n_nodes = 0;
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
    SCE_Geometry_Init (&ft->geom);

    ft->matrix_data = NULL;
    ft->drad_data = NULL;
    ft->indices_data = NULL;

    ft->index_counter = ft->vertex_counter = 0;

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
}
void SCE_FTree_Clear (SCE_SForestTree *ft)
{
    SCE_FTree_ClearNode (&ft->root);
    SCE_Geometry_Clear (&ft->geom);
    SCE_free (ft->matrix_data);
    SCE_free (ft->drad_data);
    SCE_free (ft->indices_data);
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
    dst->index = src->index;
    for (i = 0; i < src->n_children; i++)
        dst->children[i] = src->children[i];
    dst->n_children = src->n_children;
    dst->n_vertices = src->n_vertices;
    dst->n_indices = src->n_indices;
    dst->n_nodes = src->n_nodes;
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
        node->distance = 0.0;   /* lol */
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
    size_t n_nodes = 0, n_vertices = 0, n = node->n_children;

    for (i = 0; i < n; i++) {
        SCE_FTree_Count (node->children[i]);
        n_nodes += node->children[i]->n_nodes;
        n_vertices += node->children[i]->n_vertices;
    }

    node->n_nodes = n_nodes + 1;
    node->n_vertices = n_vertices + 1 + (n > 1 ? n - 1 : 0);
    node->n_indices = (node->n_nodes - 1) * 2;
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

int SCE_FTree_Build (SCE_SForestTree *ft)
{
    SCE_FTree_Count (&ft->root);

    if (ft->root.n_vertices >= 256 * 256) {
        /* the tree is too fucking big */
        return SCE_ERROR;
    }

    SCE_FTree_ComputePlanes (&ft->root);

    /* allocate data */
    if (!(ft->matrix_data = SCE_malloc (15 * ft->root.n_vertices *
                                        sizeof *ft->matrix_data)))
        goto fail;
    if (!(ft->drad_data = SCE_malloc (2 * ft->root.n_vertices *
                                      sizeof *ft->drad_data)))
        goto fail;
    if (!(ft->indices_data = SCE_malloc (ft->root.n_indices *
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

    SCE_Geometry_AddArrayRecDup (&ft->geom, &ft->ar1, SCE_FALSE);
    SCE_Geometry_SetIndexArray (&ft->geom, &ft->ind, SCE_FALSE);
    SCE_Geometry_SetPrimitiveType (&ft->geom, SCE_LINES);

    /* fillup vertex arrays */
    SCE_FTree_Update (ft);

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
    ft->indices_data[ft->index_counter + 0] = (SCEindices)node->parent->index;
    ft->indices_data[ft->index_counter + 1] = (SCEindices)node->index;
    ft->index_counter += 2;
    if (node->n_children > 0)
        SCE_FTree_UpdateNode (ft, node->children[0]);

    SCE_Matrix4x3_GetTranslation (node->matrix, pos);
    SCE_FTree_CopyNode (&tmp, node);

    for (i = 1; i < node->n_children; i++) {
        /* construct new vertex */
        SCE_Matrix4x3_GetTranslation (node->children[i]->matrix, tmp.plane);
        SCE_Vector3_Operator2v (tmp.plane, =, tmp.plane, -, pos);
        SCE_Vector3_Normalize (tmp.plane);
        tmp.radius = node->children[i]->radius;

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
void SCE_FTree_Update (SCE_SForestTree *ft)
{
    size_t i;

    /* DFS: keep vertex locality for optimal vertex caching */
    ft->root.index = 0;
    ft->index_counter = 0;
    ft->vertex_counter = 0;
    SCE_FTree_OutputVertex (ft, &ft->root);

    /* TODO: warning, no additional vertex is created as in UpdateNode() */
    for (i = 0; i < ft->root.n_children; i++)
        SCE_FTree_UpdateNode (ft, ft->root.children[i]);

    SCE_FTree_Count (&ft->root);
    SCE_Geometry_SetNumVertices (&ft->geom, ft->root.n_vertices);
    SCE_Geometry_SetNumIndices (&ft->geom, ft->root.n_indices);
}

SCE_SGeometry* SCE_FTree_GetGeometry (SCE_SForestTree *ft)
{
    return &ft->geom;
}
