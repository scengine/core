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

#ifndef LIBWAR_H
#define LIBWAR_H

#include <stdio.h>

#ifndef WAR_VERSION_STRING
 #define WAR_VERSION_STRING "(use war_getversionstring() function)"
#endif

#define WAR_FILE_EXTENSION "obj"

typedef unsigned int WAuint;
/**
 * \todo a reviser : les types doivent etres de taille constante
 */
typedef float WAfloat4;         /* doit faire 4 octets */
typedef unsigned short WAuint2; /* doit faire 2 octets */
typedef WAuint WAuint4;         /* doit faire 4 octets */
typedef int WAint4;             /* doit faire 4 octets */


typedef struct WarMesh {
    int canfree;     /* autorisation de detruire les donnees */
    int canfree_indices;

    WAfloat4 *pos;
    WAfloat4 *tex;
    WAfloat4 *nor;
    WAint4 vcount;

    WAint4 *indices;
    WAint4 icount;
} WarMesh;

const char* war_geterror (void);
const char* war_getversionstring (void);

WarMesh* war_new (void);
void war_canfree (WarMesh*, int);
void war_clear (WarMesh*);
void war_free (WarMesh*);

WarMesh* war_read (FILE*, int, unsigned int);

#endif /* guard */
