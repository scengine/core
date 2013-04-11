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
    SCE_Vector3_Set (node->plane, 0.0, 0.0, 0.0);
    node->radius = 1.0;
    node->distance = 1.0;
    node->n_polygons = 8;

    node->leaf_index = -1;      /* no leaf */
    SCE_Matrix4x3_Identity (node->leaf_matrix);

    node->index = 0;
    for (i = 0; i < SCE_MAX_FTREE_DEGREE; i++)
        node->children[i] = NULL;
    node->n_children = 0;
    node->n_nodes = 0;
    for (i = 0; i < SCE_MAX_FTREE_BUSH_TYPES; i++)
        node->n_leaves[i] = 0;

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
    SCE_Geometry_AttachArray (&ft->nor, &ft->tc);

    ft->index_counter = ft->vertex_counter = 0;
}
void SCE_FTree_Clear (SCE_SForestTree *ft)
{
    size_t i;

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
    for (i = 0; i < SCE_MAX_FTREE_BUSH_TYPES; i++)
        SCE_free (ft->bushes[i]);
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

    dst->leaf_index = src->leaf_index;
    SCE_Matrix4x3_Copy (dst->leaf_matrix, src->leaf_matrix);

    dst->index = src->index;
    for (i = 0; i < src->n_children; i++)
        dst->children[i] = src->children[i];
    dst->n_children = src->n_children;
    dst->n_nodes = src->n_nodes;
    for (i = 0; i < SCE_MAX_FTREE_BUSH_TYPES; i++)
        dst->n_leaves[i] = src->n_leaves[i];

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

/**
 * \brief Sets a type of bush to a node, -1 sets none
 * \param node a node
 * \param index whatever index you want, below SCE_MAX_FTREE_BUSH_TYPES
 */
void SCE_FTree_SetBush (SCE_SForestTreeNode *node, int index)
{
    node->leaf_index = index;
}
int SCE_FTree_GetBush (const SCE_SForestTreeNode *node)
{
    return node->leaf_index;
}
void SCE_FTree_SetBushMatrix (SCE_SForestTreeNode *node, const SCE_TMatrix4x3 m)
{
    SCE_Matrix4x3_Copy (node->leaf_matrix, m);
}


/* counts the number of nodes */
static void
SCE_FTree_Count /* Dooku */ (SCE_SForestTreeNode *node)
{
    size_t i, j;
    size_t n_nodes = 0, n_vertices1 = 0, n_vertices2 = 0, n_indices2 = 0;
    size_t n = node->n_children;

    for (i = 0; i < SCE_MAX_FTREE_BUSH_TYPES; i++)
        node->n_leaves[i] = 0;

    for (i = 0; i < n; i++) {
        SCE_FTree_Count (node->children[i]);
        n_nodes += node->children[i]->n_nodes;
        n_vertices1 += node->children[i]->n_vertices1;
        n_vertices2 += node->children[i]->n_vertices2;
        n_indices2 += node->children[i]->n_indices2;
        n_indices2 += 3 * (node->n_polygons + node->children[i]->n_polygons);
        for (j = 0; j < SCE_MAX_FTREE_BUSH_TYPES; j++)
            node->n_leaves[j] += node->children[i]->n_leaves[j];
    }

    node->n_nodes = n_nodes + 1;
    node->n_vertices1 = n_vertices1 + 1 + (n > 1 ? n - 1 : 0);
    node->n_vertices2 = n_vertices2 + node->n_polygons + 1;
    if (n > 1)
        node->n_vertices2 += (n - 1) * (node->n_polygons + 1);

    node->n_indices1 = n_nodes * 2;
    node->n_indices2 = n_indices2;

    if (node->leaf_index >= 0)
        node->n_leaves[node->leaf_index]++;
}


static void
SCE_FTree_ComputeStuffAux (SCE_SForestTreeNode *node)
{
    size_t i;
    SCE_TVector3 u, v, pos;

    /* vector parent -> node */
    SCE_Matrix4x3_GetTranslation (node->parent->matrix, v);
    SCE_Matrix4x3_GetTranslation (node->matrix, pos);
    SCE_Vector3_Operator2v (u, =, pos, -, v);
    node->distance = SCE_Vector3_Length (u);
    node->distance += node->parent->distance;
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
        SCE_FTree_ComputeStuffAux (node->children[i]);
}

static void
SCE_FTree_ComputeStuff (SCE_SForestTreeNode *node)
{
    size_t i;
    SCE_TVector3 x, y, z;

    /* root plane is determined by the direction vector */
    SCE_Matrix4x3_GetBase (node->matrix, x, y, z);
    SCE_Vector3_Copy (node->plane, z);
    SCE_Vector3_Normalize (node->plane);
    node->distance = 0.0;

    for (i = 0; i < node->n_children; i++)
        SCE_FTree_ComputeStuffAux (node->children[i]);
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
#define V_SIZE 8
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
    SCE_Geometry_SetArrayData (&ft->tc, SCE_TEXCOORD0, SCE_FLOAT, 0, 2,
                               &ft->vertices[6], SCE_FALSE);

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
    size_t i;

    SCE_FTree_Count (&ft->root);

    if (ft->root.n_vertices1 >= 256 * 256 ||
        ft->root.n_vertices2 >= 256 * 256) {
        SCEE_Log (875);
        SCEE_LogMsg ("the tree is too big, cannot fit SCEindices");
        return SCE_ERROR;
    }

    SCE_FTree_ComputeStuff (&ft->root);

    if (SCE_FTree_BuildTreeGeom (ft) < 0) goto fail;
    if (SCE_FTree_BuildFinalGeom (ft) < 0) goto fail;

    if (!(ft->vindex = SCE_malloc (ft->root.n_vertices1 * sizeof *ft->vindex)))
        goto fail;

    for (i = 0; i < SCE_MAX_FTREE_BUSH_TYPES; i++) {
        if (ft->root.n_leaves[i] > 0) {
            if (!(ft->bushes[i] = SCE_malloc (ft->root.n_leaves[i] *
                                              sizeof (SCE_TMatrix4x3))))
                goto fail;
        }
    }

    /* fillup vertex arrays */
    SCE_FTree_UpdateTreeGeometry (ft);
    SCE_FTree_UpdateFinalGeometry (ft);
    SCE_FTree_UpdateBushesMatrices (ft);

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
    SCE_TVector3 pos, ppos;     /* node pos, node->parent pos */

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
        if (!node->parent) {
            SCE_Vector3_Operator1v (tmp.plane, -=, pos);
        } else {
            SCE_Matrix4x3_GetTranslation (node->parent->matrix, ppos);
            SCE_Vector3_Operator1v (tmp.plane, -=, ppos);
        }

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
        SCE_TVector3 x, y, z, xnorm, ynorm;

        /* sum up the total number of output vertices to get indices */
        ft->vindex[i] = previous_index;

        SCE_Vector3_Copy (x, &ft->matrix_data[i * 15 + 3]);
        SCE_Vector3_Copy (y, &ft->matrix_data[i * 15 + 6]);
        SCE_Vector3_Copy (xnorm, x);
        SCE_Vector3_Copy (ynorm, y);
        SCE_Vector3_Normalize (xnorm);
        SCE_Vector3_Normalize (ynorm);

        SCE_Vector3_Copy (z, &ft->matrix_data[i * 15 + 12]);
        SCE_Vector3_Normalize (z);
        SCE_Plane_SetFromPointv (&p, z, &ft->matrix_data[i * 15]);
        SCE_Vector3_Copy (z, &ft->matrix_data[i * 15 + 9]);
        SCE_Vector3_Normalize (z);
        SCE_Line3_SetNormal (&l, z);

        for (j = 0; j < ft->npoly_data[i] + 1; j++) {
            SCE_TVector3 pos, posnorm;
            float c, s, radius;
            float *vertices = NULL;
            float angle = 0.0;

            if (j < ft->npoly_data[i])
                angle = M_PI * 2.0 * j / (float)ft->npoly_data[i];

            /* compute position */
            c = cos (angle);
            s = sin (angle);
            radius = ft->drad_data[i * 2];
            SCE_Vector3_Operator2v (pos, = c * radius *, x, + s * radius *, y);
            SCE_Vector3_Copy (posnorm, pos);
            SCE_Vector3_Normalize (posnorm);
            SCE_Vector3_Operator1v (pos, +=, &ft->matrix_data[i * 15]);

            vertices = &ft->vertices[previous_index * V_SIZE];

            SCE_Line3_SetOrigin (&l, pos);
            /* project the position on the node plane */
            SCE_Plane_LineIntersection (&p, &l, vertices);

            /* compute normal: vector from node's position to current vertex */
            SCE_Vector3_Operator2v (&vertices[3], =, vertices, -,
                                    &ft->matrix_data[i * 15]);
            SCE_Vector3_Normalize (&vertices[3]);

            /* compute texture coordinates */
            vertices[7] = ft->drad_data[i * 2 + 1];

            if (j == ft->npoly_data[i])
                vertices[6] = 1.;
            else {
#if 1
                vertices[6] = j / (float)ft->npoly_data[i];
#else
                float dx, dy;

                dx = SCE_Vector3_Dot (posnorm, xnorm);
                dy = SCE_Vector3_Dot (posnorm, ynorm);
                if (dy < 0)
                    vertices[6] = 2.0 * M_PI - acos (dx);
                else
                    vertices[6] = acos (dx);

                vertices[6] /= 2.0 * M_PI; /* scale between 0 and 1 */
                vertices[6] = 0.6 * vertices[6] +
                              0.4 * j / (float)ft->npoly_data[i];
#endif
            }

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

            if (index > n2) index = n2;

            ft->indices[previous_index * 3 + 1] = index1 + j;
            ft->indices[previous_index * 3 + 0] = index2 + index;
            ft->indices[previous_index * 3 + 2] = index1 + j + 1;

            previous_index++;
        }

        /* repeat for the other side */
        for (j = 0; j < n2; j++) {
            /* I STILL know what I'm doing. */
            float offset = ((float)j + 0.5) / n2;
            size_t index = offset * n1 + 0.5;

            if (index > n1) index = n1;

            ft->indices[previous_index * 3 + 0] = index2 + j;
            ft->indices[previous_index * 3 + 1] = index1 + index;
            ft->indices[previous_index * 3 + 2] = index2 + j + 1;

            previous_index++;
        }
    }

    SCE_Geometry_SetNumVertices (&ft->final_geom, ft->root.n_vertices2);
    SCE_Geometry_SetNumIndices (&ft->final_geom, ft->root.n_indices2);
}


static void
SCE_FTree_UpdateBushesMatricesAux (SCE_SForestTree *ft,
                                   SCE_SForestTreeNode *node)
{
    size_t i;

    if (node->leaf_index >= 0) {
        SCE_Matrix4x3_Copy (&ft->bushes[node->leaf_index][ft->index_counter],
                            node->leaf_matrix);
        ft->index_counter += 4 * 3;
    }

    for (i = 0; i < node->n_children; i++)
        SCE_FTree_UpdateBushesMatricesAux (ft, node->children[i]);
}

void SCE_FTree_UpdateBushesMatrices (SCE_SForestTree *ft)
{
    ft->index_counter = 0;
    SCE_FTree_UpdateBushesMatricesAux (ft, &ft->root);
}


SCE_SGeometry* SCE_FTree_GetTreeGeometry (SCE_SForestTree *ft)
{
    return &ft->tree_geom;
}

SCE_SGeometry* SCE_FTree_GetFinalGeometry (SCE_SForestTree *ft)
{
    return &ft->final_geom;
}

/**
 * \brief Gets the number of instances of a type of bush
 * \param ft a tree
 * \param index bush's index
 * \return number of instances of bush of type \p index
 * \sa SCE_FTree_GetBushMatrix()
 */
size_t SCE_FTree_GetBushNumInstances (SCE_SForestTree *ft, SCEuint index)
{
    return ft->root.n_leaves[index];
}
/**
 * \brief Gets the matrix of a bush
 * \param ft a tree
 * \param bush bush index
 * \param offset instance index
 * \returns a pointer to the instance matrix
 * \sa SCE_FTree_GetBushNumInstances()
 */
float* SCE_FTree_GetBushMatrix (SCE_SForestTree *ft, SCEuint bush, SCEuint offset)
{
    return &ft->bushes[bush][offset * 4 * 3];
}
