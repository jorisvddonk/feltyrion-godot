/**
 * This file contains functions specific to Feltyrion.
 * In some cases, they're experimental modified versions of original functions, where modifications needed were sufficiently severe
 * that preprocessor directives wouldn't otherwise make that much sense....
 */

#include <iostream>
#include <stack>

#include "brtl.h"
#include "noctis-d.h"
#include "noctis-0.h"
#include "godot_cpp/variant/utility_functions.hpp"

extern int16_t cplx_planet_viewpoint(int16_t logical_id);

// Adaptation of `draw_planets()`; does many things similar to the original, but doesn't actually invoke any of the drawing functions, and callers get to specify which body to prepare via the target_body parameter
// This *does* invoke surface(), so after this function, it's OK to grab the planet surface textures and stuff...
void not_actually_draw_planet(int16_t target_body) {
    auto *atmosphere     = (int8_t *) objectschart;
    auto *surface_backup = (uint8_t *) p_background;
    int8_t is_moon;
    int32_t poffs;
    int32_t test;
    int16_t i1, i2, n1, n2, c1, c2, pnpcs;
    int16_t c, n, t, ts, te, ll, plwp = 0, riwp;
    int16_t te_ll_distance, te_ll_distance_1, te_ll_distance_2;
    int16_t ts_ll_distance, ts_ll_distance_1, ts_ll_distance_2;
    double xx, yy, zz;
    double d3, d2, md2 = 1E9;
    uint8_t colorbase, showdisc, showrings, surfacemap;
    int16_t ringlayers = 0;

    if (ip_targetting) {
        ip_targetted = -1;
    }

    if (!nearstar_nop) {
        return;
    }

    for (n = 0; n < nearstar_nob; n++) {
        planet_xyz(n);
        xx                       = plx - dzat_x;
        yy                       = ply - dzat_y;
        zz                       = plz - dzat_z;
        nearstar_p_qsortindex[n] = n;
        nearstar_p_qsortdist[n]  = sqrt(xx * xx + yy * yy + zz * zz);
    }

    quick_sort(nearstar_p_qsortindex, nearstar_p_qsortdist, 0, nearstar_nob - 1);

    if (nearstar_nob == 1) {
        pnpcs = npcs;
        npcs  = 0;
    } else {
        pnpcs = npcs;
        n1    = nearstar_nob - 1;
        i1    = nearstar_p_qsortindex[n1];
        c1    = nearstar_p_owner[i1];
        n2    = nearstar_nob - 2;
        i2    = nearstar_p_qsortindex[n2];
        c2    = nearstar_p_owner[i2];

        if (c1 > -1) {
            while (c2 > -1) {
                n2--;
                i2 = nearstar_p_qsortindex[n2];
                c2 = nearstar_p_owner[i2];
            }
        }

        npcs = i1 * maxbodies - i2;
    }

    for (c = 0; c < nearstar_nob; c++) {
        n = nearstar_p_qsortindex[c];
        //
        d3 = nearstar_p_qsortdist[n];
        planet_xyz(n);

        if (nearstar_p_owner[n] > -1) {
            colorbase    = 128;
            is_moon      = 1;
        } else {
            colorbase    = 192;
            is_moon      = 0;
        }

        showdisc   = 0;
        showrings  = 0;
        surfacemap = 0;

        if (n == target_body) {
            showrings  = 1;
            ringlayers = (int16_t) (0.05 * (d3 / nearstar_p_ray[n]));
            showdisc = 1;
            surfacemap = 1;
        }

        //
        if (ip_targetting || !showdisc) {
            //far_pixel_at(plx, ply, plz, nearstar_p_ray[n], 0);

            if (ip_targetting) {
                pxx -= VIEW_X_CENTER;
                pyy -= VIEW_Y_CENTER;
                d2 = pxx * pxx + pyy * pyy;

                if (d2 < md2) {
                    md2          = d2;
                    ip_targetted = n;
                }
            }
        }

        //
        if (nearstar_p_type[n] == 10) {
            goto notaplanet;
        }

        //
        if (showrings) {
            if (surfacemap) {
                //multicolourmask = 0xC0;
            } else {
                //multicolourmask = 0x40;
            }

            plwp = 359 - planet_viewpoint(dzat_x, dzat_z);

            if (nearstar_p_ring[n] != 0.0) {
                riwp = plwp + 180;

                if (riwp > 359) {
                    riwp -= 360;
                }

                //ring(n, plx, ply, plz, riwp, ringlayers);
            }
        }

        if (showdisc) {
            if (surfacemap) {
                if (true) {
                    if (is_moon) {
                        if (nearstar_p_type[n]) {
                            surface(n, nearstar_p_type[n],
                                    1000000.0 * nearstar_ray * nearstar_p_type[n] * nearstar_p_orb_orient[n], 128, true, true);
                        } else {
                            surface(n, nearstar_p_type[n], 2000000.0 * n * nearstar_ray * nearstar_p_orb_orient[n],
                                    128, true, true);
                        }
                    } else {
                        if (nearstar_p_type[n]) {
                            surface(n, nearstar_p_type[n],
                                    1000000.0 * nearstar_p_type[n] * nearstar_p_orb_seed[n] * nearstar_p_orb_tilt[n] *
                                        nearstar_p_orb_ecc[n] * nearstar_p_orb_orient[n],
                                    192, true, true);
                        } else {
                            surface(n, nearstar_p_type[n],
                                    2000000.0 * n * nearstar_p_orb_seed[n] * nearstar_p_orb_tilt[n] *
                                        nearstar_p_orb_ecc[n] * nearstar_p_orb_orient[n],
                                    192, true, true);
                        }
                    }

                }

                if (n == ip_targetted && landing_point) {
                    nightzone = 0;
                    //
                    ts = nearstar_p_term_start[n];
                    te = nearstar_p_term_end[n];
                    ll = landing_pt_lon;

                    //
                    if (ts > te) {
                        if (ll >= ts || ll < te) {
                            nightzone = 1;
                        }
                    } else {
                        if (ll >= ts && ll < te) {
                            nightzone = 1;
                        }
                    }

                    //
                    te_ll_distance_1 = 0;
                    ll               = landing_pt_lon;

                    while (ll != te) {
                        te_ll_distance_1++;
                        ll++;

                        if (ll >= 360) {
                            ll = 0;
                        }
                    }

                    te_ll_distance_2 = 0;
                    ll               = landing_pt_lon;

                    while (ll != te) {
                        te_ll_distance_2++;
                        ll--;

                        if (ll <= -1) {
                            ll = 359;
                        }
                    }

                    if (te_ll_distance_1 < te_ll_distance_2) {
                        te_ll_distance = te_ll_distance_1;
                    } else {
                        te_ll_distance = te_ll_distance_2;
                    }

                    ts_ll_distance_1 = 0;
                    ll               = landing_pt_lon;

                    while (ll != ts) {
                        ts_ll_distance_1++;
                        ll++;

                        if (ll >= 360) {
                            ll = 0;
                        }
                    }

                    ts_ll_distance_2 = 0;
                    ll               = landing_pt_lon;

                    while (ll != ts) {
                        ts_ll_distance_2++;
                        ll--;

                        if (ll <= -1) {
                            ll = 359;
                        }
                    }

                    if (ts_ll_distance_1 < ts_ll_distance_2) {
                        ts_ll_distance = ts_ll_distance_1;
                    } else {
                        ts_ll_distance = ts_ll_distance_2;
                    }

                    if (ts_ll_distance <= te_ll_distance) {
                        sun_x_factor = +1;
                        crepzone     = ts_ll_distance;
                    } else {
                        sun_x_factor = -1;
                        crepzone     = te_ll_distance;
                    }

                    fast_srand((int32_t) (nearstar_p_orb_seed[n] * nearstar_p_orb_ecc[n] * 12345));
                    ptr            = 360 * landing_pt_lat + landing_pt_lon;
                    sky_red_filter = fast_random(31) + 32;
                    sky_grn_filter = fast_random(15) + 48;
                    sky_blu_filter = fast_random(15) + 48;

                    if (is_moon) {
                        gnd_red_filter = tmppal[(3 * p_background[ptr]) + 128 * 3 + 0];
                        gnd_grn_filter = tmppal[(3 * p_background[ptr]) + 128 * 3 + 1];
                        gnd_blu_filter = tmppal[(3 * p_background[ptr]) + 128 * 3 + 2];
                    } else {
                        gnd_red_filter = tmppal[(3 * p_background[ptr]) + 192 * 3 + 0];
                        gnd_grn_filter = tmppal[(3 * p_background[ptr]) + 192 * 3 + 1];
                        gnd_blu_filter = tmppal[(3 * p_background[ptr]) + 192 * 3 + 2];
                    }

                    gnd_red_filter += fast_random(15);
                    gnd_grn_filter += fast_random(15);
                    gnd_blu_filter += fast_random(15);
                    test = nearstar_p_type[n];

                    if (nightzone) {
                        albedo = p_background[ptr];
                        albedo <<= 2u;

                        if (test == 3 || test == 5) {
                            albedo -= atmosphere[ptr >> 1u];
                        }

                        albedo >>= 2u;
                        albedo <<= 2u;
                    } else {
                        albedo = p_background[ptr];

                        if (test == 3 || test == 5) {
                            albedo -= atmosphere[ptr >> 1u];
                        }

                        albedo >>= 2u;
                        albedo <<= 2u;
                    }

                    if (test == 3 || test == 5) {
                        albedo *= 2; // da 0 a 1F --> da 0 a 3F
                    }

                    rainy = (float) atmosphere[ptr >> 1u] * 0.25;

                    if (rainy > 5) {
                        rainy = 5;
                    }

                    if (nightzone) {
                        sky_grn_filter /= 2;
                        sky_blu_filter /= 2;

                        if (crepzone > 5) {
                            sky_red_filter /= 2;
                            sky_brightness = 8;
                        } else {
                            sky_brightness = 32;
                        }
                    } else {
                        if (crepzone > 5) {
                            sky_brightness = 48;
                        } else {
                            sky_grn_filter /= 2;
                            sky_blu_filter /= 3;
                            sky_brightness = 40;
                        }
                    }

                    for (poffs = -180; poffs < 180; poffs++) {
                        test = poffs + ptr;

                        if (test > 0 && test < 64800) {
                            p_background[test] ^= 0x1Eu;
                        }
                    }

                    for (poffs = -60; poffs < 60; poffs++) {
                        if (poffs) {
                            test = 360 * poffs + ptr;

                            if (test > 0 && test < 64800) {
                                p_background[test] ^= 0x1Eu;
                            }
                        }
                    }
                }

                t = nearstar_p_type[n];

                if (t == 2 || t == 3 || t == 6 || t == 8 || t == 9) {
                    glass_bubble = 1;
                } else {
                    glass_bubble = 0;
                }

                //globe(plwp + nearstar_p_rotation[n], adapted, p_background, (uint8_t *) n_globes_map, gl_bytes, plx,
                //      ply, plz, nearstar_p_ray[n], colorbase, 0);

                if (n == ip_targetted && landing_point) {
                    for (poffs = -180; poffs < 180; poffs++) {
                        test = poffs + ptr;

                        if (test > 0 && test < 64800) {
                            p_background[test] ^= 0x1Eu;
                        }
                    }

                    for (poffs = -60; poffs < 60; poffs++) {
                        if (poffs) {
                            test = 360 * poffs + ptr;

                            if (test > 0 && test < 64800) {
                                p_background[test] ^= 0x1Eu;
                            }
                        }
                    }
                }
            } else {
                ts = (89 + 35) - cplx_planet_viewpoint(n);

                if (ts < 0) {
                    ts += 360;
                }

                if (ts > 359) {
                    ts -= 360;
                }

                //glowing_globe(plwp, adapted, (uint8_t *) n_globes_map, gl_bytes, plx, ply, plz, nearstar_p_ray[n], ts,
                //              130, 127);
            }
        }

        if (showrings) {
            if (nearstar_p_ring[n] != 0.0) {
                //ring(n, plx, ply, plz, plwp, ringlayers);

                if (!showdisc) {
                    //far_pixel_at(plx, ply, plz, nearstar_p_ray[n], 0);
                }
            }
        }

    notaplanet:
        int abc123;
    }

    p_background = surface_backup;
}

