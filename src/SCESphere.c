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

/* created: 04/08/2009
   updated: 13/08/2009 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCESphere.h"

void SCE_Sphere_Init (SCE_SSphere *s)
{
    SCE_Vector3_Set (s->center, 0.0f, 0.0f, 0.0f);
    s->radius = 1.0f;
}

void SCE_Sphere_Copy (SCE_SSphere *dst, const SCE_SSphere *src)
{
    SCE_Vector3_Copy (dst->center, src->center);
    dst->radius = src->radius;
}
