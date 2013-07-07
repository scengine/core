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

/* created: 06/07/2013
   updated: 06/07/2013 */

#ifndef SCENOISE_H
#define SCENOISE_H

#ifdef __cplusplus
extern "C" {
#endif

int SCE_Init_Noise (void);

double SCE_Noise_Perlin1D (double);
double SCE_Noise_Perlin2D (double[2]);

float SCE_Noise_Linear3D (double[3]);
float SCE_Noise_Smooth3D (double[3]);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* guard */
