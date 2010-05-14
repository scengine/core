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
 
/* created: 02/11/2008
   updated: 21/06/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCELevelOfDetail.h"

static int SCE_Lod_DefaultGetLodFunc (float);

void SCE_Lod_Init (SCE_SLevelOfDetail *lod)
{
    lod->lod = 0;
    lod->box = NULL;
    lod->size = 0.0f;
    lod->dist = 0.0f;
    lod->getlod = SCE_Lod_DefaultGetLodFunc;
}

SCE_SLevelOfDetail* SCE_Lod_Create (void)
{
    SCE_SLevelOfDetail *lod = NULL;
    lod = SCE_malloc (sizeof *lod);
    if (!lod)
        SCEE_LogSrc();
    else
        SCE_Lod_Init (lod);
    return lod;
}

void SCE_Lod_Delete (SCE_SLevelOfDetail *lod)
{
    SCE_free (lod);
}


void SCE_Lod_SetBoundingBox (SCE_SLevelOfDetail *lod, SCE_SBoundingBox *box)
{
    lod->box = box;
}

void SCE_Lod_SetGetLODFunc (SCE_SLevelOfDetail *lod, SCE_FGetLODFunc f)
{
    if (f)
        lod->getlod = f;
    else
        lod->getlod = SCE_Lod_DefaultGetLodFunc;
}


SCE_SBoundingBox* SCE_Lod_GetBoundingBox (SCE_SLevelOfDetail *lod)
{
    return lod->box;
}


float SCE_Lod_ComputeBoundingBoxSurfaceFromDist (SCE_SBox *box,
                                                 float dist,
                                                 SCE_SCamera *cam)
{
    SCE_TVector4 v[4];
    float area;

    {
        /* see SCEBox.c for infos about the queried points */
        float *points;
        points = SCE_Box_GetPoints (box);
        SCE_Vector3_Copy (v[0], &points[0]);
        SCE_Vector3_Copy (v[1], &points[2]);
        SCE_Vector3_Copy (v[2], &points[4]);
        SCE_Vector3_Copy (v[3], &points[6]);

        v[0][2] -= dist;
        v[1][2] -= dist;
        v[2][2] -= dist;
        v[3][2] -= dist;
        v[0][3] = v[1][3] = v[2][3] = v[3][3] = 1.0f;
    }
    {
        /* project points */
        float *proj;
        proj = SCE_Camera_GetProj (cam);
        SCE_Matrix4_MulV4Copy (proj, v[0]);
        SCE_Matrix4_MulV4Copy (proj, v[1]);
        SCE_Matrix4_MulV4Copy (proj, v[2]);
        SCE_Matrix4_MulV4Copy (proj, v[3]);
    }
    {
        /* depth useless */
        SCE_Vector2_Operator1 (v[0], /=, v[0][3]);
        SCE_Vector2_Operator1 (v[1], /=, v[1][3]);
        SCE_Vector2_Operator1 (v[2], /=, v[2][3]);
        SCE_Vector2_Operator1 (v[3], /=, v[3][3]);
    }
    {
        /* make the rectangle */
        SCE_SFloatRect r;
        unsigned int i;
        r.p1[0] = r.p2[0] = v[0][0];
        r.p1[1] = r.p2[1] = v[0][1];
        for (i = 1; i < 4; i++)
        {
            if (v[i][0] < r.p1[0])
                r.p1[0] = v[i][0];
            if (v[i][0] > r.p2[0])
                r.p2[0] = v[i][0];

            if (v[i][1] < r.p1[1])
                r.p1[1] = v[i][1];
            if (v[i][1] > r.p2[1])
                r.p2[1] = v[i][1];
        }
        area = SCE_Rectangle_GetAreaf (&r);
    }
    return area;
}

float SCE_Lod_ComputeBoundingBoxSurface (SCE_SBox *box, SCE_SCamera *cam)
{
    float area;
    float dist = 1.0f;
    SCE_TVector3 t;
    SCE_Box_GetCenterv (box, t);
    {
        SCE_TVector3 v1;
        SCE_Camera_GetPositionv (cam, v1);
        dist = SCE_Vector3_Distance (v1, t);
    }
    SCE_Box_SetCenter (box, 0.0f, 0.0f, 0.0f);
    area = SCE_Lod_ComputeBoundingBoxSurfaceFromDist (box, dist, cam);
    SCE_Box_SetCenterv (box, t);
    return area;
}

static int SCE_Lod_DefaultGetLodFunc (float size)
{
    /* FIXME: WTF!? */
    return (unsigned int)(0.4f/SCE_Math_Sqrt (size));
}

int SCE_Lod_Compute (SCE_SLevelOfDetail *lod, SCE_TMatrix4 m, SCE_SCamera *cam)
{
    SCE_TVector3 t;
    {
        SCE_TVector4 v;
        SCE_Camera_GetPositionv (cam, v);
        SCE_Matrix4_GetTranslation (m, t);
        lod->dist = SCE_Vector3_Distance (v, t);
    }
    m[3] = m[7] = m[11] = 0.0f;
    {
        SCE_SBox box, *bptr;
        bptr = SCE_BoundingBox_GetBox (lod->box);
        /* do projection! */
        SCE_BoundingBox_Push (lod->box, m, &box);
        lod->size = SCE_Lod_ComputeBoundingBoxSurfaceFromDist (bptr,
                                                               lod->dist, cam);
        SCE_BoundingBox_Pop (lod->box, &box);
    }
    /* restore initial translation */
    m[3] = t[0]; m[7] = t[1]; m[11] = t[2];

    lod->lod = lod->getlod (lod->size);
    return lod->lod;
}

int SCE_Lod_GetLOD (SCE_SLevelOfDetail *lod)
{
    return lod->lod;
}

float SCE_Lod_GetSize (SCE_SLevelOfDetail *lod)
{
    return lod->size;
}

float SCE_Lod_GetDistance (SCE_SLevelOfDetail *lod)
{
    return lod->dist;
}
