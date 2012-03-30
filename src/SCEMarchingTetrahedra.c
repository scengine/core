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

/* created: 28/03/2012
   updated: 30/03/2012 */

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCEGeometry.h"
#include "SCE/core/SCEMarchingTetrahedra.h"


/* wa and wb must be between -1 and 1 */
static void vlerp (SCE_TVector3 v, const SCE_TVector3 a, const SCE_TVector3 b,
                   const float wa, const float wb)
{
    float w = -wa / (wb - wa);
    SCE_Vector3_Operator2 (v, =, a, *, 1.0 - w);
    SCE_Vector3_Operator2 (v, +=, b, *, w);
}

/*                + 0
                 /|\
                / | \
               /  |  \
              1   2   0
             /    |    \
            /     |     \
           +---4--|------+ 1
          3 \     |     /
             \    |    /
              5   |   3
               \  |  /
                \ | /
                 \|/
                  + 2
*/


/* dont call this function with code == 0 or code == 15 :) */
static size_t SCE_MT_Tetrahedron (SCEvertices *v, const int code,
                                  const float d[4], const SCE_TVector3 p[4])
{
#define N_CASES (16)

    /* TODO: same table as in interface/SCEVoxelRenderer */

    /* number of polygons, 0 indicates one and 1 indicates two */
    const unsigned char lt_triangles_count[N_CASES] = {
        0, 0, 0, 1, 0, 1, 1, 0, 0, 1,
        1, 0, 1, 0, 0, 0
    };

    /* frontfacing triangles are clockwise */
    const int lt_triangles[N_CASES * 4] = {
                                    // 3210 vertex index
        0, 0, 0, 0,                 // 0000
        0, 1, 2, 0,                 // 0001
        0, 3, 4, 0,                 // 0010
        1, 2, 3, 4,                 // 0011
        2, 5, 3, 0,                 // 0100
        0, 1, 5, 3,                 // 0101
        0, 2, 5, 4,                 // 0110
        1, 5, 4, 0,                 // 0111
        1, 4, 5, 0,                 // 1000
        0, 4, 5, 2,                 // 1001
        0, 3, 5, 1,                 // 1010
        2, 3, 5, 0,                 // 1011
        1, 4, 3, 2,                 // 1100
        0, 4, 3, 0,                 // 1101
        0, 2, 1, 0,                 // 1110
        0, 0, 0, 0                  // 1111
    };

    /* given the ID of an edge, returns the two corresponding vertices */
    const int lt_edges[6 * 2] = {
        0, 1,
        0, 3,
        0, 2,
        1, 2,
        1, 3,
        2, 3
    };

    int i, index, v1, v2;
    size_t offset = 0;

#if 0
    for (i = 0; i < 3; i++, offset++) {
        index = lt_triangles[code * 4 + i];
        v1 = lt_edges[index * 2];
        v2 = lt_edges[index * 2 + 1];
        vlerp (&v[offset * 3], p[v1], p[v2], d[v1], d[v2]);
    }

    if (lt_triangles_count[code]) {
        SCE_Vector3_Copy (&v[offset * 3], &v[(offset - 3) * 3]);
        offset++;
        for (i = 2; i < 4; i++, offset++) {
            index = lt_triangles[code * 4 + i];
            v1 = lt_edges[index * 2];
            v2 = lt_edges[index * 2 + 1];
            vlerp (&v[offset * 3], p[v1], p[v2], d[v1], d[v2]);
        }
    }
#else
    /* counter-clockwise (wtf ?) */
    for (i = 2; i >= 0; i--, offset++) {
        index = lt_triangles[code * 4 + i];
        v1 = lt_edges[index * 2];
        v2 = lt_edges[index * 2 + 1];
        vlerp (&v[offset * 3], p[v1], p[v2], d[v1], d[v2]);
    }

    if (lt_triangles_count[code]) {
        SCE_Vector3_Copy (&v[offset * 3], &v[(offset - 1) * 3]);
        offset++;
        for (i = 3; i >= 2; i--, offset++) {
            index = lt_triangles[code * 4 + i];
            v1 = lt_edges[index * 2];
            v2 = lt_edges[index * 2 + 1];
            vlerp (&v[offset * 3], p[v1], p[v2], d[v1], d[v2]);
        }
    }
#endif

    return offset;
}

/*
 *  4_____________5
 *  |\           |\
 *  | \          | \
 *  |  \         |  \
 *  |   \        |   \
 *  |    \3___________\2
 *  |    |            |         z   y
 *  |7__ | ______|6   |          \ |
 *   \   |        \   |           \|__ x
 *    \  |         \  |
 *     \ |          \ |
 *      \|___________\|
 *       0            1
 */

size_t SCE_MT_GenerateCell (SCEvertices v[3 * 36], const float densities[8],
                            const SCE_TVector3 origin)
{
    /* TODO: same table as in interface/SCEVoxelRenderer */
    const int lt_vertices[6 * 4] = {
        3, 6, 7, 0,
        3, 4, 7, 6,
        3, 5, 4, 6,
        3, 2, 5, 6,
        3, 1, 2, 6,
        3, 0, 1, 6
    };

    unsigned int in;
    size_t offset = 0;
    int code;
    int i;
    int p0, p1, p2, p3;         /* indices of tetrahedron's vertices */
    SCE_TVector3 points[8], t_points[4];
    float t_densities[4];

    /* construct points */
    for (i = 0; i < 8; i++)
        SCE_Vector3_Operator1v (points[i], =, origin);
    points[1][0] += 1.0;
    points[2][0] += 1.0;
    points[2][1] += 1.0;
    points[3][1] += 1.0;
    points[4][1] += 1.0;
    points[4][2] += 1.0;
    points[5][0] += 1.0;
    points[5][1] += 1.0;
    points[5][2] += 1.0;
    points[6][0] += 1.0;
    points[6][2] += 1.0;
    points[7][2] += 1.0;

    /* cell case */
    in = 0;
    for (i = 0; i < 8; i++)
        in |= (densities[i] > 0.0) << i;

    /* for each tetrahedron */
    for (i = 0; i < 6; i++) {
        p0 = lt_vertices[i * 4 + 0];
        p1 = lt_vertices[i * 4 + 1];
        p2 = lt_vertices[i * 4 + 2];
        p3 = lt_vertices[i * 4 + 3];

        code  = (1 & (in >> p0));
        code |= (1 & (in >> p1)) << 1;
        code |= (1 & (in >> p2)) << 2;
        code |= (1 & (in >> p3)) << 3;

        if (code > 0 && code < 15) {
            SCE_Vector3_Copy (t_points[0], points[p0]);
            SCE_Vector3_Copy (t_points[1], points[p1]);
            SCE_Vector3_Copy (t_points[2], points[p2]);
            SCE_Vector3_Copy (t_points[3], points[p3]);
            t_densities[0] = densities[p0];
            t_densities[1] = densities[p1];
            t_densities[2] = densities[p2];
            t_densities[3] = densities[p3];

            offset += SCE_MT_Tetrahedron (&v[offset * 3], code,
                                          t_densities, t_points);
        }
    }

    return offset;
}



/**
 * \brief 
 * 
 * \param vertices 
 * \param normals 
 * \param voxels 
 * \param region 
 * \param w 
 * \param h 
 * \param d 
 */
size_t SCE_MT_Generate (SCEvertices *vertices, const unsigned char *voxels,
                        const SCE_SIntRect3 *region,
                        SCEuint w, SCEuint h, SCEuint d)
{
    SCE_TVector3 origin;
    float densities[8];
    SCEuint x, y, z, i;
    size_t offset = 0;
    int p1[3], p2[3];

#define getoffset(x_, y_, z_) ((w) * ((h) * (z_) + (y_)) + (x_))

    SCE_Rectangle3_GetPointsv (region, p1, p2);

    for (z = p1[2]; z < p2[2]; z++) {
        origin[2] = z;
        for (y = p1[1]; y < p2[1]; y++) {
            origin[1] = y;
            for (x = p1[0]; x < p2[0]; x++) {
                origin[0] = x;

                densities[0] = voxels[getoffset(x,     y,     z    )];
                densities[1] = voxels[getoffset(x + 1, y,     z    )];
                densities[2] = voxels[getoffset(x + 1, y + 1, z    )];
                densities[3] = voxels[getoffset(x,     y + 1, z    )];
                densities[4] = voxels[getoffset(x,     y + 1, z + 1)];
                densities[5] = voxels[getoffset(x + 1, y + 1, z + 1)];
                densities[6] = voxels[getoffset(x + 1, y,     z + 1)];
                densities[7] = voxels[getoffset(x,     y,     z + 1)];

                for (i = 0; i < 8; i++)
                    densities[i] = densities[i] / 128.0 - 1.0;

                offset += SCE_MT_GenerateCell (&vertices[offset * 3],
                                               densities, origin);
            }
        }
    }

#undef getoffset
    return offset;
}


/* samples voxels with linear interpolation */
static float sample (const unsigned char *voxels, SCEuint dim[3],
                     const SCE_TVector3 coord,
                     float addx, float addy, float addz)
{
    SCE_TVector3 p;
    size_t x, y, z;
    float wx, wy, wz;
    float a, b, c, d;
    float x1, y1, z1, z2, final;

#define lerp(a, b, w) ((a) * (1.0 - (w)) + (b) * (w))
#define floatify(c) ((c) / 256.0)
#define getof(x, y, z) ((dim[0]) * ((dim[1]) * (z) + (y)) + (x))

    p[0] = coord[0] + addx;
    p[1] = coord[1] + addy;
    p[2] = coord[2] + addz;

    wx = SCE_Math_Fractf (p[0]);
    wy = SCE_Math_Fractf (p[1]);
    wz = SCE_Math_Fractf (p[2]);

    x = p[0];
    y = p[1];
    z = p[2];

    a = floatify (voxels[getof (x,     y,     z)]);
    b = floatify (voxels[getof (x + 1, y,     z)]);
    c = floatify (voxels[getof (x,     y + 1, z)]);
    d = floatify (voxels[getof (x + 1, y + 1, z)]);
    x1 = lerp (a, b, wx);
    y1 = lerp (c, d, wx);
    z1 = lerp (x1, y1, wy);

    a = floatify (voxels[getof (x,     y,     z + 1)]);
    b = floatify (voxels[getof (x + 1, y,     z + 1)]);
    c = floatify (voxels[getof (x,     y + 1, z + 1)]);
    d = floatify (voxels[getof (x + 1, y + 1, z + 1)]);
    x1 = lerp (a, b, wx);
    y1 = lerp (c, d, wx);
    z2 = lerp (x1, y1, wy);

    final = lerp (z1, z2, wz);

    return final;
}

void SCE_MT_GenerateNormals (SCEvertices *normals, const SCEvertices *vertices,
                             size_t n_vertices,
                             const unsigned char *voxels,
                             SCEuint w, SCEuint h, SCEuint d)
{
    SCE_TVector3 grad;
    const SCEvertices *p = NULL;
    SCEuint dim[3];
    SCEuint i;

    dim[0] = w;
    dim[1] = h;
    dim[2] = d;

    for (i = 0; i < n_vertices; i++) {
        p = &vertices[i * 3];
        grad[0] = sample (voxels, dim, p,  1.0,  0.0,  0.0)
                - sample (voxels, dim, p, -1.0,  0.0,  0.0);
        grad[1] = sample (voxels, dim, p,  0.0,  1.0,  0.0)
                - sample (voxels, dim, p,  0.0, -1.0,  0.0);
        grad[2] = sample (voxels, dim, p,  0.0,  0.0,  1.0)
                - sample (voxels, dim, p,  0.0,  0.0, -1.0);
        SCE_Vector3_Normalize (grad);
        SCE_Vector3_Operator1 (grad, *=, -1.0);
        SCE_Vector3_Copy (&normals[i * 3], grad);
    }
}
