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

#include <SCE/utils/SCEUtils.h>

#include "SCE/core/SCENoise.h"

#define B 0x100
#define BM 0xff
#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

#define s_curve(t) ((t) * (t) * (3. - 2. * (t)) )
#define lerp(t, a, b) ((a) + (t) * ((b) - (a)) )
#define setup(i, b0, b1, r0, r1) do {           \
        t = vec[i] + N;                         \
        b0 = ((int)t) & BM;                     \
        b1 = (b0 + 1) & BM;                     \
        r0 = t - (int)t;                        \
        r1 = r0 - 1.;                           \
    } while (0)
#define at2(rx, ry) ((rx) * q[0] + (ry) * q[1] )
#define at3(rx, ry, rz) ((rx) * q[0] + (ry) * q[1] + (rz) * q[2])

static int p[B + B + 2];
static double g3[B + B + 2][3];
static double g2[B + B + 2][2];
static float g1[B + B + 2];
static int start = 1;

double SCE_Noise_Perlin1D (double arg)
{
    int bx0, bx1;
    double rx0, rx1, sx, t, u, v, vec[1];

    vec[0] = arg;

    setup (0, bx0, bx1, rx0, rx1);

    sx = s_curve (rx0);
    u = rx0 * g1[p[bx0]];
    v = rx1 * g1[p[bx1]];

    return lerp (sx, u, v);
}

double SCE_Noise_Perlin2D (double vec[2])
{
    int bx0, bx1, by0, by1, b00, b10, b01, b11;
    double rx0, rx1, ry0, ry1, *q, sx, sy, a, b, t, u, v;
    int i, j;

    setup (0, bx0, bx1, rx0, rx1);
    setup (1, by0, by1, ry0, ry1);

    i = p[bx0];
    j = p[bx1];

    b00 = p[i + by0];
    b10 = p[j + by0];
    b01 = p[i + by1];
    b11 = p[j + by1];

    sx = s_curve (rx0);
    sy = s_curve (ry0);

    q = g2[b00]; u = at2 (rx0, ry0);
    q = g2[b10]; v = at2 (rx1, ry0);
    a = lerp (sx, u, v);

    q = g2[b01]; u = at2 (rx0, ry1);
    q = g2[b11]; v = at2 (rx1, ry1);
    b = lerp (sx, u, v);

    return lerp (sy, a, b);
}

float SCE_Noise_Linear3D (double vec[3])
{
    int bx0, bx1, by0, by1, bz0, bz1, b00, b10, b01, b11;
    double t;
    float v[8];
    SCE_TVector3 w;
    int i, j;

    setup (0, bx0, bx1, w[0], i);
    setup (1, by0, by1, w[1], i);
    setup (2, bz0, bz1, w[2], i);

    i = p[bx0];
    j = p[bx1];

    b00 = p[i + by0];
    b10 = p[j + by0];
    b01 = p[i + by1];
    b11 = p[j + by1];

    v[0] = g1[b00 + bz0];
    v[1] = g1[b10 + bz0];
    v[2] = g1[b01 + bz0];
    v[3] = g1[b11 + bz0];
    v[4] = g1[b00 + bz1];
    v[5] = g1[b10 + bz1];
    v[6] = g1[b01 + bz1];
    v[7] = g1[b11 + bz1];

    return SCE_Vector3_Trilinear1 (w, v);
}

float SCE_Noise_Smooth3D (double vec[3])
{
    double a, b, c, d, e, f;

    /* TODO: floor is slow. */
    a = floor (vec[0]);
    b = floor (vec[1]);
    c = floor (vec[2]);
    d = vec[0] - a;
    e = vec[1] - b;
    f = vec[2] - c;
    d = d*d*d*(d*(d*6.0-15.0)+10.0);
    e = e*e*e*(e*(e*6.0-15.0)+10.0);
    f = f*f*f*(f*(f*6.0-15.0)+10.0);
    vec[0] = a + d;
    vec[1] = b + e;
    vec[2] = c + f;

    return SCE_Noise_Linear3D (vec);
}


static void normalize2 (double v[2])
{
    double s;

    s = sqrt(v[0] * v[0] + v[1] * v[1]);
    v[0] = v[0] / s;
    v[1] = v[1] / s;
}

static void normalize3 (double v[3])
{
    double s;

    s = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    v[0] = v[0] / s;
    v[1] = v[1] / s;
    v[2] = v[2] / s;
}

int SCE_Init_Noise (void)
{
    int i, j, k;

    for (i = 0 ; i < B ; i++) {
        p[i] = i;
        g1[i] = (double)((random() % (B + B)) - B) / B;

        for (j = 0 ; j < 2 ; j++)
            g2[i][j] = (double)((random() % (B + B)) - B) / B;
        normalize2(g2[i]);

        for (j = 0 ; j < 3 ; j++)
            g3[i][j] = (double)((random() % (B + B)) - B) / B;
        normalize3(g3[i]);
    }

    while (--i) {
        k = p[i];
        p[i] = p[j = random() % B];
        p[j] = k;
    }

    for (i = 0 ; i < B + 2 ; i++) {
        p[B + i] = p[i];
        g1[B + i] = g1[i];
        for (j = 0 ; j < 2 ; j++)
            g2[B + i][j] = g2[i][j];
        for (j = 0 ; j < 3 ; j++)
            g3[B + i][j] = g3[i][j];
    }

    return SCE_OK;
}
