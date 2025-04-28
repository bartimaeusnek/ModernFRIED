// This file is distributed under a BSD license. See LICENSE.txt for details.

// FRIED
// transform innerloops
#include "fried_internal.hpp"
#include <cstdint>

namespace FRIED {
    // new 1d dct (11A 6S)
    // b norm: 1.3260 d norm: 1.5104
    static void ndct4(int32_t &ar, int32_t &br, int32_t &cr, int32_t &dr) {
        int32_t a, b, c, d;

        a = ar;
        b = br;
        c = cr;
        d = dr;

        // stage 1
        a += d;
        c -= b;
        d <<= 1;
        d -= a;

        // stage 2
        b += (c - a) >> 1;
        a += b;

        c -= (d >> 1) - (d >> 3);
        d += (c >> 1) - (c >> 3);

        // store (with reordering!)
        ar = a;
        br = d; // !
        cr = b; // !
        dr = c; // !
    }

    // 1d wht (lifting-based)
    static void wht4(int32_t &ar, int32_t &br, int32_t &cr, int32_t &dr) {
        int32_t a, b, c, d, t;

        // load
        a = ar;
        b = br;
        c = cr;
        d = dr;

        // computation
        a += d;
        c -= b;
        t = (c - a) >> 1;
        d += t;
        b += t;
        c -= d;
        a += b;

        // store (with reordering)
        ar = a;
        br = d; // !
        cr = b; // !
        dr = c; // !
    }

    // several dct variants. ndcts generate permuted output,
    // indcts expect permuted input. all permutation handling
    // is done during coefficient reordering.
    //
    // current cost:
    //   88A 48S
    //
    // dct 4x4:  72A 24M
    // h.264 IT: 64A 16S

    void ndct42D(int32_t *x0, int32_t *x1, int32_t *x2, int32_t *x3) {
        // transpose in
        sSwap(x0[1], x1[0]);
        sSwap(x0[2], x2[0]);
        sSwap(x0[3], x3[0]);
        sSwap(x1[2], x2[1]);
        sSwap(x1[3], x3[1]);
        sSwap(x2[3], x3[2]);

        // horizontal
        ndct4(x0[0], x0[1], x0[2], x0[3]);
        ndct4(x1[0], x1[1], x1[2], x1[3]);
        ndct4(x2[0], x2[1], x2[2], x2[3]);
        ndct4(x3[0], x3[1], x3[2], x3[3]);

        // vertical
        ndct4(x0[0], x1[0], x2[0], x3[0]);
        ndct4(x0[1], x1[1], x2[1], x3[1]);
        ndct4(x0[2], x1[2], x2[2], x3[2]);
        ndct4(x0[3], x1[3], x2[3], x3[3]);
    }

    /**
     * Inverse Discrete Cosine Transform for 4x2 blocks
     *
     * This implementation processes transform coefficients using scalar operations
     * rather than MMX instructions.
     *
     * @param coeff1 First coefficient block pointer
     * @param coeff2 Second coefficient block pointer
     * @param coeff3 Third coefficient block pointer
     * @param coeff4 Fourth coefficient block pointer
     */
    void indct42D(int16_t *coeff1, int16_t *coeff2, int16_t *coeff3, int16_t *coeff4) {
        // Load the input coefficients from param_4
        int16_t in0 = *coeff4;
        int16_t in1 = coeff4[1];
        int16_t in2 = coeff4[2];
        int16_t in3 = coeff4[3];

        // First transform stage - combination with param_2
        int16_t t0 = (in0 >> 3) + (*coeff2 - (in0 >> 1));
        int16_t t1 = (in1 >> 3) + (coeff2[1] - (in1 >> 1));
        int16_t t2 = (in2 >> 3) + (coeff2[2] - (in2 >> 1));
        int16_t t3 = (in3 >> 3) + (coeff2[3] - (in3 >> 1));

        // Second transform stage
        int16_t m0 = ((t0 >> 1) + in0) - (t0 >> 3);
        int16_t m1 = ((t1 >> 1) + in1) - (t1 >> 3);
        int16_t m2 = ((t2 >> 1) + in2) - (t2 >> 3);
        int16_t m3 = ((t3 >> 1) + in3) - (t3 >> 3);

        // Load differences between param_1 and param_3
        int16_t diff0 = *coeff1 - *coeff3;
        int16_t diff1 = coeff1[1] - coeff3[1];
        int16_t diff2 = coeff1[2] - coeff3[2];
        int16_t diff3 = coeff1[3] - coeff3[3];

        // Calculate intermediate values combining param_3 and differences
        int16_t s0 = *coeff3 - ((int16_t)(m0 - diff0) >> 1);
        int16_t s1 = coeff3[1] - ((int16_t)(m1 - diff1) >> 1);
        int16_t s2 = coeff3[2] - ((int16_t)(m2 - diff2) >> 1);
        int16_t s3 = coeff3[3] - ((int16_t)(m3 - diff3) >> 1);

        // Calculate combined values
        int16_t c3 = m3 + s3;
        int16_t temp = (c3 >> 3) + ((m1 + s1) - (c3 >> 1));
        int16_t c0 = t3 + diff3;

        // More intermediate calculations
        int16_t avg1 = (int16_t)(t1 + diff1) >> 1;
        int16_t d0 = diff3 - (c0 >> 1);
        int16_t r0 = (diff1 - ((d0 >> 1) + avg1)) + (d0 >> 3);
        int16_t r1 = (s1 - (s3 >> 1)) + (s3 >> 3);
        int16_t r2 = (avg1 - (c0 >> 2)) + (c0 >> 4);

        int16_t avg2 = (int16_t)(t0 + diff0) >> 1;
        int16_t avg3 = (int16_t)(t2 + diff2) >> 1;
        int16_t d1 = (diff0 - avg2) - (diff2 - avg3);

        // Final transform stage calculations
        int16_t f0 = ((r0 >> 1) + d0) - (r0 >> 3);
        int16_t f1 = ((r1 >> 1) + s3) - (r1 >> 3);
        int16_t f2 = ((temp >> 1) + c3) - (temp >> 3);

        int16_t e0 = (diff2 - avg3) - ((int16_t)(f0 - d1) >> 1);

        int16_t c1 = m2 + s2;
        int16_t d2 = s0 - s2;
        int16_t e1 = s2 - ((int16_t)(f1 - d2) >> 1);

        int16_t d3 = (m0 + s0) - c1;
        int16_t e2 = c1 - ((int16_t)(f2 - d3) >> 1);

        // Output calculations
        int16_t avg4 = (int16_t)(r0 + d1) >> 1;
        *coeff1 = d1 - avg4;
        *coeff2 = e0;
        *coeff3 = e0 + f0;
        *coeff4 = avg4;

        int16_t avg5 = (int16_t)(r1 + d2) >> 1;
        coeff1[1] = d2 - avg5;
        coeff2[1] = e1;
        coeff3[1] = e1 + f1;
        coeff4[1] = avg5;

        int16_t avg6 = (int16_t)(temp + d3) >> 1;
        coeff1[2] = d3 - avg6;
        coeff2[2] = e2;
        coeff3[2] = e2 + f2;

        int16_t d4 = avg2 - avg3;
        int16_t avg7 = (int16_t)(r2 + d4) >> 1;
        coeff4[2] = avg6;
        coeff1[3] = d4 - avg7;

        int16_t f3 = ((r2 >> 1) + (c0 >> 1)) - (r2 >> 3);
        int16_t e3 = avg3 - ((int16_t)(f3 - d4) >> 1);

        coeff2[3] = e3;
        coeff3[3] = e3 + f3;
        coeff4[3] = avg7;
    }

    void ndct42D_MB(int32_t *x0, int32_t *x1, int32_t *x2, int32_t *x3) {
        // horizontal
        wht4(x0[0], x0[4], x0[8], x0[12]);
        wht4(x1[0], x1[4], x1[8], x1[12]);
        wht4(x2[0], x2[4], x2[8], x2[12]);
        wht4(x3[0], x3[4], x3[8], x3[12]);

        // vertical
        wht4(x0[0], x1[0], x2[0], x3[0]);
        wht4(x0[4], x1[4], x2[4], x3[4]);
        wht4(x0[8], x1[8], x2[8], x3[8]);
        wht4(x0[12], x1[12], x2[12], x3[12]);
    }

    // hardcoded now to save on call costs
    void indct42D_MB(int16_t *x0, int16_t *x1, int16_t *x2, int16_t *x3) {
        int32_t temp[16];
        int32_t i, a, b, c, d, t;

        // vertical
        for (i = 0; i < 4; i++) {
            a = x0[i * 4];
            b = x2[i * 4];
            c = x3[i * 4];
            d = x1[i * 4];

            a -= b;
            c += d;
            t = (c - a) >> 1;
            b -= t;
            d -= t;
            c += b;
            a -= d;

            temp[0 + i] = a;
            temp[4 + i] = b;
            temp[8 + i] = c;
            temp[12 + i] = d;
        }

        // horizontal
        for (i = 0; i < 4; i++) {
            a = temp[i * 4 + 0];
            b = temp[i * 4 + 2];
            c = temp[i * 4 + 3];
            d = temp[i * 4 + 1];

            a -= b;
            c += d;
            t = (c - a) >> 1;
            b -= t;
            d -= t;
            c += b;
            a -= d;

            switch (i) {
                case 0:
                    x0[0] = a;
                    x0[4] = b;
                    x0[8] = c;
                    x0[12] = d;
                    break;
                case 1:
                    x1[0] = a;
                    x1[4] = b;
                    x1[8] = c;
                    x1[12] = d;
                    break;
                case 2:
                    x2[0] = a;
                    x2[4] = b;
                    x2[8] = c;
                    x2[12] = d;
                    break;
                case 3:
                    x3[0] = a;
                    x3[4] = b;
                    x3[8] = c;
                    x3[12] = d;
                    break;
            }
        }
    }

    static void rot_pp(int32_t &u, int32_t &v) {
        v -= u;
        u <<= 1;
        u += v >> 1;
        v += u >> 1;
    }

    static void inline irot_pp(int32_t &ur, int32_t &vr) {
        int32_t u, v;

        u = ur;
        v = vr;

        v -= u >> 1;
        u -= v >> 1;
        u >>= 1;
        v += u;

        ur = u;
        vr = v;
    }

    // gain: 2 (+1bit)
    static void lbtpre1D(int32_t &a, int32_t &b, int32_t &c, int32_t &d) {
        // stage 1 butterfly
        d -= a;
        c -= b;
        a += a + d;
        b += b + c;

        // rotation
        rot_pp(c, d);

        // stage 3 butterfly
        a -= d - 1;
        b -= c;
        c += c + b + 1;
        d += d + a;

        a >>= 1;
        b >>= 1;
        c >>= 1;
        d >>= 1;
    }

    static void lbtpost1D(int16_t &ar, int16_t &br, int16_t &cr, int16_t &dr) {
        int32_t a, b, c, d;

        a = ar;
        b = br;
        c = cr;
        d = dr;

        // stage 1 butterfly
        d -= a;
        c -= b;
        a += a + d;
        b += b + c;

        // inverse rotation
        irot_pp(c, d);

        // stage 3 butterfly
        a -= d;
        b -= c;
        c += c + b;
        d += d + a;

        ar = a << 1;
        br = b << 1;
        cr = c << 1;
        dr = d << 1;
    }

    // several variants of lbt pre/postfilters
    void lbt4pre2x4(int32_t *x0, int32_t *x1) {
        lbtpre1D(x0[0], x0[1], x0[2], x0[3]);
        lbtpre1D(x1[0], x1[1], x1[2], x1[3]);
    }

    void lbt4post2x4(int16_t *x0, int16_t *x1) {
        lbtpost1D(x0[0], x0[1], x0[2], x0[3]);
        lbtpost1D(x1[0], x1[1], x1[2], x1[3]);
    }

    void lbt4pre4x2(int32_t *x0, int32_t *x1, int32_t *x2, int32_t *x3) {
        lbtpre1D(x0[0], x1[0], x2[0], x3[0]);
        lbtpre1D(x0[1], x1[1], x2[1], x3[1]);
    }

    void lbt4post4x2(int16_t *x0, int16_t *x1, int16_t *x2, int16_t *x3) {
        lbtpost1D(x0[0], x1[0], x2[0], x3[0]);
        lbtpost1D(x0[1], x1[1], x2[1], x3[1]);
    }

    void lbt4pre4x4(int32_t *x0, int32_t *x1, int32_t *x2, int32_t *x3) {
        // horizontal
        lbtpre1D(x0[0], x0[1], x0[2], x0[3]);
        lbtpre1D(x1[0], x1[1], x1[2], x1[3]);
        lbtpre1D(x2[0], x2[1], x2[2], x2[3]);
        lbtpre1D(x3[0], x3[1], x3[2], x3[3]);

        // vertical
        lbtpre1D(x0[0], x1[0], x2[0], x3[0]);
        lbtpre1D(x0[1], x1[1], x2[1], x3[1]);
        lbtpre1D(x0[2], x1[2], x2[2], x3[2]);
        lbtpre1D(x0[3], x1[3], x2[3], x3[3]);
    }

    void lbt4post4x4(int16_t *param_1, int16_t *param_2, int16_t *param_3, int16_t *param_4)
    {
        // First round of vertical transform calculations
        int16_t d0 = param_4[0] - param_1[0];
        int16_t d1 = param_4[1] - param_1[1];
        int16_t e0 = param_3[0] - param_2[0];
        int16_t e1 = param_3[1] - param_2[1];

        int16_t f0 = d0 - (e0 >> 1);
        int16_t f1 = d1 - (e1 >> 1);

        int16_t d2 = param_4[2] - param_1[2];
        int16_t e2 = param_3[2] - param_2[2];
        int16_t f2 = d2 - (e2 >> 1);

        int16_t d3 = param_4[3] - param_1[3];
        int16_t e3 = param_3[3] - param_2[3];
        int16_t f3 = d3 - (e3 >> 1);

        // Calculate adjustment values
        int16_t g0 = (int16_t)(e0 * 2 - f0) >> 2;
        int16_t g1 = (int16_t)(e1 * 2 - f1) >> 2;
        int16_t g2 = (int16_t)(e2 * 2 - f2) >> 2;
        int16_t g3 = (int16_t)(e3 * 2 - f3) >> 2;

        // Apply adjustments
        f0 = f0 + g0;
        f1 = f1 + g1;
        f2 = f2 + g2;
        f3 = f3 + g3;

        // Calculate intermediate values
        int16_t h0 = (d0 + param_1[0] * 2) - f0;
        int16_t h1 = (d1 + param_1[1] * 2) - f1;
        int16_t h2 = (d2 + param_1[2] * 2) - f2;
        int16_t h3 = (d3 + param_1[3] * 2) - f3;

        int16_t j0 = h0 + f0 * 2;

        // Horizontal transform setup for row 0
        int16_t m0 = h3 - h0;
        int16_t m1 = h2 - h1;
        int16_t n0 = m0 - (m1 >> 1);

        // Calculate for remaining rows' vertical transform
        int16_t i0 = (e0 + param_2[0] * 2) - g0;
        int16_t i1 = (e1 + param_2[1] * 2) - g1;
        int16_t i2 = (e2 + param_2[2] * 2) - g2;
        int16_t i3 = (e3 + param_2[3] * 2) - g3;

        int16_t k0 = i0 + g0 * 2;

        // Horizontal transform for row 1
        int16_t m2 = i3 - i0;
        int16_t m3 = i2 - i1;
        int16_t n1 = m2 - (m3 >> 1);

        // More intermediate values for rows 2 and 3
        int16_t j1 = h1 + f1 * 2;
        int16_t k1 = i1 + g1 * 2;
        int16_t k2 = (i3 + g3 * 2) - k0;
        int16_t k3 = (i2 + g2 * 2) - k1;
        int16_t n2 = k2 - (k3 >> 1);

        int16_t l2 = (h3 + f3 * 2) - j0;
        int16_t l3 = (h2 + f2 * 2) - j1;
        int16_t n3 = l2 - (l3 >> 1);

        // Calculate adjustments for horizontal transform
        int16_t p0 = (int16_t)(m1 * 2 - n0) >> 2;
        int16_t n0_adj = n0 + p0;
        int16_t out00 = (m0 + h0 * 2) - n0_adj;

        int16_t p1 = (int16_t)(m3 * 2 - n1) >> 2;
        int16_t p2 = (int16_t)(k3 * 2 - n2) >> 2;
        int16_t p3 = (int16_t)(l3 * 2 - n3) >> 2;

        int16_t n1_adj = n1 + p1;
        int16_t n2_adj = n2 + p2;
        int16_t n3_adj = n3 + p3;

        // Calculate output values
        int16_t out10 = (m2 + i0 * 2) - n1_adj;
        int16_t out20 = (k2 + k0 * 2) - n2_adj;
        int16_t out30 = (l2 + j0 * 2) - n3_adj;

        int16_t out01 = (m1 + h1 * 2) - p0;
        int16_t out11 = (m3 + i1 * 2) - p1;
        int16_t out21 = (k3 + k1 * 2) - p2;
        int16_t out31 = (l3 + j1 * 2) - p3;

        param_1[0] = out00;
        param_2[0] = out10;
        param_3[0] = out20;
        param_4[0] = out30;

        param_1[1] = out01;
        param_2[1] = out11;
        param_3[1] = out21;
        param_4[1] = out31;

        param_1[2] = out01 + p0 * 2;
        param_2[2] = out11 + p1 * 2;
        param_3[2] = out21 + p2 * 2;
        param_4[2] = out31 + p3 * 2;

        param_1[3] = out00 + n0_adj * 2;
        param_2[3] = out10 + n1_adj * 2;
        param_3[3] = out20 + n2_adj * 2;
        param_4[3] = out30 + n3_adj * 2;
    }
}