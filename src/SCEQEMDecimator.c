/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2013  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

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

/* created: 09/03/2013
   updated: 10/03/2013 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"

#include "SCE/core/SCEQEMDecimator.h"

void SCE_QEMD_Init (SCE_SQEMMesh *mesh)
{
    mesh->max_vertices = mesh->max_indices = 0;
    mesh->n_vertices = mesh->n_indices = 0;
    mesh->vertices = NULL;
    mesh->indices = NULL;
}
void SCE_QEMD_Clear (SCE_SQEMMesh *mesh)
{
    SCE_free (mesh->vertices);
    SCE_free (mesh->indices);
}

/* you'd better not give this function 0 */
void SCE_QEMD_SetMaxVertices (SCE_SQEMMesh *mesh, SCEuint n)
{
    mesh->max_vertices = n;
}
void SCE_QEMD_SetMaxIndices (SCE_SQEMMesh *mesh, SCEuint n)
{
    mesh->max_indices = n;
}

int SCE_QEMD_Build (SCE_SQEMMesh *mesh)
{
    SCE_free (mesh->vertices);
    SCE_free (mesh->indices);

    if (!(mesh->vertices = SCE_malloc (mesh->max_vertices *
                                       sizeof *mesh->vertices)))
        goto fail;
    if (!(mesh->indices = SCE_malloc (mesh->max_indices *
                                      sizeof *mesh->indices)))
        goto fail;

    return SCE_OK;
fail:
    SCEE_LogSrc ();
    return SCE_ERROR;
}


static void SCE_QEMD_MakeMatrix (SCE_SPlane *p, SCE_TMatrix4 m)
{
    float a, b, c, d;
    SCE_TVector3 n;

    SCE_Plane_GetNormalv (p, n);
    d = SCE_Plane_GetDistance (p);
    a = n[0]; b = n[1]; c = n[2];
    SCE_Matrix4_Set (m,
                     a*a, a*b, a*c, a*d,
                     b*a, b*b, b*c, b*d,
                     c*a, c*b, c*c, c*d,
                     d*a, d*b, d*c, d*d);
}

static void SCE_QEMD_InitQuadrics (SCE_SQEMMesh *mesh)
{
    size_t i;
    SCE_SPlane p;
    SCE_SQEMVertex *a, *b, *c;
    SCE_TMatrix4 q;
    SCE_TVector3 v;

    for (i = 0; i < mesh->n_vertices; i++)
        SCE_Matrix4_Null (mesh->vertices[i].q);

    /* for each plane */
    for (i = 0; i < mesh->n_indices; i += 3) {
        a = &mesh->vertices[mesh->indices[i]];
        b = &mesh->vertices[mesh->indices[i + 1]];
        c = &mesh->vertices[mesh->indices[i + 2]];

        /* compute quadric matrix */
        SCE_Plane_SetFromTriangle (&p, a->v, b->v, c->v);
        SCE_Plane_GetNormalv (&p, v);
        if (SCE_Vector3_IsNull (v))
            continue;
        SCE_Plane_Normalize (&p, SCE_TRUE);
        SCE_QEMD_MakeMatrix (&p, q);

        /* add it to every vertex */
        SCE_Matrix4_AddCopy (a->q, q);
        SCE_Matrix4_AddCopy (b->q, q);
        SCE_Matrix4_AddCopy (c->q, q);
    }
}

void SCE_QEMD_Set (SCE_SQEMMesh *mesh, const SCEvertices *vertices,
                   const SCEindices *indices, SCEuint n_vertices,
                   SCEuint n_indices)
{
    size_t i;

    mesh->n_vertices = n_vertices;
    mesh->n_indices = n_indices;

    memcpy (mesh->indices, indices, n_indices * sizeof *indices);
    for (i = 0; i < n_vertices; i++) {
        SCE_Vector3_Copy (mesh->vertices[i].v, &vertices[i * 3]);
        mesh->vertices[i].index = -1;
    }

    SCE_QEMD_InitQuadrics (mesh);
}

void SCE_QEMD_AnchorVertices (SCE_SQEMMesh *mesh, const SCEindices *indices,
                              SCEuint n)
{
    SCEuint i, j;
    SCE_SQEMVertex *v;

    for (i = 0; i < n; i++) {
        v = &mesh->vertices[indices[i]];
        for (j = 0; j < 16; j++)
            v->q[j] = 10000.0;  /* derp */
    }
}


static SCEuint SCE_QEMD_SolveVertexList (SCE_SQEMMesh *mesh, SCEuint v)
{
    SCE_SQEMVertex *vertex = NULL;
    SCEuint last = v;

    vertex = &mesh->vertices[v];
    if (vertex->index > -1) {

        /* get last index */
        do {
            last = vertex->index;
            vertex = &mesh->vertices[vertex->index];
        } while (vertex->index > -1);

        /* replace indices of the vertices with last index */
        vertex = &mesh->vertices[v];
        while (vertex->index > -1) {
            long next = vertex->index;
            vertex->index = last;
            vertex = &mesh->vertices[next];
        }
    }

    return last;
}

void SCE_QEMD_Get (SCE_SQEMMesh *mesh, SCEvertices *vertices,
                   SCEindices *indices, SCEuint *n_vertices, SCEuint *n_indices)
{
    size_t i, index;
    SCE_SQEMVertex *v = NULL;
    SCEuint i1, i2, i3;

    /* vertices output is pretty straightforward, we just reassign them
       a new index */
    index = 0;
    for (i = 0; i < mesh->n_vertices; i++) {
        v = &mesh->vertices[i];
        if (v->index == -1) {
            SCE_Vector3_Copy (&vertices[index * 3], v->v);
            v->final = index;
            index++;
        }
    }
    *n_vertices = index;

    /* solve vertex lists */
    for (i = 0; i < mesh->n_vertices; i++) {
        index = SCE_QEMD_SolveVertexList (mesh, i);
        mesh->vertices[i].final = mesh->vertices[index].final;
    }

    /* TODO: we dont need to check for removed triangles if we
       already removed them (via FixInversion for example) */
    index = 0;
    for (i = 0; i < mesh->n_indices; i += 3) {
        i1 = mesh->vertices[mesh->indices[i]].final;
        i2 = mesh->vertices[mesh->indices[i + 1]].final;
        i3 = mesh->vertices[mesh->indices[i + 2]].final;
        if (i1 != i2 && i2 != i3 && i1 != i3) {
            indices[index + 0] = i1;
            indices[index + 1] = i2;
            indices[index + 2] = i3;
            index += 3;
        }
    }
    *n_indices = index;
}


/* TODO: wtf there's still no function to do that in the sce? */
static SCEuint myrand (SCEuint inf, SCEuint sup)
{
    float n = random ();
    n /= RAND_MAX;
    n = n * (sup - inf) + inf;
    return (SCEuint)(n + 0.5);
}

typedef struct edge Edge;
struct edge {
    SCEuint index;
    SCEindices v1, v2;
    float error;
    SCE_TVector3 v;
    SCE_TMatrix4 q;
};

static void SCE_QEMD_PickCandidates (SCE_SQEMMesh *mesh, SCEuint n,
                                     Edge *candidates)
{
    SCEuint step = mesh->n_indices / (3 * n);
    SCEuint m = n, i;

    while (n--) {
        /* TODO: stupid trick to avoid duplicates */
        SCEuint r = myrand (step * n, step * (n + 1) - 1);

        candidates[n].index = r * 3;
        /* NOTE: this switch is awful */
        switch (myrand (0, 2)) {
        case 0:
            candidates[n].v1 = mesh->indices[r * 3];
            candidates[n].v2 = mesh->indices[r * 3 + 1];
            break;
        case 1:
            candidates[n].v1 = mesh->indices[r * 3 + 1];
            candidates[n].v2 = mesh->indices[r * 3 + 2];
            break;
        case 2:
            candidates[n].v1 = mesh->indices[r * 3];
            candidates[n].v2 = mesh->indices[r * 3 + 2];
            break;
        default:;
        }
    }
}

static float SCE_QEMD_VertexError (const SCE_TMatrix4 m, const SCE_TVector3 v)
{
    SCE_TVector4 a, b;
    SCE_Vector4_Set (a, v[0], v[1], v[2], 1.0);
    SCE_Matrix4_MulV4 (m, a, b);
    return SCE_Math_Fabsf (SCE_Vector4_Dot (a, b));
}

static void SCE_QEMD_ComputeError (SCE_SQEMMesh *mesh, Edge *edge)
{
    SCE_TVector3 d;
    SCE_TMatrix4 m;

    SCE_Matrix4_Add (mesh->vertices[edge->v1].q, mesh->vertices[edge->v2].q,
                     edge->q);

    /* compute least error vertex position */
    if (SCE_Matrix4_Inverse (edge->q, m) && m[15] > SCE_EPSILONF) {
        SCE_Matrix4_GetTranslation (m, edge->v);
        SCE_Vector3_Operator1 (edge->v, /=, m[15]);
    } else {
        SCE_Vector3_Operator2v (edge->v, = 0.5 *, mesh->vertices[edge->v1].v,
                                + 0.5 *, mesh->vertices[edge->v2].v);
        /* TODO: choose between v1, v2 and (v1 + v2) / 2 */
    }

    edge->error = SCE_QEMD_VertexError (edge->q, edge->v);
    SCE_Vector3_Operator2v (d, =, mesh->vertices[edge->v1].v, -,
                            mesh->vertices[edge->v2].v);
    edge->error *= SCE_Vector3_Dot (d, d);
}

static void SCE_QEMD_RemoveTriangle (SCE_SQEMMesh *mesh, SCEuint index)
{
    mesh->indices[index]     = mesh->indices[mesh->n_indices - 3];
    mesh->indices[index + 1] = mesh->indices[mesh->n_indices - 2];
    mesh->indices[index + 2] = mesh->indices[mesh->n_indices - 1];
    mesh->n_indices -= 3;
}

static void SCE_QEMD_CollapseEdge (SCE_SQEMMesh *mesh, Edge *edge)
{
    /* merge vertices by linking them */
    mesh->vertices[edge->v2].index = edge->v1;
    /* copy data */
    SCE_Vector3_Copy (mesh->vertices[edge->v1].v, edge->v);
    SCE_Matrix4_Copy (mesh->vertices[edge->v1].q, edge->q);

    /* invalidate triangle in the index list */
    SCE_QEMD_RemoveTriangle (mesh, edge->index);
}


static void SCE_QEMD_CollapseLeastErrorEdge (SCE_SQEMMesh *mesh, SCEuint n,
                                             Edge *edges)
{
    SCEuint i;
    Edge *best = NULL;
    float error;

    /* get the least error edge */
    best = &edges[0];
    error = best->error;
    for (i = 1; i < n; i++) {
        if (edges[i].error < error) {
            best = &edges[i];
            error = best->error;
        }
    }

    /* collapse the edge */
    SCE_QEMD_CollapseEdge (mesh, best);
}


#define SCE_QEMD_NUM_CANDIDATES 10

/**
 * \brief Decimate the mesh
 * \param mesh a QEM mesh
 * \param n number of edge contractions to perform
 */
void SCE_QEMD_Process (SCE_SQEMMesh *mesh, SCEuint n)
{
    int i, n_candidates;
    Edge candidates[SCE_QEMD_NUM_CANDIDATES];

    while (n) {
        /* pick random edges */
        n_candidates = SCE_QEMD_NUM_CANDIDATES;
        SCE_QEMD_PickCandidates (mesh, n_candidates, candidates);

        /* solve vertex list */
        for (i = 0; i < n_candidates; i++) {
            candidates[i].v1 = SCE_QEMD_SolveVertexList (mesh,candidates[i].v1);
            candidates[i].v2 = SCE_QEMD_SolveVertexList (mesh,candidates[i].v2);
            if (candidates[i].v1 == candidates[i].v2) {
                /* edge was already merged, remove triangle */
                SCE_QEMD_RemoveTriangle (mesh, candidates[i].index);
                n_candidates--;
                candidates[i].v1 = candidates[n_candidates].v1;
                candidates[i].v2 = candidates[n_candidates].v2;
                candidates[i].index = candidates[n_candidates].index;
                i--;
            }
        }

#define THRESHOLD (SCE_QEMD_NUM_CANDIDATES / 2)
        if (n_candidates < THRESHOLD)
            continue;

        /* compute their cost */
        for (i = 0; i < n_candidates; i++)
            SCE_QEMD_ComputeError (mesh, &candidates[i]);

        SCE_QEMD_CollapseLeastErrorEdge (mesh, n_candidates, candidates);

        n--;
    }
}

void SCE_QEMD_FixInversion (SCE_SQEMMesh *mesh, const SCEvertices *vertices)
{
    long i;
    SCEuint i1, i2, i3;
    SCE_TVector3 n1, n2, s1, s2;

    /* for each face */
    for (i = 0; i < mesh->n_indices; i += 3) {
        /* fetch modified indices */
        i1 = SCE_QEMD_SolveVertexList (mesh, mesh->indices[i]);
        i2 = SCE_QEMD_SolveVertexList (mesh, mesh->indices[i + 1]);
        i3 = SCE_QEMD_SolveVertexList (mesh, mesh->indices[i + 2]);
        /* ignore removed trianles */
        /* NOTE: remove the triangle to spare Get() some work */
        if (i1 == i2 || i2 == i3 || i1 == i3)
            continue;

        /* get original normal */
        SCE_Vector3_Operator2v (s1, =, &vertices[3 * mesh->indices[i + 1]],
                                    -, &vertices[3 * mesh->indices[i]]);
        SCE_Vector3_Operator2v (s2, =, &vertices[3 * mesh->indices[i + 2]],
                                    -, &vertices[3 * mesh->indices[i]]);
        SCE_Vector3_Cross (n1, s1, s2);
        if (!SCE_Vector3_IsNull (n1))
            SCE_Vector3_Normalize (n1);

        /* get modified normal */
        SCE_Vector3_Operator2v (s1, =, mesh->vertices[i2].v,
                                    -, mesh->vertices[i1].v);
        SCE_Vector3_Operator2v (s2, =, mesh->vertices[i3].v,
                                    -, mesh->vertices[i1].v);
        SCE_Vector3_Cross (n2, s1, s2);
        if (!SCE_Vector3_IsNull (n2))
            SCE_Vector3_Normalize (n2);

        /* compare */
        if (SCE_Vector3_Dot (n1, n2) < 0.2 || SCE_Vector3_IsNull (n2) ||
            SCE_Vector3_IsNull (n1)) {

            /* normal has been flipped, remove triangle */
            Edge edges[3];

            /* make 3 edges of the triangle */
            edges[0].v1 = i1;
            edges[0].v2 = i2;
            edges[0].index = i;
            edges[1].v1 = i2;
            edges[1].v2 = i3;
            edges[1].index = i;
            edges[2].v1 = i1;
            edges[2].v2 = i3;
            edges[2].index = i;

            /* compute their cost */
            for (i = 0; i < 3; i++)
                SCE_QEMD_ComputeError (mesh, &edges[i]);

            /* collapse one of them */
            SCE_QEMD_CollapseLeastErrorEdge (mesh, 3, edges);

            /* triangle has been replaced */
            i -= 3;
        }
    }
}
