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

/* created: 07/03/2013
   updated: 13/03/2013 */

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/SCEGeometry.h"
#include "SCE/core/SCEGrid.h"

#include "SCE/core/SCEMarchingCube.h"

/* lookup tables (achtung) */
static const SCEubyte lt_num_tri[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 3, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,
    2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2, 1, 2, 2, 3, 2, 3, 3, 4,
    2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4,
    2, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 5, 4, 5, 3, 2, 3, 4, 4, 3, 4, 5, 3, 2,
    4, 5, 5, 4, 5, 2, 4, 1, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3, 4, 3, 5, 2, 2, 3, 3, 4, 3, 4, 4, 5,
    3, 4, 4, 5, 4, 5, 5, 4, 3, 4, 4, 3, 4, 5, 5, 4, 4, 3, 5, 2, 5, 4, 2, 1,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2, 3, 4, 4, 5, 4, 5, 5, 2,
    4, 3, 5, 4, 3, 2, 4, 1, 3, 4, 4, 5, 4, 5, 3, 4, 4, 5, 5, 2, 3, 4, 2, 1,
    2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0
};

static const SCEubyte lt_edges[256 * 15] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 8, 3, 9, 8, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 8, 3, 1, 2, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 2, 10, 0, 2, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 8, 3, 2, 10, 8, 10, 9, 8, 0, 0, 0, 0, 0, 0,
    3, 11, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 11, 2, 8, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 9, 0, 2, 3, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 11, 2, 1, 9, 11, 9, 8, 11, 0, 0, 0, 0, 0, 0,
    3, 10, 1, 11, 10, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 10, 1, 0, 8, 10, 8, 11, 10, 0, 0, 0, 0, 0, 0,
    3, 9, 0, 3, 11, 9, 11, 10, 9, 0, 0, 0, 0, 0, 0,
    9, 8, 10, 10, 8, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 3, 0, 7, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 9, 8, 4, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 1, 9, 4, 7, 1, 7, 3, 1, 0, 0, 0, 0, 0, 0,
    1, 2, 10, 8, 4, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 4, 7, 3, 0, 4, 1, 2, 10, 0, 0, 0, 0, 0, 0,
    9, 2, 10, 9, 0, 2, 8, 4, 7, 0, 0, 0, 0, 0, 0,
    2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, 0, 0, 0,
    8, 4, 7, 3, 11, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    11, 4, 7, 11, 2, 4, 2, 0, 4, 0, 0, 0, 0, 0, 0,
    9, 0, 1, 8, 4, 7, 2, 3, 11, 0, 0, 0, 0, 0, 0,
    4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, 0, 0, 0,
    3, 10, 1, 3, 11, 10, 7, 8, 4, 0, 0, 0, 0, 0, 0,
    1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, 0, 0, 0,
    4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, 0, 0, 0,
    4, 7, 11, 4, 11, 9, 9, 11, 10, 0, 0, 0, 0, 0, 0,
    9, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 5, 4, 0, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 5, 4, 1, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 5, 4, 8, 3, 5, 3, 1, 5, 0, 0, 0, 0, 0, 0,
    1, 2, 10, 9, 5, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 8, 1, 2, 10, 4, 9, 5, 0, 0, 0, 0, 0, 0,
    5, 2, 10, 5, 4, 2, 4, 0, 2, 0, 0, 0, 0, 0, 0,
    2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, 0, 0, 0,
    9, 5, 4, 2, 3, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 11, 2, 0, 8, 11, 4, 9, 5, 0, 0, 0, 0, 0, 0,
    0, 5, 4, 0, 1, 5, 2, 3, 11, 0, 0, 0, 0, 0, 0,
    2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, 0, 0, 0,
    10, 3, 11, 10, 1, 3, 9, 5, 4, 0, 0, 0, 0, 0, 0,
    4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, 0, 0, 0,
    5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, 0, 0, 0,
    5, 4, 8, 5, 8, 10, 10, 8, 11, 0, 0, 0, 0, 0, 0,
    9, 7, 8, 5, 7, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 3, 0, 9, 5, 3, 5, 7, 3, 0, 0, 0, 0, 0, 0,
    0, 7, 8, 0, 1, 7, 1, 5, 7, 0, 0, 0, 0, 0, 0,
    1, 5, 3, 3, 5, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 7, 8, 9, 5, 7, 10, 1, 2, 0, 0, 0, 0, 0, 0,
    10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, 0, 0, 0,
    8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, 0, 0, 0,
    2, 10, 5, 2, 5, 3, 3, 5, 7, 0, 0, 0, 0, 0, 0,
    7, 9, 5, 7, 8, 9, 3, 11, 2, 0, 0, 0, 0, 0, 0,
    9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, 0, 0, 0,
    2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, 0, 0, 0,
    11, 2, 1, 11, 1, 7, 7, 1, 5, 0, 0, 0, 0, 0, 0,
    9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, 0, 0, 0,
    5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0,
    11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0,
    11, 10, 5, 7, 11, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    10, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 8, 3, 5, 10, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 0, 1, 5, 10, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 8, 3, 1, 9, 8, 5, 10, 6, 0, 0, 0, 0, 0, 0,
    1, 6, 5, 2, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 6, 5, 1, 2, 6, 3, 0, 8, 0, 0, 0, 0, 0, 0,
    9, 6, 5, 9, 0, 6, 0, 2, 6, 0, 0, 0, 0, 0, 0,
    5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, 0, 0, 0,
    2, 3, 11, 10, 6, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    11, 0, 8, 11, 2, 0, 10, 6, 5, 0, 0, 0, 0, 0, 0,
    0, 1, 9, 2, 3, 11, 5, 10, 6, 0, 0, 0, 0, 0, 0,
    5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, 0, 0, 0,
    6, 3, 11, 6, 5, 3, 5, 1, 3, 0, 0, 0, 0, 0, 0,
    0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, 0, 0, 0,
    3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, 0, 0, 0,
    6, 5, 9, 6, 9, 11, 11, 9, 8, 0, 0, 0, 0, 0, 0,
    5, 10, 6, 4, 7, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 3, 0, 4, 7, 3, 6, 5, 10, 0, 0, 0, 0, 0, 0,
    1, 9, 0, 5, 10, 6, 8, 4, 7, 0, 0, 0, 0, 0, 0,
    10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, 0, 0, 0,
    6, 1, 2, 6, 5, 1, 4, 7, 8, 0, 0, 0, 0, 0, 0,
    1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, 0, 0, 0,
    8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, 0, 0, 0,
    7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9,
    3, 11, 2, 7, 8, 4, 10, 6, 5, 0, 0, 0, 0, 0, 0,
    5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, 0, 0, 0,
    0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, 0, 0, 0,
    9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6,
    8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, 0, 0, 0,
    5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11,
    0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7,
    6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, 0, 0, 0,
    10, 4, 9, 6, 4, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 10, 6, 4, 9, 10, 0, 8, 3, 0, 0, 0, 0, 0, 0,
    10, 0, 1, 10, 6, 0, 6, 4, 0, 0, 0, 0, 0, 0, 0,
    8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, 0, 0, 0,
    1, 4, 9, 1, 2, 4, 2, 6, 4, 0, 0, 0, 0, 0, 0,
    3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, 0, 0, 0,
    0, 2, 4, 4, 2, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 3, 2, 8, 2, 4, 4, 2, 6, 0, 0, 0, 0, 0, 0,
    10, 4, 9, 10, 6, 4, 11, 2, 3, 0, 0, 0, 0, 0, 0,
    0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, 0, 0, 0,
    3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, 0, 0, 0,
    6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1,
    9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, 0, 0, 0,
    8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1,
    3, 11, 6, 3, 6, 0, 0, 6, 4, 0, 0, 0, 0, 0, 0,
    6, 4, 8, 11, 6, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7, 10, 6, 7, 8, 10, 8, 9, 10, 0, 0, 0, 0, 0, 0,
    0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, 0, 0, 0,
    10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, 0, 0, 0,
    10, 6, 7, 10, 7, 1, 1, 7, 3, 0, 0, 0, 0, 0, 0,
    1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, 0, 0, 0,
    2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9,
    7, 8, 0, 7, 0, 6, 6, 0, 2, 0, 0, 0, 0, 0, 0,
    7, 3, 2, 6, 7, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, 0, 0, 0,
    2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7,
    1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11,
    11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, 0, 0, 0,
    8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6,
    0, 9, 1, 11, 6, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, 0, 0, 0,
    7, 11, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7, 6, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 8, 11, 7, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 9, 11, 7, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 1, 9, 8, 3, 1, 11, 7, 6, 0, 0, 0, 0, 0, 0,
    10, 1, 2, 6, 11, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 10, 3, 0, 8, 6, 11, 7, 0, 0, 0, 0, 0, 0,
    2, 9, 0, 2, 10, 9, 6, 11, 7, 0, 0, 0, 0, 0, 0,
    6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, 0, 0, 0,
    7, 2, 3, 6, 2, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7, 0, 8, 7, 6, 0, 6, 2, 0, 0, 0, 0, 0, 0, 0,
    2, 7, 6, 2, 3, 7, 0, 1, 9, 0, 0, 0, 0, 0, 0,
    1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, 0, 0, 0,
    10, 7, 6, 10, 1, 7, 1, 3, 7, 0, 0, 0, 0, 0, 0,
    10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, 0, 0, 0,
    0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, 0, 0, 0,
    7, 6, 10, 7, 10, 8, 8, 10, 9, 0, 0, 0, 0, 0, 0,
    6, 8, 4, 11, 8, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 6, 11, 3, 0, 6, 0, 4, 6, 0, 0, 0, 0, 0, 0,
    8, 6, 11, 8, 4, 6, 9, 0, 1, 0, 0, 0, 0, 0, 0,
    9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, 0, 0, 0,
    6, 8, 4, 6, 11, 8, 2, 10, 1, 0, 0, 0, 0, 0, 0,
    1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, 0, 0, 0,
    4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, 0, 0, 0,
    10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3,
    8, 2, 3, 8, 4, 2, 4, 6, 2, 0, 0, 0, 0, 0, 0,
    0, 4, 2, 4, 6, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, 0, 0, 0,
    1, 9, 4, 1, 4, 2, 2, 4, 6, 0, 0, 0, 0, 0, 0,
    8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, 0, 0, 0,
    10, 1, 0, 10, 0, 6, 6, 0, 4, 0, 0, 0, 0, 0, 0,
    4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3,
    10, 9, 4, 6, 10, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 9, 5, 7, 6, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 8, 3, 4, 9, 5, 11, 7, 6, 0, 0, 0, 0, 0, 0,
    5, 0, 1, 5, 4, 0, 7, 6, 11, 0, 0, 0, 0, 0, 0,
    11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, 0, 0, 0,
    9, 5, 4, 10, 1, 2, 7, 6, 11, 0, 0, 0, 0, 0, 0,
    6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, 0, 0, 0,
    7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, 0, 0, 0,
    3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6,
    7, 2, 3, 7, 6, 2, 5, 4, 9, 0, 0, 0, 0, 0, 0,
    9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, 0, 0, 0,
    3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, 0, 0, 0,
    6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8,
    9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, 0, 0, 0,
    1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4,
    4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10,
    7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, 0, 0, 0,
    6, 9, 5, 6, 11, 9, 11, 8, 9, 0, 0, 0, 0, 0, 0,
    3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, 0, 0, 0,
    0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, 0, 0, 0,
    6, 11, 3, 6, 3, 5, 5, 3, 1, 0, 0, 0, 0, 0, 0,
    1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, 0, 0, 0,
    0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10,
    11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5,
    6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, 0, 0, 0,
    5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, 0, 0, 0,
    9, 5, 6, 9, 6, 0, 0, 6, 2, 0, 0, 0, 0, 0, 0,
    1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8,
    1, 5, 6, 2, 1, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6,
    10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, 0, 0, 0,
    0, 3, 8, 5, 6, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    10, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    11, 5, 10, 7, 5, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    11, 5, 10, 11, 7, 5, 8, 3, 0, 0, 0, 0, 0, 0, 0,
    5, 11, 7, 5, 10, 11, 1, 9, 0, 0, 0, 0, 0, 0, 0,
    10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, 0, 0, 0,
    11, 1, 2, 11, 7, 1, 7, 5, 1, 0, 0, 0, 0, 0, 0,
    0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, 0, 0, 0,
    9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, 0, 0, 0,
    7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2,
    2, 5, 10, 2, 3, 5, 3, 7, 5, 0, 0, 0, 0, 0, 0,
    8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, 0, 0, 0,
    9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, 0, 0, 0,
    9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2,
    1, 3, 5, 3, 7, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 8, 7, 0, 7, 1, 1, 7, 5, 0, 0, 0, 0, 0, 0,
    9, 0, 3, 9, 3, 5, 5, 3, 7, 0, 0, 0, 0, 0, 0,
    9, 8, 7, 5, 9, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    5, 8, 4, 5, 10, 8, 10, 11, 8, 0, 0, 0, 0, 0, 0,
    5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, 0, 0, 0,
    0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, 0, 0, 0,
    10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4,
    2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, 0, 0, 0,
    0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11,
    0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5,
    9, 4, 5, 2, 11, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, 0, 0, 0,
    5, 10, 2, 5, 2, 4, 4, 2, 0, 0, 0, 0, 0, 0, 0,
    3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9,
    5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, 0, 0, 0,
    8, 4, 5, 8, 5, 3, 3, 5, 1, 0, 0, 0, 0, 0, 0,
    0, 4, 5, 1, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, 0, 0, 0,
    9, 4, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 11, 7, 4, 9, 11, 9, 10, 11, 0, 0, 0, 0, 0, 0,
    0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, 0, 0, 0,
    1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, 0, 0, 0,
    3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4,
    4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, 0, 0, 0,
    9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3,
    11, 7, 4, 11, 4, 2, 2, 4, 0, 0, 0, 0, 0, 0, 0,
    11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, 0, 0, 0,
    2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, 0, 0, 0,
    9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7,
    3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10,
    1, 10, 2, 8, 7, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 9, 1, 4, 1, 7, 7, 1, 3, 0, 0, 0, 0, 0, 0,
    4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, 0, 0, 0,
    4, 0, 3, 7, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    4, 8, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    9, 10, 8, 10, 11, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 9, 3, 9, 11, 11, 9, 10, 0, 0, 0, 0, 0, 0,
    0, 1, 10, 0, 10, 8, 8, 10, 11, 0, 0, 0, 0, 0, 0,
    3, 1, 10, 11, 3, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 2, 11, 1, 11, 9, 9, 11, 8, 0, 0, 0, 0, 0, 0,
    3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, 0, 0, 0,
    0, 2, 11, 8, 0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 3, 8, 2, 8, 10, 10, 8, 9, 0, 0, 0, 0, 0, 0,
    9, 10, 2, 0, 9, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, 0, 0, 0,
    1, 10, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 3, 8, 9, 1, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 9, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


static void SCE_MC_InitCell (SCE_SMCCell *cell)
{
    cell->x = cell->y = cell->z = 0;
    cell->indices[0] = cell->indices[1] = cell->indices[2] = 0;
    cell->conf = 0;
}
static void SCE_MC_ClearCell (SCE_SMCCell *cell)
{
    (void)cell;
}


void SCE_MC_Init (SCE_SMCGenerator *mc)
{
    mc->n_cells = 0;
    mc->cells = NULL;
    mc->n_vertices = 0;
    mc->n_indices = 0;
    mc->cell_indices = NULL;
    mc->x = mc->y = mc->z = 0;
    mc->w = mc->h = mc->d = 0;

    mc->last_x = mc->last_y = mc->last_z = 0;
    mc->finished = SCE_TRUE;
}
void SCE_MC_Clear (SCE_SMCGenerator *mc)
{
    SCE_free (mc->cells);
    SCE_free (mc->cell_indices);
}

/* you definetely need to call SCE_MC_Build() after that */
void SCE_MC_SetNumCells (SCE_SMCGenerator *mc, size_t n_cells)
{
    mc->n_cells = n_cells;
}
size_t SCE_MC_GetNumCells (const SCE_SMCGenerator *mc)
{
    return mc->n_cells;
}

int SCE_MC_Build (SCE_SMCGenerator *mc)
{
    size_t n;

    SCE_free (mc->cells);
    mc->cells = NULL;
    SCE_free (mc->cell_indices);
    mc->cell_indices = NULL;

    n = mc->n_cells;

    if (n > 0) {
        if (!(mc->cells = SCE_malloc (n * sizeof *mc->cells))) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
        if (!(mc->cell_indices = SCE_malloc (n * sizeof *mc->cell_indices))) {
            SCEE_LogSrc ();
            return SCE_ERROR;
        }
    }
    /* no need to initialize the cells */
    return SCE_OK;
}


/*
 *  4________4________5
 *  |\               |\
 *  | \              | \
 *  |  \             |  \
 *  |   \7          9|   \5
 * 8|    \           |    \
 *  |     \          |     \
 *  |      \7______6________\6
 *  |      |                |         y   z
 *  |0____ | ________|1     |          \ |
 *   \     |   0      \     |           \|__ x
 *    \    |           \    |
 *    3\   |11          \1  |10
 *      \  |             \  |
 *       \ |              \ |
 *        \|_______2_______\|
 *         3                2
 *
 *
 * local numbering:
 *
 *    y   z
 *     \  |
 *     0\ |2
 *       \|____ x
 *           1
 */

static SCEuint SCE_MC_GetConfig (SCEubyte corners[8])
{
    SCEuint c = 0, i;
    for (i = 0; i < 8; i++)
        c |= (corners[i] & 0x80) >> (7 - i);
    return c;
}

static void SCE_MC_Interp (SCEvertices *v, SCEubyte a_, SCEubyte b_,
                           SCE_TVector3 v0, SCE_TVector3 v1)
{
    float a, b, w;

    a = (float)a_ / 256.0 - 0.5;
    b = (float)b_ / 256.0 - 0.5;

    w = -a / (b - a);
    SCE_Vector3_Operator2v (v, = (1.0 - w) *, v0, + w *, v1);
}

static size_t SCE_MC_MakeCellVertices (SCE_SMCCell *cell, SCEubyte corners[8],
                                       size_t n_vertices, SCEvertices *vertices,
                                       float coefx, float coefy, float coefz)
{
    SCEuint conf, corner3;
    size_t n = 0;
    SCE_TVector3 vertex0, vertex1, div;

    SCE_Vector3_Set (div, coefx, coefy, coefz);
    vertices = &vertices[n_vertices * 3];
    SCE_Vector3_Set (vertex0, (float)cell->x, (float)cell->y, (float)cell->z);
    SCE_Vector3_Operator1v (vertex0, *=, div);

    conf = cell->conf;
    corner3 = (conf >> 3) & 1;

    /* corners 3 and 0 */
    if (corner3 != (conf & 1)) {
        cell->indices[0] = n_vertices;
        n++;
        SCE_Vector3_Copy (vertex1, vertex0);
        vertex1[1] += div[1];
        SCE_MC_Interp (vertices, corners[3], corners[0], vertex0, vertex1);
        vertices = &vertices[3];
    }
    /* corners 3 and 2 */
    if (corner3 != ((conf >> 2) & 1)) {
        cell->indices[1] = n_vertices + n;
        n++;
        SCE_Vector3_Copy (vertex1, vertex0);
        vertex1[0] += div[0];
        SCE_MC_Interp (vertices, corners[3], corners[2], vertex0, vertex1);
        vertices = &vertices[3];
    }
    /* corners 3 and 7 */
    if (corner3 != ((conf >> 7) & 1)) {
        cell->indices[2] = n_vertices + n;
        n++;
        SCE_Vector3_Copy (vertex1, vertex0);
        vertex1[2] += div[2];
        SCE_MC_Interp (vertices, corners[3], corners[7], vertex0, vertex1);
        vertices = &vertices[3];
    }

    return n;
}

/**
 * \brief Generates vertices according to the marching cube algorithm
 * \param mc a mc generator
 * \param region a region
 * \param grid voxel grid
 * \param vertices output vertices
 *
 * Make sure to leave additionnal slices in grid filled with 0
 * to properly generate vertices in the borders, otherwise unused vertices
 * will be generated.
 *
 * \return the number of vertices generated
 * \sa SCE_MC_GenerateVerticesRange()
 */
size_t SCE_MC_GenerateVertices (SCE_SMCGenerator *mc,
                                const SCE_SIntRect3 *region,
                                const SCE_SGrid *grid, SCEvertices *vertices)
{
    return SCE_MC_GenerateVerticesRange (mc, region, grid, vertices, 0);
}

/**
 * \brief Generates vertices according to the marching cube algorithm
 * \param mc a mc generator
 * \param region a region
 * \param grid voxel grid
 * \param vertices output vertices
 * \param num number of cells to process
 *
 * Make sure to leave additionnal slices in grid filled with 0
 * to properly generate vertices in the borders, otherwise unused vertices
 * will be generated.
 *
 * \return the number of vertices generated
 * \sa SCE_MC_GenerateVertices()
 */
size_t SCE_MC_GenerateVerticesRange (SCE_SMCGenerator *mc,
                                     const SCE_SIntRect3 *region,
                                     const SCE_SGrid *grid,
                                     SCEvertices *vertices, size_t num)
{
    SCE_SMCCell *cell = NULL;
    SCEuint x, y, z, x_, y_, z_, w, h, d;
    float a, b, c;
    SCEubyte corners[8];
    size_t n;
    int p1[3], p2[3];

    w = mc->w = SCE_Rectangle3_GetWidth (region);
    h = mc->h = SCE_Rectangle3_GetHeight (region);
    d = mc->d = SCE_Rectangle3_GetDepth (region);
    a = 1.0 / SCE_Grid_GetWidth (grid);
    b = 1.0 / SCE_Grid_GetHeight (grid);
    c = 1.0 / SCE_Grid_GetDepth (grid);

    SCE_Rectangle3_GetPointsv (region, p1, p2);
    /* memorize the origin for generatenormals() below */
    mc->x = p1[0];
    mc->y = p1[1];
    mc->z = p1[2];

    /* ignore num if null */
    if (num == 0)
        num = 2 * SCE_Grid_GetNumPoints (grid); /* times two, because. */

    mc->finished = SCE_FALSE;

    z = mc->last_z;
    mc->last_z = 0;
    for (z_ = p1[2] + z; z < d; z++, z_++) {
        y = mc->last_y;
        mc->last_y = 0;
        for (y_ = p1[1] + y; y < h; y++, y_++) {
            x = mc->last_x;
            mc->last_x = 0;
            for (x_ = p1[0] + x; x < w; x++, x_++) {
                size_t offset = w * (z * h + y) + x;

                if (num == 0) {
                    /* save up and quit */
                    mc->last_x = x;
                    mc->last_y = y;
                    mc->last_z = z;
                    return 0;
                }
                num--;

                cell = &mc->cells[offset];
                cell->x = x;
                cell->y = y;
                cell->z = z;

                SCE_Grid_GetPoint (grid, x_,     y_,     z_,     &corners[3]);
                SCE_Grid_GetPoint (grid, x_ + 1, y_,     z_,     &corners[2]);
                SCE_Grid_GetPoint (grid, x_,     y_ + 1, z_,     &corners[0]);
                SCE_Grid_GetPoint (grid, x_ + 1, y_ + 1, z_,     &corners[1]);
                SCE_Grid_GetPoint (grid, x_,     y_,     z_ + 1, &corners[7]);
                SCE_Grid_GetPoint (grid, x_ + 1, y_,     z_ + 1, &corners[6]);
                SCE_Grid_GetPoint (grid, x_,     y_ + 1, z_ + 1, &corners[4]);
                SCE_Grid_GetPoint (grid, x_ + 1, y_ + 1, z_ + 1, &corners[5]);

                cell->conf = SCE_MC_GetConfig (corners);

                if (cell->conf != 0 && cell->conf != 255) {
                    n = SCE_MC_MakeCellVertices (cell, corners, mc->n_vertices,
                                                 vertices, a, b, c);
                    mc->n_vertices += n;
                    mc->cell_indices[mc->n_indices] = offset;
                    mc->n_indices++;
                }
            }
        }
    }

    mc->last_x = mc->last_y = mc->last_z = 0;
    mc->finished = SCE_TRUE;
    n = mc->n_vertices;
    mc->n_vertices = 0;

    return n;
}

int SCE_MC_IsGenerationFinished (const SCE_SMCGenerator *mc)
{
    return mc->finished;
}


static void SCE_MC_MakeNormal (const SCE_SGrid *grid, int x, int y, int z,
                               SCE_TVector3 normal)
{
    SCEubyte p[6];
    SCE_Grid_GetPoint (grid, x - 1, y,     z, &p[0]);
    SCE_Grid_GetPoint (grid, x + 1, y,     z, &p[1]);
    SCE_Grid_GetPoint (grid, x,     y - 1, z, &p[2]);
    SCE_Grid_GetPoint (grid, x,     y + 1, z, &p[3]);
    SCE_Grid_GetPoint (grid, x,     y,     z - 1, &p[4]);
    SCE_Grid_GetPoint (grid, x,     y,     z + 1, &p[5]);
    normal[0] = (float)p[0] - (float)p[1];
    normal[1] = (float)p[2] - (float)p[3];
    normal[2] = (float)p[4] - (float)p[5];
    SCE_Vector3_Normalize (normal);
}

static size_t SCE_MC_MakeCellNormals (const SCE_SGrid *grid, SCEuint conf,
                                      int x, int y, int z, SCEubyte corners[8],
                                      SCEvertices *vertices)
{
    SCEuint corner3;
    size_t n = 0;
    SCE_TVector3 vertex0, vertex1;

    SCE_MC_MakeNormal (grid, x, y, z, vertex0);

    corner3 = (conf >> 3) & 1;

    /* corners 3 and 0 */
    if (corner3 != (conf & 1)) {
        n++;
        SCE_MC_MakeNormal (grid, x, y + 1, z, vertex1);
        SCE_MC_Interp (vertices, corners[3], corners[0], vertex0, vertex1);
        SCE_Vector3_Normalize (vertices);
        vertices = &vertices[3];
    }
    /* corners 3 and 2 */
    if (corner3 != ((conf >> 2) & 1)) {
        n++;
        SCE_MC_MakeNormal (grid, x + 1, y, z, vertex1);
        SCE_MC_Interp (vertices, corners[3], corners[2], vertex0, vertex1);
        SCE_Vector3_Normalize (vertices);
        vertices = &vertices[3];
    }
    /* corners 3 and 7 */
    if (corner3 != ((conf >> 7) & 1)) {
        n++;
        SCE_MC_MakeNormal (grid, x, y, z + 1, vertex1);
        SCE_MC_Interp (vertices, corners[3], corners[7], vertex0, vertex1);
        SCE_Vector3_Normalize (vertices);
        vertices = &vertices[3];
    }

    return n;
}

void SCE_MC_GenerateNormals (SCE_SMCGenerator *mc, const SCE_SGrid *grid,
                             SCEvertices *vertices)
{
    SCE_SMCCell *cell = NULL;
    SCEuint x, y, z;
    SCEubyte corners[8];
    size_t n_vertices = 0, i;

    for (i = 0; i < mc->n_indices; i++) {
        cell = &mc->cells[mc->cell_indices[i]];

        x = cell->x + mc->x;
        y = cell->y + mc->y;
        z = cell->z + mc->z;

        /* NOTE: grid fetch overhead: vertices and normal generation could
                 be merged */
        SCE_Grid_GetPoint (grid, x,     y,     z,     &corners[3]);
        SCE_Grid_GetPoint (grid, x + 1, y,     z,     &corners[2]);
        SCE_Grid_GetPoint (grid, x,     y + 1, z,     &corners[0]);
        SCE_Grid_GetPoint (grid, x,     y,     z + 1, &corners[7]);

        n_vertices += SCE_MC_MakeCellNormals (grid, cell->conf, x, y, z,
                                              corners, &vertices[n_vertices*3]);
    }
}



static SCE_SMCCell* fetch_cell (const SCE_SMCGenerator *mc, SCEuint x,
                                SCEuint y, SCEuint z)
{
    size_t offset = mc->w * (z * mc->h + y) + x;
    return &mc->cells[offset];
}

static size_t SCE_MC_MakeCellIndices (const SCE_SMCGenerator *mc,
                                      SCE_SMCCell *cell, SCEindices *indices)
{
    SCEuint i, n_tri;
    SCEuint edges[12];
    SCE_SMCCell *c = NULL;

    /* fill edges with vertex indices */
    c = cell;
    edges[3] =  c->indices[0];
    edges[2] =  c->indices[1];
    edges[11] = c->indices[2];
    c = fetch_cell (mc, cell->x,     cell->y + 1, cell->z);
    edges[0] =  c->indices[1];
    edges[8] =  c->indices[2];
    c = fetch_cell (mc, cell->x + 1, cell->y + 1, cell->z);
    edges[9] =  c->indices[2];
    c = fetch_cell (mc, cell->x + 1, cell->y,     cell->z);
    edges[1] =  c->indices[0];
    edges[10] = c->indices[2];
    c = fetch_cell (mc, cell->x,     cell->y,     cell->z + 1);
    edges[6] =  c->indices[1];
    edges[7] =  c->indices[0];
    c = fetch_cell (mc, cell->x,     cell->y + 1, cell->z + 1);
    edges[4] =  c->indices[1];
    c = fetch_cell (mc, cell->x + 1, cell->y,     cell->z + 1);
    edges[5] =  c->indices[0];

    /* make triangles */
    n_tri = lt_num_tri[cell->conf];

    for (i = 0; i < n_tri * 3; i++)
        indices[i] = edges[lt_edges[cell->conf * 15 + i]];

    return n_tri * 3;
}

size_t SCE_MC_GenerateIndices (SCE_SMCGenerator *mc, SCEindices *indices)
{
    SCE_SMCCell *cell = NULL;
    SCEuint i;
    size_t n_indices = 0;

    for (i = 0; i < mc->n_indices; i++) {
        cell = &mc->cells[mc->cell_indices[i]];
        /* cell must not be across a border
           we could have removed those cells at vertices generation,
           but we actually need them for normal generation */
        if (cell->x < mc->w - 1 && cell->y < mc->h - 1 && cell->z < mc->d - 1)
            n_indices += SCE_MC_MakeCellIndices (mc, cell, &indices[n_indices]);
    }

    /* kinda important for split generation */
    mc->n_indices = 0;

    return n_indices;
}
