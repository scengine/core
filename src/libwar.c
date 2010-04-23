/*------------------------------------------------------------------------------
    SCEngine - A 3D real time rendering engine written in the C language
    Copyright (C) 2006-2010  Antony Martin <martin(dot)antony(at)yahoo(dot)fr>

    This program is SCE_free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the SCE_Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 -----------------------------------------------------------------------------*/

/* updated: 01/04/2009 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include <SCE/utils/SCEUtils.h>
#include "SCE/core/libwar.h"

static void war_error (const char *e, ...) SCE_GNUC_PRINTF (1, 2);


static char error[256] = {0};

/*** fonctions de gestion des erreurs ***/
static void war_error (const char *e, ...)
{
    va_list args;
    memset (error, '\0', sizeof error);
    va_start (args, e);
    vsprintf (error, e, args);
    va_end (args);
}

const char* war_geterror (void)
{
    return error;
}

const char* war_getversionstring (void)
{
    return WAR_VERSION_STRING;
}

/*** fonctions de manipulation d'une structure WarMesh ***/
static void war_init (WarMesh *m)
{
    m->pos = m->tex = m->nor = NULL;
    m->vcount = m->icount = 0;
    m->indices = NULL;
    m->canfree = SCE_FALSE;
    m->canfree_indices = SCE_FALSE;
}
WarMesh* war_new (void)
{
    WarMesh *m = SCE_malloc (sizeof *m);
    if (m)
        war_init (m);
    return m;
}
void war_canfree (WarMesh *m, int c)
{
    m->canfree = c;
}
void war_canfreeindices (WarMesh *m, int c)
{
    m->canfree_indices = c;
}
void war_clear (WarMesh *m)
{
    if (m) {
        if (m->canfree) {
            SCE_free (m->pos);
            SCE_free (m->tex);
            SCE_free (m->nor);
        }
        if (m->canfree_indices) {
            SCE_free (m->indices);
        }
        m->pos = NULL;
        m->tex = NULL;
        m->nor = NULL;
        m->indices = NULL;
    }
}
void war_free (WarMesh *m)
{
    war_clear (m);
    SCE_free (m);
}


/*** fonctions outils ***/

static void war_jumpline (FILE *f)
{
    int c;
    while ((c = fgetc (f)) != '\n' && c != EOF);
}

static void war_jumpspaces (FILE *f)
{
    while (isspace (fgetc (f)));
    fseek (f, -1, SEEK_CUR);
}

static long war_pow (long x, WAuint n)
{
    if (n == 0)
        return 1;
    else if (n == 1)
        return x;
    else if (n%2)
        return x * war_pow (x*x, (n-1)/2);
    else
        return war_pow (x*x, n/2);
}

static WAfloat4 war_valof (const char *str)
{
    WAuint i;              /* compteur */
    WAfloat4 res = 0.;   /* valeur de retour */
    WAfloat4 coeff = 0.;
    int val = 0;
    WAuint str_ilen = 0;   /* longueur de la valeur entiere */
    WAuint str_dlen = 0;   /* longueur de la valeur décimale */
    WAuint unit_tmp = 0;

    /* calcul de la longueur de la chaine str -->
       longueur de la partie entiere */
    while (str[str_ilen] != '\0' &&
           str[str_ilen] != '\n' &&
           str[str_ilen] != '.') {
        str_ilen++;
    }
    /* --> longueur de la partie decimale (si elle existe uniquement) */
    if (str[str_ilen] == '.') {
        while (str[str_dlen+str_ilen+1] != '\0' &&
               str[str_dlen+str_ilen] != '\n') {
            str_dlen++;
        }
    }

    if (str[0] == '-' || str[0] == '+')
        val = 1;

    coeff = war_pow (10, str_ilen - val);

    /* calcul de la valeur entiere */
    for (i=val; i<str_ilen; i++) {
        unit_tmp = str[i] - '0';
        if (unit_tmp > 9) {
            fprintf (stderr, "valof() : argument %d : "
                     "'%c' is not a number\n", i, str[i]);
            /* on saute, on s'en occupe pas */
            unit_tmp = 0;
        } else
            coeff /= 10.0f;

        res += unit_tmp * coeff;
    }

    coeff = 1.0f;
    /* calcul de la valeur decimale */
    for (i=str_ilen+1; i<str_dlen+str_ilen+1; i++) {
        unit_tmp = str[i] - '0';
        if (unit_tmp > 9) {
            fprintf (stderr, "valof() : argument %d : "
                     "'%c' is not a number\n", i, str[i]);
            /* on saute, on s'en occupe pas */
            unit_tmp = 0;
        } else
            coeff /= 10.0f;

        res += unit_tmp*coeff;
    }

    if (str[0] == '-')
        res = -res;

    return res;
}

static int war_readnumber (FILE *fp, WAfloat4 *fres, int *ires, WAint4 *uires)
{
    int c;
    char number[16] = {'\0'}; /* nombre sous forme de chaine */
    WAuint pn = 0;              /* position dans 'number' */
    int pointread = 0;        /* pour eviter de lire deux fois un point */

    while ((c = fgetc (fp)) != EOF && (
               (c <= '9' && c >= '0') ||
               (c == '.' && !pointread) ||
               (c == '-') ) && pn < sizeof number) {
        number[pn] = c;
        pn++;
        if (c == '.')
            pointread = 1;
    }
    if (pn == 0) 
        return 1;
    else {
        /* on revient au premier caractere invalide */
        fseek (fp, -1, SEEK_CUR);
        if (fres)
            *fres = war_valof (number);
        if (ires)
            *ires = strtol (number, NULL, 10);
        if (uires)
            *uires = strtoul (number, NULL, 10);
        return 0;
    }
}

static int war_readfloat (FILE *fp, WAfloat4 *result)
{
    return war_readnumber (fp, result, NULL, NULL);
}
#if 0
static int war_readint (FILE *fp, int *result)
{
    return war_readnumber (fp, NULL, result, NULL);
}
#endif
static int war_readuint (FILE *fp, WAint4 *result)
{
    return war_readnumber (fp, NULL, NULL, result);
}

static WAuint war_strsinline (FILE *fp, const int a)
{
    int valid;
    WAuint n = 0;     /* nombre de chaines */
    int c;
    long curo;

    curo = ftell (fp);

    do {
        valid = 0;
        while (!isspace ((c = fgetc (fp))) && c != EOF && c != '\n')
            valid++;
        if (valid >= 1)
            n++;
    } while (c != EOF && c != '\n');

    if (a)
        fseek (fp, curo, SEEK_SET);

    return n;
}

static WAuint war_getline (FILE *fp)
{
    WAuint n = 1;
    int c;
    long end = ftell (fp);
    rewind (fp);
    while ((c = fgetc (fp)) != EOF && ftell (fp) < end) {
        if (c == '\n')
            n++;
    }
    fseek (fp, end, SEEK_SET);
    return n;
}


/*** fonctions de lecture d'un .obj ***/

static void war_readinfos (FILE *fp, unsigned int *n_objs,
                           size_t *v, size_t *vt, size_t *vn)
{
    int c;

    *n_objs = 0;
    *v = *vt = *vn = 0;
    do {
        c = fgetc (fp);
        switch (c) {
        case 'v':
            c = fgetc (fp);
            switch (c) {
            case ' ':
                (*v) += 3;
                break;
            case 't':
                (*vt) += 2;
                break;
            case 'n':
                (*vn) += 3;
            }
            break;

        case 'g':
        case 'o':
            (*n_objs)++;
        }
        if (c != '\n')
            war_jumpline (fp);
    } while (c != EOF);
    if (*n_objs == 0)
        (*n_objs) = 1;             /* prout */
}

/* lit la taille du tableau d'indices de l'objet selectionne
   (retourne le nombre de valeurs que devra stocker le tableau) */
static int war_readnumindices (FILE *fp)
{
    int c;
    int n = 0;

    do {
        c = fgetc (fp);
        switch (c) {
        case 'f':
            /* nombre d indices que comporte la face */
            switch (war_strsinline (fp, 1)) {
            case 0:
            case 1:
                war_error ("line %u : bad file format, "
                           "face with lesser than two vertices!",
                           war_getline (fp));
                return -1;
                break;
            case 2:
            case 3:
                n += 9;
                break;
            case 4:
                n += 18;
                break;
            default:
                war_error ("line %u : bad file format, "
                           "face with more than 4 vertices!",
                           war_getline (fp));
                return -1;
            }
            break;

        /* objet --> fini */
        case 'o':
            c = EOF;
        }
        if (c != '\n')
            war_jumpline (fp);
    } while (c != EOF);

    return n;
}

static void war_readdataline (FILE *fp, WAfloat4 *data, int tex)
{
    war_jumpspaces (fp);
    war_readfloat (fp, &data[0]);
    war_jumpspaces (fp);
    war_readfloat (fp, &data[1]);
    if (!tex) {
        war_jumpspaces (fp);
        war_readfloat (fp, &data[2]);
    }
}

static void war_readdata (FILE *fp, WAfloat4 *v, WAfloat4 *vt, WAfloat4 *vn)
{
    int c;
    WAuint i = 0, j = 0, k = 0; /* initialisations importantes */

    do {
        c = fgetc (fp);
        /* si on se trouve a une definition de sommet, lire le sommet */    
        if (c == 'v') {
            c = fgetc (fp);
            switch (c) {
            case ' ':
                if (v) {
                    war_readdataline (fp, &v[i], 0);
                    i += 3;
                }
                break;
            case 't':
                if (vt) {
                    war_readdataline (fp, &vt[j], 1);
                    j += 2;
                }
                break;
            case 'n':
                if (vn) {
                    war_readdataline (fp, &vn[k], 0);
                    k += 3;
                }
            }
        }
        if (c != '\n')
            war_jumpline (fp);
    } while (c != EOF);
}

static int war_readindexline (FILE *fp, WAint4 *indices)
{
    int c;
    WAuint i, j;
    WAuint p = 0; /* position dans 'indices' */
    WAint4 index[4][3] = {{0}};
    WAuint numstr;

    /* algorithme de lecture des donnees dans 'fp' */
    numstr = war_strsinline (fp, 1);
    for (i = 0; i < numstr; i++) {
        war_jumpspaces (fp);
        for (j=0; j<3; j++) {
            war_readuint (fp, &index[i][j]);
            c = fgetc (fp);
            if (c == '/') {
                c = fgetc (fp);
                if (c == '/')
                    j++;
                else
                    fseek (fp, -1, SEEK_CUR);
            } else {
                fseek (fp, -1, SEEK_CUR);
                break;
            }
        }
    }

    /* algorithme de tri des donnees selon le type de polygone lu */
    for (i=0; i<3; i++) {
        indices[p]   = index[i][0];
        indices[p+1] = index[i][1];
        indices[p+2] = index[i][2];
        p += 3;
    }

    switch (numstr) {
    case 2:
        p -= 3;
        indices[p]   = index[0][0];
        indices[p+1] = index[0][1];
        indices[p+2] = index[0][2];
    case 3:
        numstr = 9;
        break;

    case 4:
        indices[p]   = index[2][0];
        indices[p+1] = index[2][1];
        indices[p+2] = index[2][2];
        p += 3;
        indices[p]   = index[3][0];
        indices[p+1] = index[3][1];
        indices[p+2] = index[3][2];
        p += 3;
        indices[p]   = index[0][0];
        indices[p+1] = index[0][1];
        indices[p+2] = index[0][2];
        numstr = 18;
    /* pas la peine de verifier via default si une mauvaise valeur a ete lue,
       la fonction de lecture de la taille des indices s'en charge */
    }

    /* 'numstr' vaut maintenant le nombre de "cases" de 'indices' remplies */
    return numstr;
}
static int war_readfakeindexline (FILE *fp, WAint4 *indices)
{
    int c;
    WAuint i, j;
    WAuint p = 0; /* position dans 'indices' */
    WAint4 index[4][3] = {{0}};
    WAuint numstr;

    /* algorithme de lecture des donnees dans 'fp' */
    numstr = war_strsinline (fp, 1);
    for (i = 0; i < numstr; i++) {
        war_jumpspaces (fp);
        for (j=0; j<3; j++) {
            war_readuint (fp, &index[i][j]);
            c = fgetc (fp);
            if (c == '/') {
                c = fgetc (fp);
                if (c == '/')
                    j++;
                else
                    fseek (fp, -1, SEEK_CUR);
            } else {
                fseek (fp, -1, SEEK_CUR);
                break;
            }
        }
    }

    switch (numstr) {
    case 2:
    case 3:
        numstr = 9;
        break;
    case 4:
        numstr = 18;
    /* pas la peine de verifier via default si une mauvaise valeur a ete lue,
       la fonction de lecture de la taille des indices s'en charge */
    }

    return numstr;
}

static void war_readindices (FILE *fp, WAint4 *indices)
{
    int c;
    size_t pos = 0; /* initialisation importante */
    int (*readindexlinefunc)(FILE*, WAint4*);

    if (indices)
        readindexlinefunc = war_readindexline;
    else
        readindexlinefunc = war_readfakeindexline;

    do {
        c = fgetc (fp);
        if (c == 'f') {
            pos += readindexlinefunc (fp, &indices[pos]);
        } else if (c == 'o') /* definition d'un autre objet, arret */
            break;
        else if (c != '\n')
            war_jumpline (fp);
    } while (c != EOF);
}


static int war_vectcmp (WAint4 *a, WAint4 *b)
{
    return (a[0] == b[0] && a[1] == b[1] && a[2] == b[2]);
}

/* construit des indices pour un mesh et tri ses sommets en consequence */
static int war_makeindices (WarMesh *me)
{
    WAint4 *indices = NULL;
    WAuint i, j, k = 0, l = 0, m = 0;
    int ok = 1;
    float prc = 0.;
    float prc_back = 0.;
    WAfloat4 *pos = NULL, *nor = NULL, *tex = NULL;
    WAuint n_triangles = me->icount / 9;
    WAint4 *index = me->indices;

    indices = SCE_malloc (n_triangles * 3 * sizeof *indices);
    if (!indices)
        return -1;

    /* taille reelle impossible a evaluer, on prend la taille maximale */
    if (!(pos = SCE_malloc (n_triangles * 9 * sizeof *pos))) {
        SCE_free (indices);
        return -1;
    }
    if (index[2])
        if (!(nor = SCE_malloc (n_triangles * 9 * sizeof *nor))) {
            SCE_free (pos);
            SCE_free (indices);
            return -1;
        }
    if (index[1])
        if (!(tex = SCE_malloc (n_triangles * 6 * sizeof *tex))) {
            SCE_free (nor);
            SCE_free (pos);
            SCE_free (indices);
            return -1;
        }

    /* pour chaque triangle */
    for (i = 0; i < n_triangles; i++) {
        prc = (float)i / n_triangles;
        prc *= 100.;
        if (prc >= prc_back+0.1)
            prc_back = prc;

        /* pour chaque sommet */
        for (j=0; j<3; j++) {
            ok = 1;

            /* on verifie tous les precedents indices pour en trouver
               un identique... */
            for (m=l; m>0; m--) {
                if (war_vectcmp (&index[(m-1)*3], &index[l*3])) {
                    /* un indice a ete trouve, on l'utilise et ignore
                       la copie des donnees vectorielles */
                    indices[l] = indices[m-1];
                    ok = 0;
                    break;
                }
            }

            if (ok) {
                /* creation d'un nouvel indice & copie des donnees
                   pour le nouveau sommet */
                indices[l] = k / 3;

                pos[k] = me->pos [3*index[l*3]-3];
                pos[k+1] = me->pos [3*index[l*3]-2];
                pos[k+2] = me->pos [3*index[l*3]-1];

                if (index[2]) {
                    nor[k] = me->nor [3*index[l*3+2]-3];
                    nor[k+1] = me->nor [3*index[l*3+2]-2];
                    nor[k+2] = me->nor [3*index[l*3+2]-1];
                }

                if (index[1]) {
                    tex[k-(k/3)] = me->tex [2*index[l*3+1]-2];
                    tex[k-(k/3)+1] = me->tex [2*index[l*3+1]-1];
                }
                k += 3;
            }
            l++;
        }
    }

    war_clear (me);

    war_canfree (me, 1);
    me->icount = n_triangles * 3;
    me->vcount = k / 3;
    me->pos = pos;
    me->nor = nor;
    me->tex = tex;
    me->indices = indices;

    return 0;
}

/* cree une suite de sommets en fonction d'indices du type des .obj */
static int war_makevertices (WarMesh *me)
{
    WAuint i, j;
    WAfloat4 *pos = NULL, *nor = NULL, *tex = NULL;

    /* icount = nombre de variables dans le tableau d'indices, mais
       etant donne que les indices sont intrelaces (*3 pour
       3 types de donnees par sommet), pas besoin de multiplier par 3  */
    if (!(pos = SCE_malloc (me->icount * sizeof *pos)))
        return -1;
    if (me->indices[2])
        if (!(nor = SCE_malloc (me->icount * sizeof *nor))) {
            SCE_free (pos);
            return -1;
        }
    if (me->indices[1])
        if (!(tex = SCE_malloc ((me->icount/3)*2 * sizeof *tex))) {
            SCE_free (nor);
            SCE_free (pos);
            return -1;
        }

    for (i = 0; i < me->icount; i += 3) {
        for (j = 0; j < 3; j++)
            pos[i+j] = me->pos[3*me->indices[i]+j-3];
    }
    if (nor) {
        for (i = 2; i < me->icount; i += 3) {
            for (j = 0; j < 3; j++)
                nor[i-2+j] = me->nor[3*me->indices[i]+j-3];
        }
    }
    if (tex) {
        for (i = 1; i < me->icount; i += 3) {
            tex[i-1-(i-1)/3]   = me->tex[2*me->indices[i]-2];
            tex[i-1-(i-1)/3+1] = me->tex[2*me->indices[i]-1];
        }
    }

    war_clear (me);

    me->vcount = me->icount / 3;
    me->icount = 0;
    me->pos = pos;
    me->nor = nor;
    me->tex = tex;

    return 0;
}

WarMesh* war_read (FILE *fp, int gen_indices, unsigned int lod_level)
{
    int ret = SCE_OK;
    unsigned int i = 0;
    int num_indices = 0;
    WarMesh *mesh = NULL;
    size_t v, vt, vn;
    int (*make)(WarMesh*) = NULL;
    int o_first = 1;
    unsigned int n_objs = 0;

    /* lecture des informations generales */
    war_readinfos (fp, &n_objs, &v, &vt, &vn);
    if (v == 0 && vt == 0 && vn == 0) {
        war_error ("bad file format : can't read some valid OBJ data");
        return NULL;
    } else if (n_objs == 0)
        n_objs = 1;
    rewind (fp);

    /* creation du premier objet */
    if (!(mesh = war_new ()))
        goto fail;
    war_canfree (mesh, SCE_TRUE);
    war_canfreeindices (mesh, SCE_TRUE);

    /* allocation des tableaux de donnees */
    if (!(mesh->pos = SCE_malloc (v * sizeof *mesh->pos)))
        goto fail;
    if (!(mesh->tex = SCE_malloc (vt * sizeof *mesh->tex)))
        goto fail;
    if (!(mesh->nor = SCE_malloc (vn * sizeof *mesh->nor)))
        goto fail;

    /* lecture des donnees de sommet */
    war_readdata (fp, mesh->pos, mesh->tex, mesh->nor);
    rewind (fp);

    num_indices = war_readnumindices (fp);
    switch (num_indices) { /* pour aller jusqu'au prochain 'o' */
    case -1: goto fail;
    case 0: break;
    default:
        /* donnees detectees avant le premier 'o', on considere donc que
           le premier objet n'est pas defini par un 'o' */
        rewind (fp);
        o_first = 0;
    }
    /* lecture de la taille des tableaux d'indices et allocation sur le bon */
    for (i = 0; i < n_objs; i++) {
        if ((mesh->icount = war_readnumindices (fp)) < 0)
            goto fail;
        if (mesh->icount == 0) {
            war_error ("bad file format : 0 value read for index count");
            goto fail;
        }
        if (i == lod_level) {
            mesh->indices = SCE_malloc (mesh->icount * sizeof *mesh->indices);
            if (!mesh->indices)
                goto fail;
            break;
        }
    }
    rewind (fp);
    if (o_first) {
        if (war_readnumindices (fp) < 0) /* pour aller jusqu'au prochain 'o' */
            goto fail;
    }
    /* lecture des indices */
    for (i = 0; i < n_objs; i++) {
        if (i == lod_level) {
            war_readindices (fp, mesh->indices);
            break;
        } else                    /* fake read, just jump */
            war_readindices (fp, NULL);
    }

    /* construction des meshs */
    if (gen_indices)
        ret = war_makeindices (mesh);
    else
        ret = war_makevertices (mesh);
    if (ret < 0)
        goto fail;
    return mesh;
fail:
    if (!SCEE_HaveError ())
        SCEE_Log (SCE_BAD_FORMAT);
    SCEE_LogSrc ();
    war_free (mesh);
    return NULL;
}
