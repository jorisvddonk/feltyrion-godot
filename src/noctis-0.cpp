/*

    Noctis.
    -------
    NOTE: This bit was something irrelevant that was too hard to translate.|

    Noctis: Feline People.
    ----------------------------
    Space: The Final Frontier.
    These are the journeys of the cosmic rafts of the feline people.
    Their mission (missions?) is to explore strange, new worlds,
    looking for territory to conquer, and eliminating any new forms
    of life or new civilization... to get there... where no feline has
    been before.

    Story.
    -------
    Complicated. The program is complciated: maybe a little too much,
    certainly more than usual. If you do not find a way to compile it
    (we are at 6900 lines only with the main source), you will have to
    modularize it (Never done before, very laborious and very unpleasant).
    or find another compiler. Currently Implemented: Spaceships, stars
    and planets, an on-board manager, user reflections, planet surfaces,
    atmospheric noises (wind, rain, thunder NOTE: Where?!?!|). NOTE: Last
    sentence made no sense.| -> 27.1.97
    -------
    The program has been split into two modules. -> 28.1.97

    Module Containing the Base Functions of Noctis
    ----------------------------------------
    The project consists of NOCTIS-0.CPP, NOCTIS.CPP, and a file including
    definitions common to the two modules.

*/

#include <iostream>
#include <stack>

#include "brtl.h"
#include "noctis-d.h"
#include "godot_cpp/variant/utility_functions.hpp"

// HSP Inclusions.
#ifndef WITH_GODOT
#include "tdpolygs.h" // 3D Engine.
#else
#include "tdpolygs_stub.h"
#include "additional_math.h"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/label.hpp"
#include "godot_cpp/classes/object.hpp"
extern void cb_RingParticleFound(double xlight, double ylight, double zlight, double radii, int unconditioned_color);
#endif

// Support files

const char *situation_file = "data/current.bin";
const char *starmap_file   = "data/STARMAP.BIN";
const char *goesoutputfile = "data/GOESfile.txt";
const char *surface_file   = "data/surface.bin";

// Date and specific functions imported from ASSEMBLY.H

uint16_t QUADWORDS = 16000;

// Video memory. Because Noctis was originally written to use Mode 0x13, this
// represents a sequence of 64,000 color indices.
uint8_t *adapted;

uint8_t tmppal[768];
uint8_t currpal[768];
int8_t return_palette[768];
int8_t surface_palette[768];

static std::stack<int16_t> keys;

// Wait for a key?
int16_t get_key() {
    if (keys.empty())
        return 0;
    int16_t top = keys.top();
    keys.pop();
    return top;
}

// Return 1 if there is a key press to be processed.
bool is_key() { return !keys.empty(); }

uint8_t range8088[64 * 3] = {
    0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  3,  3,  4,  4,  4,  5,  5,  5,  6,  6,  6,  7,  7,  7,  8,  8,  8,  9,
    9,  9,  10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15, 15, 16, 16, 16, 17, 17, 17, 18, 18,
    18, 19, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26, 26, 27, 27, 27,
    28, 28, 28, 29, 29, 29, 30, 30, 30, 31, 31, 31, 32, 32, 32, 33, 33, 33, 34, 34, 34, 35, 35, 35, 36, 36, 36, 37,
    37, 37, 38, 38, 38, 39, 39, 39, 40, 40, 40, 41, 41, 41, 42, 42, 42, 43, 43, 43, 44, 44, 44, 45, 45, 45, 46, 46,
    46, 47, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 53, 53, 53, 54, 54, 54, 55, 55, 55,
    56, 56, 56, 57, 57, 57, 58, 58, 58, 59, 59, 59, 60, 60, 60, 61, 61, 61, 62, 62, 62, 63, 63, 63};

// This sets up the vga color palette.
void tavola_colori(const uint8_t *new_palette, uint16_t starting_color, uint16_t num_colors, int8_t red_filter,
                   int8_t green_filter, int8_t blue_filter) {
    int16_t c, cc = 0;
    uint16_t temp;
    num_colors *= 3;
    starting_color *= 3;
    c = starting_color;

    while (cc < num_colors) {
        tmppal[c] = new_palette[cc];
        cc++;
        c++;
    }

    c = starting_color;

    while (c < num_colors + starting_color) {
        temp = tmppal[c];
        temp *= red_filter;
        temp /= 63;

        if (temp > 63) {
            temp = 63;
        }

        tmppal[c] = temp;
        c++;
        temp = tmppal[c];
        temp *= green_filter;
        temp /= 63;

        if (temp > 63) {
            temp = 63;
        }

        tmppal[c] = temp;
        c++;
        temp = tmppal[c];
        temp *= blue_filter;
        temp /= 63;

        if (temp > 63) {
            temp = 63;
        }

        tmppal[c] = temp;
        c++;
    }

    for (uint16_t i = 0; i < (starting_color + num_colors); i++) {
        currpal[i] = tmppal[i];
    }
}

// Variables to hold mouse readings.
int16_t mdltx = 0, mdlty = 0, mouse_x = 0, mouse_y = 0;
uint16_t mpul = 0;

// Holds directions of WASD movement
struct wasdmov key_move_dir;

// Clears a rectangular region of the video memory.
// Either x2 & y2 OR l and h must be specified.
// This may or may not work.
void area_clear(uint8_t *dest, int32_t x, int32_t y, int32_t x2, int32_t y2, int32_t l, int32_t h, uint8_t pattern) {
    if (l == 0 || h == 0) {
        l = x2 - x;
        h = y2 - y;
    }

    for (int32_t xPos = x; xPos < x + l; xPos++) {
        for (int32_t yPos = y; yPos < y + h; yPos++) {
            uint32_t netIndex = (yPos * adapted_width) + xPos;
            dest[netIndex]    = pattern;
        }
    }
}

/*

    Another gem: smooth the screen, softening the contrast of the edges by
    averaging 4 x 4 groups of pixels. Pay attention to the trick: Normally there
    would be 16 additions and one division (for 16) to do, for each point. With
    the trick, however, only four 32-bit additions are needed, and another four
    with 8 bits, plus a pair of shifts.

    Normal: 16 + 42 = 58 Cycles.
    Now: 8 + 4 = 12 Cycles.

    Quite obvious: The color table must be unique and compact, to be included in
    the first 64 registers of the video DAC; the color zero is the smallest, 63
    the brightest.

    Note: This procedure can be repeated to obtain greater attenuation.
*/

void psmooth_grays(uint8_t *target) {
    uint16_t count = (QUADWORDS << 2u) - (320u << 2u);
    uint32_t index = 0;
    for (uint16_t i = 0; i < count; i++, index++) {
        uint8_t smoothed;
        uint32_t temp;

        temp = (target[index + 3] << 24u) + (target[index + 2] << 16u) + (target[index + 1] << 8u) + target[index];

        temp += (target[index + 323] << 24u) + (target[index + 322] << 16u) + (target[index + 321] << 8u) +
                target[index + 320];

        temp += (target[index + 643] << 24u) + (target[index + 642] << 16u) + (target[index + 641] << 8u) +
                target[index + 641];

        temp += (target[index + 963] << 24u) + (target[index + 962] << 16u) + (target[index + 961] << 8u) +
                target[index + 960];

        temp &= 0xFCFCFCFC;
        temp >>= 2u;

        smoothed = temp & 0xFFu;
        smoothed += temp & 0xFF00u;

        temp >>= 16u;

        smoothed += temp & 0xFFu;
        smoothed += temp & 0xFF00u;
        smoothed >>= 2u;
        target[index + 320] = smoothed;
    }
}

void psmooth_grays_ex(uint8_t *target) {
    uint32_t count = (QUADWORDS << 2u) - ((adapted_width) << 2u);
    uint32_t index = 0;
    for (uint32_t i = 0; i < count; i++, index++) {
        uint8_t smoothed;
        uint32_t temp;

        temp = (target[index + 3] << 24u) + (target[index + 2] << 16u) + (target[index + 1] << 8u) + target[index];

        temp += (target[index + adapted_width + 3] << 24u) + (target[index + adapted_width + 2] << 16u) +
                (target[index + adapted_width + 1] << 8u) + target[index + adapted_width];

        temp += (target[index + (adapted_width * 2) + 3] << 24u) + (target[index + (adapted_width * 2) + 2] << 16u) +
                (target[index + (adapted_width * 2) + 1] << 8u) + target[index + adapted_width * 2];

        temp += (target[index + (adapted_width * 3) + 3] << 24u) + (target[index + (adapted_width * 3) + 2] << 16u) +
                (target[index + (adapted_width * 3) + 1] << 8u) + target[index + (adapted_width * 3)];

        temp &= 0xFCFCFCFC;
        temp >>= 2u;

        smoothed = temp & 0xFFu;
        smoothed += temp & 0xFF00u;

        temp >>= 16u;

        smoothed += temp & 0xFFu;
        smoothed += temp & 0xFF00u;
        smoothed >>= 2u;
        target[index + adapted_width] = smoothed;
    }
}

// Produces the fading effect seen during vimana flight.
void pfade(uint8_t *target, uint16_t segshift, uint8_t speed) {
    // Don't know why count is set as it is.
    uint16_t count   = (QUADWORDS - 80u) << 2u;
    uint8_t *shifted = (target + segshift * 16);
    // Quasi-offset might need to be cleared.

    for (uint16_t i = 0; i < count; i++) {
        uint8_t color = shifted[i];
        color &= 0x3Fu;

        if (speed < color) {
            color -= speed;
        } else {
            color = 0;
        }

        shifted[i] = color;
    }
}

// Color version: 4 shades of 64 intensity each.
void psmooth_64(uint8_t *target, uint16_t segshift) {
    // Who knows why this is offset as it is... Definitely not me.
    uint16_t count = (QUADWORDS - 80u) << 2u;
    // We might need to align the shifted pointer to a 16 byte interval to match
    // the former offset clearing. Sketchy.
    uint8_t *shifted = (target + (segshift * 16));

    uint8_t avg, orig;
    for (uint32_t i = 0; i < count; i++) {
        orig = shifted[i + 320] & 0xC0u;
        avg  = (((shifted[i + 320] & 0x3Fu) + (shifted[i + 640] & 0x3Fu)) +
               ((shifted[i + 321] & 0x3Fu) + (shifted[i + 641] & 0x3Fu))) /
              4;

        shifted[i] = avg | orig;
    }
}

void psmooth_64_ex(uint8_t *target, uint16_t segshift) {
    // Who knows why this is offset as it is... Definitely not me.
    uint32_t count = (adapted_width * adapted_height) - (80 * 4);

    uint8_t *shifted = (target + (segshift * 16));

    uint8_t avg, orig;
    for (uint32_t i = 0; i < count; i++) {
        orig = shifted[i + adapted_width] & 0xC0u;
        avg  = (((shifted[i + adapted_width] & 0x3Fu) + (shifted[i + adapted_width * 2] & 0x3Fu)) +
               ((shifted[i + adapted_width + 1] & 0x3Fu) + (shifted[i + adapted_width * 2 + 1] & 0x3Fu))) /
              4;

        shifted[i] = avg | orig;
    }
}

// Circular version of the smoothing process.
// Used on the white corners on the hud and on planets.
void smootharound_64(uint8_t *target, int32_t cx, int32_t cy, int32_t r, int8_t diffuse) {
    int32_t x1 = cx - r, y1 = cy - r;
    int32_t x2 = cx + r, y2 = cy + r;
    int32_t px, py, rs = r * r;
    uint32_t cp;

    if (r <= 0 || x1 > adapted_width - 2 || y1 > adapted_height - 2 || x2 < 0 || y2 < 0) {
        return;
    }

    if (y1 < 0) {
        y1 = 0;
    }

    if (x2 > adapted_width - 2) {
        x2 = adapted_width - 2;
    }

    if (y2 > adapted_height - 2) {
        y2 = adapted_height - 2;
    }

    py = -r;

    while (y1 <= y2) {
        px = -r;
        x1 = cx - r;

        if (x1 < 0) {
            px -= x1;
            x1 = 0;
        }

        cp = (adapted_width * y1) + x1;

        if (diffuse) {
            while (x1 <= x2) {
                if (px * px + py * py < rs) {
                    uint8_t colors[4], colormasks[4];

                    colors[0] = target[cp];
                    colors[1] = target[cp + 1];
                    colors[2] = target[cp + adapted_width];
                    colors[3] = target[cp + adapted_width + 1];

                    memcpy(colormasks, colors, 4 * sizeof(uint8_t));
                    for (unsigned char &colormask : colormasks) {
                        colormask &= 0xC0u;
                    }

                    for (unsigned char &color : colors) {
                        color &= 0x3Fu;
                    }

                    colors[0] += colors[2];
                    colors[1] += colors[3];
                    colors[0] += colors[1];

                    colors[0] >>= 2u;

                    colors[1] = colors[0];
                    colors[2] = colors[0];
                    colors[3] = colors[1];

                    for (uint16_t i = 0; i < 4; i++) {
                        colors[i] |= colormasks[i];
                    }

                    target[cp]                     = colors[0];
                    target[cp + 1]                 = colors[1];
                    target[cp + adapted_width]     = colors[2];
                    target[cp + adapted_width + 1] = colors[3];
                }
                cp++;
                px++;
                x1++;
            }
        } else {
            while (x1 <= x2) {
                if (px * px + py * py < rs) {
                    uint8_t colors[4];

                    colors[0] = target[cp];
                    colors[1] = target[cp + 1];
                    colors[2] = target[cp + adapted_width];
                    colors[3] = target[cp + adapted_width + 1];

                    uint8_t temp = colors[0];
                    temp &= 0xC0u;

                    for (unsigned char &color : colors) {
                        color &= 0x3Fu;
                    }

                    colors[0] += colors[2];
                    colors[1] += colors[3];
                    colors[0] += colors[1];

                    colors[0] >>= 2u;
                    colors[0] |= temp;

                    target[cp] = colors[0];
                }

                cp++;
                px++;
                x1++;
            }
        }

        py++;
        y1++;
    }
}

// Using 64 levels in 4 shades, bring the screen to a single gradient.
void mask_pixels(uint8_t *target, uint8_t mask) {
    uint8_t cap = 0x3F;

    // If QUADWORDS is not cast to a long here it overflows and the loop hangs.
    for (uint32_t i = 0; i < ((uint32_t) QUADWORDS) * 4; i++) {
        uint8_t color = target[i];
        color &= cap;
        color += mask;
        target[i] = color;
    }
}

// Extended resolution version of mask_pixels()
void mask_pixels_ex(uint8_t *target, uint32_t offset, uint8_t mask) {
    uint8_t cap = 0x3F;

    for (uint32_t i = offset; i < adapted_width * adapted_height; i++) {
        uint8_t color = target[i];
        color &= cap;
        color += mask;
        target[i] = color;
    }
}

int16_t sfh; // Surface situation file handle.

// Global variables that are saved.

int8_t nsync                          = 1;         // 0
int8_t anti_rad                       = 1;         // 1
int8_t pl_search                      = 0;         // 2
int8_t field_amplificator             = 0;         // 3
int8_t ilight                         = 63;        // 4
int8_t ilightv                        = 1;         // 5
int8_t charge                         = 120;       // 6
int8_t revcontrols                    = 0;         // 7
int8_t ap_targetting                  = 0;         // 8
int8_t ap_targetted                   = 0;         // 9
int8_t ip_targetting                  = 0;         // 10
int8_t ip_targetted                   = -1;        // 11
int8_t ip_reaching                    = 0;         // 12
int8_t ip_reached                     = 0;         // 13
int8_t ap_target_spin                 = 0;         // 14
int8_t ap_target_r                    = 0;         // 15
int8_t ap_target_g                    = 0;         // 16
int8_t ap_target_b                    = 0;         // 17
int8_t nearstar_spin                  = 0;         // 18
int8_t nearstar_r                     = 0;         // 19
int8_t nearstar_g                     = 0;         // 20
int8_t nearstar_b                     = 0;         // 21
int8_t gburst                         = 0;         // 22
int8_t menusalwayson                  = 1;         // 23
int8_t depolarize                     = 0;         // 24
int16_t sys                           = 4;         // 25
int16_t pwr                           = 20000;     // 27
int16_t dev_page                      = 0;         // 29
int16_t ap_target_class               = 0;         // 31
int16_t f_ray_elapsed                 = 0;         // 33
int16_t nearstar_class                = 0;         // 35
int16_t nearstar_nop                  = 0;         // 37
float pos_x                           = 0;         // 39
float pos_y                           = 0;         // 43
float pos_z                           = -500;      // 47
float user_alfa                       = 0;         // 51
float user_beta                       = 0;         // 55
float navigation_beta                 = 0;         // 59
float ap_target_ray                   = 1000;      // 63
float nearstar_ray                    = 1000;      // 67
double dzat_x                         = +3797120;  // 71
double dzat_y                         = -4352112;  // 79
double dzat_z                         = -925018;   // 87
double ap_target_x                    = 0;         // 95
double ap_target_y                    = 1E8;       // 103
double ap_target_z                    = 0;         // 111
double nearstar_x                     = 0;         // 119
double nearstar_y                     = 1E8;       // 127
double nearstar_z                     = 0;         // 135
double helptime                       = 0;         // 143
double ip_target_initial_d            = 1E8;       // 151
double requested_approach_coefficient = 1;         // 159
double current_approach_coefficient   = 1;         // 167
double reaction_time                  = 0.01;      // 175
int8_t fcs_status[11]                 = "STANDBY"; // 183
int16_t fcs_status_delay              = 0;         // 194
int16_t psys                          = 4;         // 196
double ap_target_initial_d            = 1E8;       // 198
double requested_vimana_coefficient   = 1;         // 206
double current_vimana_coefficient     = 1;         // 214
double vimana_reaction_time           = 0.01;      // 222
int8_t lithium_collector              = 0;         // 230
int8_t autoscreenoff                  = 0;         // 231
int8_t ap_reached                     = 0;         // 232
int16_t lifter                        = 0;         // 233
double secs                           = 0;         // 235
int8_t data                           = 0;         // 243
int8_t surlight                       = 16;        // 244

// Surface landing control data.

int8_t land_now        = 0;
int8_t landing_point   = 0;
int16_t landing_pt_lon = 0;
int16_t landing_pt_lat = 60;

int16_t crepzone;
int16_t nightzone;
int16_t sun_x_factor;

// Global data that isn't saved.
int16_t epoc = 6011;

int8_t ctb[512];
char dec[20];

int8_t _delay     = 12;
int8_t stspeed    = 0;
int8_t elight     = 0;
uint16_t gl_start = 0;
uint16_t point;
uint32_t vptr;
int16_t infoarea  = 0;
int16_t s_control = 1;
int16_t s_command = 0;
int16_t isecs, p_isecs;
double fsecs;
int16_t gl_fps     = 1;
int16_t fps        = 1;
float dlt_alfa     = 0;
float dlt_beta     = 0;
float dlt_nav_beta = 0;
float step         = 0;
float shift        = 0;
double s_m         = 1000;
double plx, ply, plz;
double pxx, pyy;
double delta_x, delta_y;
double nearstar_identity;
int16_t nearstar_labeled;
int16_t nearstar_nob = 0;
int16_t npcs, resident_map1, resident_map2;
int8_t ontheroof;
int16_t datasheetscroll = 0;
int16_t datasheetdelta  = 0;

// Constant data in the global segment.

// Some ordinals (from 0 to 20) for certain representations.
const char *ord[21] = {"zeroth",     "first",     "second",    "third",       "fourth",     "fifth",      "sixth",
                       "seventh",    "eight",     "nineth",    "tenth",       "eleventh",   "twelveth",   "thiteenth",
                       "fourteenth", "fifteenth", "sixteenth", "seventeenth", "eighteenth", "nineteenth", "twentyth"};

const char *star_description[star_classes] = {"medium size, yellow star, suitable for planets having indigenous "
                                              "lifeforms.",
                                              "very large, blue giant star, high energy radiations around.",
                                              "white dwarf star, possible harmful radiations.",
                                              "very large, ancient, red giant star.",
                                              "large and glowing, orange giant star, high nuclear mass.",
                                              "small, weak, cold, brown dwarf substellar object.",
                                              "large, very weak, very cold, gray giant dead star.",
                                              "very small, blue dwarf star, strong gravity well around.",
                                              "possible MULTIPLE system - planets spread over wide ranges.",
                                              "medium size, surrounded by gas clouds, young star.",
                                              "very large and ancient runaway star, unsuitable for planets.",
                                              "tiny pulsar object, unsafe, high radiation, strong gravity."};

int8_t class_rgb[3 * star_classes] = {63, 58, 40, 30, 50, 63, 63, 63, 63, 63, 30, 20, 63, 55, 32, 32, 16, 10,
                                      32, 28, 24, 10, 20, 63, 63, 32, 16, 48, 32, 63, 40, 10, 10, 00, 63, 63};

int16_t class_ray[star_classes] = {5000, 15000, 300, 20000, 15000, 1000, 3000, 2000, 4000, 1500, 30000, 250};

int16_t class_rayvar[star_classes] = {2000, 10000, 200, 15000, 5000, 1000, 3000, 500, 5000, 10000, 1000, 10};

int8_t class_planets[star_classes] = {12, 18, 8, 15, 20, 3, 0, 1, 7, 20, 2, 5};

int8_t nearstar_p_type[maxbodies];
int16_t nearstar_p_owner[maxbodies];
int8_t nearstar_p_moonid[maxbodies];
double nearstar_p_ring[maxbodies];
double nearstar_p_tilt[maxbodies];
double nearstar_p_ray[maxbodies];
double nearstar_p_orb_ray[maxbodies];
double nearstar_p_orb_seed[maxbodies];
double nearstar_p_orb_tilt[maxbodies];
double nearstar_p_orb_orient[maxbodies];
double nearstar_p_orb_ecc[maxbodies];
double nearstar_p_plx[maxbodies];
double nearstar_p_ply[maxbodies];
double nearstar_p_plz[maxbodies];
double nearstar_p_seedval[maxbodies];
double nearstar_p_identity[maxbodies];
double stars_visible[8232]; // x/y/z coordinates of stars that are visible; 14 * 14 * 14 * 3

int16_t nearstar_p_rtperiod[maxbodies];
int16_t nearstar_p_rotation[maxbodies];
int16_t nearstar_p_viewpoint[maxbodies]; // added by JORIS, to store the viewpoint location in
int16_t nearstar_p_term_start[maxbodies];
int16_t nearstar_p_term_end[maxbodies];

int16_t nearstar_p_qsortindex[maxbodies];
float nearstar_p_qsortdist[maxbodies];

const char *planet_description[] = {"medium size, internally hot, unstable surface, no atmosphere.",
                                    "small, solid, dusty, craterized, no atmosphere.",
                                    "medium size, solid, thick atmosphere, fully covered by clouds.",
                                    "medium size, felisian, breathable atmosphere, suitable for life.",
                                    "medium size, rocky, creased, no atmosphere.",
                                    "small, solid, thin atmosphere.",
                                    "large, not consistent, covered with dense clouds.",
                                    "small, solid, icy surface, no atmosphere.",
                                    "medium size, surface is mainly native quartz, oxygen atmosphere.",
                                    "very large, substellar object, not consistent.",
                                    "companion star - not a planet"};

uint8_t planet_rgb_and_var[] = {60, 30, 15, 20, 40, 50, 40, 25, 32, 32, 32, 32, 16, 32, 48, 40, 32, 40, 32, 20, 32, 32,
                                32, 32, 32, 32, 32, 32, 32, 40, 48, 24, 40, 40, 40, 30, 50, 25, 10, 20, 40, 40, 40, 40};

int16_t planet_possiblemoons[] = {1, 1, 2, 3, 2, 2, 18, 2, 3, 20, 20};

const double planet_orb_scaling = 5.0;
const double avg_planet_sizing  = 2.4;
const double moon_orb_scaling   = 12.8;
const double avg_moon_sizing    = 1.8;

double avg_planet_ray[] = {0.007, 0.003, 0.010, 0.011, 0.010, 0.008, 0.064, 0.009, 0.012, 0.125, 5.000};

float mindiff = 0.01;

// Physical and logical video matrices, cartography, and other memory blocks.

uint8_t *s_background;
uint8_t *p_background;
uint8_t *p_surfacemap;
quadrant *objectschart;
uint8_t *ruinschart; // As objectschart, but declared in bytes.
uint8_t *pvfile;
uint8_t *n_offsets_map;
int8_t *n_globes_map;

// Planetary surface adjustment data.

int8_t sky_red_filter = 63; // Filters for the sky.
int8_t sky_grn_filter = 63;
int8_t sky_blu_filter = 63;
int8_t gnd_red_filter = 63; // Filters for the ground.
int8_t gnd_grn_filter = 63;
int8_t gnd_blu_filter = 63;

float planet_grav = 150; // Corresponds to the planetary gravity.
float rainy       = 0;   // Raininess, based on cloud albedo. [0,5]
int16_t albedo    = 0;   // Albedo average of the landing surface. [0,62]

uint8_t sky_brightness = 32; // Sky luminosity. [0,48]

uint16_t m200[200]; // Numbers from 0 to 199, multipled by 200 (Lookup table)

float rwp     = 15; // Required wind power (in knots).
float iwp     = 0;  // Ideal wind power (from 0 to 1).
float wp      = 0;  // Current wind power.
float wdir    = 0;  // Wind origin direction.
float wdirsin = 0;  // Optimization value.
float wdircos = 1;  // Optimization value.

int8_t landed; // Flag set at the time of landing.
// Coded position quotiento of the lander (quotient:remainder).
int32_t atl_x, atl_z, atl_x2, atl_z2;

double qid = 1.0 / 16384; // Constant to find the x / z id of a quadrant.

// Polygonal graphics area (characters, ships, labels, etc...)

#define handles 16 // 16 handles in this case.

uint32_t pvfile_datatop = 0; // Top of the data.

// Note: dataptr is a relative pointer that can go from 0 to "pvbytes".
uint16_t pvfile_dataptr[handles] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint16_t pvfile_datalen[handles] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint16_t pvfile_npolygs[handles] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int8_t *pv_n_vtx[handles];  // Number of vertices for each polygon (3 or 4).
float *pvfile_x[handles];   // X coordinate (four vertices) of each polygon.
float *pvfile_y[handles];   // Y coordinate (four vertices) of each polygon.
float *pvfile_z[handles];   // Z coordinate (four vertices) of each polygon.
int8_t *pvfile_c[handles];  // Intensity of the color of each polygon (0 to 63).
float *pv_mid_x[handles];   // X coordinate of the midpoint of each polygon.
float *pv_mid_y[handles];   // Y coordinate of the midpoint of each polygon.
float *pv_mid_z[handles];   // Z coordinate of the midpoint of each polygon.
float *pv_mid_d[handles];   // Buffer distance of average points from the observer.
int16_t *pv_dep_i[handles]; // Distance index (sorting of polygons).

// Pseudo-random number generation procedures.

int32_t flat_rnd_seed;

// Pseudo table selection.
// There are 4,294,967,295 possible tables, and about 20000 elements per table.
void fast_srand(int32_t seed) { flat_rnd_seed = ((uint32_t) seed) | 0x03u; }

// Extraction of a number: "mask" activates the bits.
// This is very sketchy!
int32_t fast_random(int32_t mask) {
    uint32_t eax = flat_rnd_seed;
    uint32_t edx = flat_rnd_seed;

    uint64_t result = ((uint64_t) eax) * ((uint64_t) edx);
    eax             = (result & 0xFFFFFFFFu);
    edx             = ((result >> 32u) & 0xFFFFFFFFu);
    uint8_t al      = (eax & 0xFFu);
    uint8_t dl      = (edx & 0xFFu);
    al += dl;
    eax = (eax & 0xFFFFFF00u) | al;
    flat_rnd_seed += eax;

    int32_t num = eax & ((uint32_t) (mask));
    return num;
}

int16_t ranged_fast_random(int16_t range) {
    if (range <= 0) {
        range = 1;
    }

    return (fast_random(0x7FFF) % range);
}

float flandom() { return ((float) brtl_random(32767) * 0.000030518); }

float fast_flandom() { return ((float) fast_random(32767) * 0.000030518); }

// Loads virtual file handles from supports.nct
FILE *sa_open(int32_t offset_of_virtual_file) {
    FILE *fh = fopen("res/supports.nct", "rb");

    if (fh == nullptr) {
        return nullptr;
    }

    if (fseek(fh, offset_of_virtual_file, SEEK_END) > -1) {
        return fh;
    } else {
        fclose(fh);
        return nullptr;
    }
}

// Defines a part of the color table so that a gradual gradient goes from one
// color to another, within a certain number of intermediate colors.
void shade(uint8_t *palette_buffer, int16_t first_color, int16_t number_of_colors, float start_r, float start_g,
           float start_b, float finish_r, float finish_g, float finish_b) {
    int16_t count = number_of_colors;
    float k       = 1.00 / (float) number_of_colors;
    float delta_r = (finish_r - start_r) * k;
    float delta_g = (finish_g - start_g) * k;
    float delta_b = (finish_b - start_b) * k;
    first_color *= 3;

    while (count) {
        if (start_r >= 0 && start_r < 64) {
            palette_buffer[first_color + 0] = (uint8_t) start_r;
        } else {
            if (start_r > 0) {
                palette_buffer[first_color + 0] = 63;
            } else {
                palette_buffer[first_color + 0] = 00;
            }
        }

        if (start_g >= 0 && start_g < 64) {
            palette_buffer[first_color + 1] = (uint8_t) start_g;
        } else {
            if (start_g > 0) {
                palette_buffer[first_color + 1] = 63;
            } else {
                palette_buffer[first_color + 1] = 00;
            }
        }

        if (start_b >= 0 && start_b < 64) {
            palette_buffer[first_color + 2] = (uint8_t) start_b;
        } else {
            if (start_b > 0) {
                palette_buffer[first_color + 2] = 63;
            } else {
                palette_buffer[first_color + 2] = 00;
            }
        }

        start_r += delta_r;
        start_g += delta_g;
        start_b += delta_b;

        first_color += 3;
        count--;
    }
}

// Sets the 3d projection from a still viewpoint.
void proj_from_vehicle() {
    cam_x = dzat_x;
    cam_y = dzat_y;
    cam_z = dzat_z;
    alfa  = user_alfa;
    beta  = user_beta + navigation_beta + 180;

    if (beta >= 360) {
        beta -= 360;
    }

    change_angle_of_view();
}

// Sets the 3d projection from the user's point of view.
void proj_from_user() {
    cam_x = pos_x;
    cam_y = pos_y;
    cam_z = pos_z;
    alfa  = user_alfa;
    beta  = user_beta;
    change_angle_of_view();
}

// Calculate the position of a certain planet in orbit around a star (the
// neighbouring one) based on orbital parameters (inclination, eccentricity,
// radius and orientation of major axis), and returns i values in plx, ply, plz.
double mox, moy, moz;
void moonorigin(int16_t n) {
    double xx;
    double p_riv, ors;
    double alfa, beta;
    ors   = nearstar_p_orb_ray[n] * nearstar_p_orb_ray[n];
    p_riv = sqrt(s_m / ors);
    beta  = (secs * p_riv * M_PI) / 180;
    alfa  = nearstar_p_orb_tilt[n] * deg;
    xx    = -nearstar_p_orb_ray[n] * sin(beta) * cos(alfa);
    moz   = nearstar_p_orb_ray[n] * cos(beta) * cos(alfa);
    moy   = nearstar_p_orb_ray[n] * sin(alfa);
    moz *= nearstar_p_orb_ecc[n];
    beta = nearstar_p_orb_orient[n];
    mox  = xx * cos(beta) + moz * sin(beta);
    moz  = moz * cos(beta) - xx * sin(beta);
}

void planet_xyz(int16_t n) {
    double xx;
    double alfa, beta;
    double p_m, p_riv, ors;
    ors = nearstar_p_orb_ray[n] * nearstar_p_orb_ray[n];

    if (nearstar_p_owner[n] > -1) {
        xx    = nearstar_p_ray[nearstar_p_owner[n]];
        p_m   = qt_M_PI * xx * xx * xx * 0.44e-4;
        p_riv = sqrt(p_m / ors);
    } else {
        p_riv = sqrt(s_m / ors);
    }

    beta = (secs * p_riv * M_PI) / 180;
    alfa = nearstar_p_orb_tilt[n] * deg;
    xx   = -nearstar_p_orb_ray[n] * sin(beta) * cos(alfa);
    plz  = nearstar_p_orb_ray[n] * cos(beta) * cos(alfa);
    ply  = nearstar_p_orb_ray[n] * sin(alfa);
    plz *= nearstar_p_orb_ecc[n];
    beta = nearstar_p_orb_orient[n];
    plx  = xx * cos(beta) + plz * sin(beta);
    plz  = plz * cos(beta) - xx * sin(beta);
    plx += nearstar_x;
    ply += nearstar_y;
    plz += nearstar_z;

    if (nearstar_p_owner[n] > -1) {
        moonorigin(nearstar_p_owner[n]);
        plx += mox;
        ply += moy;
        plz += moz;
    }
    nearstar_p_plx[n] = plx;
    nearstar_p_ply[n] = ply;
    nearstar_p_plz[n] = plz;
}

// Calculate the revolution period of a body, in seconds.
// For planets: Around the star.
// For moons: Around the planet.
// .01e-7 is the equivalent, in the virtual cosmos of Noctis.
float rtp(int16_t n) {
    double p_m, p_riv, ors, xx;
    ors = nearstar_p_orb_ray[n] * nearstar_p_orb_ray[n];

    if (nearstar_p_owner[n] > -1) {
        xx    = nearstar_p_ray[nearstar_p_owner[n]];
        p_m   = qt_M_PI * xx * xx * xx * 0.44e-4;
        p_riv = sqrt(p_m / ors);
    } else {
        p_riv = sqrt(s_m / ors);
    }

    return (360 / p_riv);
}

// Calculates the longitude of the point on a planet that the user is looking
// at, given by (plx, ply, plz). The observed is placed at (obs_x, obs_z),
// its y is irrelevant.
int16_t planet_viewpoint(double obs_x, double obs_z) {
    uint16_t a;
    int16_t plwp = 0;
    double xx;
    double zz;
    double min = 1E99;

    for (a = 0; a < 360; a++) {
        xx = plx + cos(deg * (double) a) - obs_x;
        zz = plz + sin(deg * (double) a) - obs_z;
        xx = xx * xx + zz * zz;

        if (xx < min) {
            plwp = a;
            min  = xx;
        }
    }

    return (plwp);
}

int16_t cplx_planet_viewpoint(int16_t logical_id) {
    int16_t owner;
    double ownerplx;
    double ownerplz;

    if (nearstar_p_owner[logical_id] == -1) {
        return (planet_viewpoint(nearstar_x, nearstar_z));
    }

    owner = nearstar_p_owner[logical_id];

    if (nearstar_p_type[owner] == 10) {
        planet_xyz(owner);
        ownerplx = plx;
        ownerplz = plz;
        planet_xyz(logical_id);
        return (planet_viewpoint(ownerplx, ownerplz));
    } else {
        return (planet_viewpoint(nearstar_x, nearstar_z));
    }
}

// Effect selection via "flares" control variable.

int8_t previous_flares_value = 0;
void setfx(int8_t fx) {
#ifndef WITH_GODOT
    previous_flares_value = flares;
    flares                = fx;
#endif
}

void chgfx(int8_t fx) { flares = fx; }

void resetfx() { flares = previous_flares_value; }

/* Tracing sticks (3D Part). */

int32_t fpx = -1; // First-point-x
int32_t fpy;      // First-point-y

float p_rx, p_ry, p_rz;
float stick_uneg = 200;

// Tracing luminous sticks (in 2d, for the glows, generally used with the
// flares flag = 1).

/*
    Recursive function that divides triangular polygons into four parts,
    equally triangular, slightly casualizing the color.
*/

uint8_t map_color_a = 30;
uint8_t map_color_b = 31;
uint8_t map_color_c = 32;
uint8_t map_color_d = 33;

void randomic_mapper(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2,
                     int8_t divisions) {
    float vx[3], vy[3], vz[3];
    float e0, f0, g0;
    float e1, f1, g1;
    float e2, f2, g2;
    divisions--;

    if (divisions) {
        e0 = (x0 + x1) * 0.5;
        f0 = (y0 + y1) * 0.5;
        g0 = (z0 + z1) * 0.5;
        e1 = (x1 + x2) * 0.5;
        f1 = (y1 + y2) * 0.5;
        g1 = (z1 + z2) * 0.5;
        e2 = (x0 + x2) * 0.5;
        f2 = (y0 + y2) * 0.5;
        g2 = (z0 + z2) * 0.5;

        if (divisions == 1) {
            vx[0] = x0;
            vy[0] = y0;
            vz[0] = z0;
            vx[1] = e0;
            vy[1] = f0;
            vz[1] = g0;
            vx[2] = e2;
            vy[2] = f2;
            vz[2] = g2;
            poly3d(vx, vy, vz, 3, map_color_a);
            randomic_mapper(x0, y0, z0, e0, f0, g0, e2, f2, g2, divisions);
            vx[0] = e1;
            vy[0] = f1;
            vz[0] = g1;
            poly3d(vx, vy, vz, 3, map_color_b);
            randomic_mapper(e1, f1, g1, e0, f0, g0, e2, f2, g2, divisions);
            vx[1] = x2;
            vy[1] = y2;
            vz[1] = z2;
            poly3d(vx, vy, vz, 3, map_color_c);
            randomic_mapper(e1, f1, g1, x2, y2, z2, e2, f2, g2, divisions);
            vx[2] = x1;
            vy[2] = y1;
            vz[2] = z1;
            vx[1] = e0;
            vy[1] = f0;
            vz[1] = g0;
            poly3d(vx, vy, vz, 3, map_color_d);
            randomic_mapper(e1, f1, g1, e0, f0, g0, x1, y1, z1, divisions);
        } else {
            randomic_mapper(x0, y0, z0, e0, f0, g0, e2, f2, g2, divisions);
            randomic_mapper(e1, f1, g1, e0, f0, g0, e2, f2, g2, divisions);
            randomic_mapper(e1, f1, g1, x2, y2, z2, e2, f2, g2, divisions);
            randomic_mapper(e1, f1, g1, e0, f0, g0, x1, y1, z1, divisions);
        }
    }
}

/*
    Free a handle in which a polygonal graphic file was loaded: if the handle
    is already free nothing happens.
*/

void unloadpv(int16_t handle) {
    int16_t h;
    uint16_t eod;

    if (handle >= handles) {
        return;
    }

    if (!pvfile_datalen[handle]) {
        return;
    }

    /*
        Updates the pointers of all handles which are stored beyond the
        specified one. The type cast is used to convince those ANSI dicks
        that the pointer is moved byte by byte, and not depending on the type
        of data to which it is pointing.
    */
    for (h = 0; h < handles; h++)
        if (pvfile_dataptr[h] > pvfile_dataptr[handle]) {
            ((int8_t *) pv_n_vtx)[h] -= pvfile_datalen[handle];
            ((int8_t *) pvfile_x)[h] -= pvfile_datalen[handle];
            ((int8_t *) pvfile_y)[h] -= pvfile_datalen[handle];
            ((int8_t *) pvfile_z)[h] -= pvfile_datalen[handle];
            ((int8_t *) pvfile_c)[h] -= pvfile_datalen[handle];

            if (pv_mid_x[h]) {
                ((int8_t *) pv_mid_x)[h] -= pvfile_datalen[handle];
                ((int8_t *) pv_mid_y)[h] -= pvfile_datalen[handle];
                ((int8_t *) pv_mid_z)[h] -= pvfile_datalen[handle];
                ((int8_t *) pv_mid_d)[h] -= pvfile_datalen[handle];
                ((int8_t *) pv_dep_i)[h] -= pvfile_datalen[handle];
            }

            pvfile_dataptr[h] -= pvfile_datalen[handle];
        }

    // Move data back to free memory in the polyonal area (if necessary).
    eod = pvfile_dataptr[handle] + pvfile_datalen[handle];

    if (eod < pvfile_datatop) {
        memmove(pv_n_vtx[handle], pv_n_vtx[handle] + pvfile_datalen[handle], pvfile_datatop - eod);
    }

    // Update the top of the polygonal area.
    pvfile_datatop -= pvfile_datalen[handle];
    // Update the memory situaton for polygonal graphics, to free the handle.
    pvfile_datalen[handle] = 0;
}

// Free all the handles.
void unloadallpv() {
    int16_t h;
    pvfile_datatop = 0;

    for (h = 0; h < handles; h++) {
        pvfile_datalen[h] = 0;
    }
}

/*
    Load a polygonal model file. If the specified handle is busy, it will be
   freed and reassigned. Parameters: Handle: Number of the database in which to
   host the file, from 0 to 15. virtual_file_position: Negative offset from the
   end of SUPPORTS.NCT. x/y/z_scale: Scale correction of the polygons on all the
   axes. x/y/z_move: Translation of the polygons on all the axes. base_color:
   Basic color of the polygons. depth_sort: Flag that specifies whether to
   allocate space for the calculation of the midpoints of the vertices of each
   polygon and carry out the depth-sorting when the object is to be traced.
    Return:
        -1: The file cannot be accessed.
        0: The hndle is unassignable (does not exist), or there is not enough
        memory in the pvfile buffer to load and / or manage the polygons.
        +1: Everything went well.
*/

int8_t loadpv(int16_t handle, int32_t virtual_file_position, float xscale, float yscale, float zscale, float xmove,
              float ymove, float zmove, uint8_t base_color, int8_t depth_sort) {
    int16_t c, p;

    // Check availability of the file and the handle.
    if (handle >= handles) {
        return (0);
    }

    FILE *fh = sa_open(virtual_file_position);

    if (fh == nullptr) {
        return -1;
    }

    // Free the handle if it is currently occupied.
    if (pvfile_datalen[handle]) {
        unloadpv(handle);
    }

    // Internal handle update.
    pvfile_datalen[handle] = 0;
    pvfile_dataptr[handle] = pvfile_datatop;
    // Reading polygon numbers.
    fread(&pvfile_npolygs[handle], 2, 1, fh);
    // Pointer preparation.
    pv_n_vtx[handle] = (int8_t *) (pvfile + pvfile_datatop);
    pvfile_datatop += 1 * pvfile_npolygs[handle];
    pvfile_x[handle] = (float *) (pvfile + pvfile_datatop);
    pvfile_datatop += 16 * pvfile_npolygs[handle];
    pvfile_y[handle] = (float *) (pvfile + pvfile_datatop);
    pvfile_datatop += 16 * pvfile_npolygs[handle];
    pvfile_z[handle] = (float *) (pvfile + pvfile_datatop);
    pvfile_datatop += 16 * pvfile_npolygs[handle];
    pvfile_c[handle] = (int8_t *) (pvfile + pvfile_datatop);
    pvfile_datatop += 1 * pvfile_npolygs[handle];
    /*
        Clear the first data pointer for the depth sort. If it is no
       subsequently modified, then depth sorting is not required for the object
       in question.
    */
    pv_mid_x[handle] = 0;

    // Check availabity before reading the data.
    if (pvfile_datatop > pv_bytes) {
        pvfile_datatop = pvfile_dataptr[handle];
        fclose(fh);
        return (0);
    }

    // Reading all the data on the polygons, in a single block.
    fread(pvfile + pvfile_dataptr[handle], pvfile_datatop - pvfile_dataptr[handle], 1, fh);
    // dopodich� si pu� anche richiudere il file...
    fclose(fh);

    // Resetting unused vertex data (for triangles).
    for (p = 0; p < pvfile_npolygs[handle]; p++)
        if (pv_n_vtx[handle][p] == 3) {
            pvfile_x[handle][4 * p + 3] = 0;
            pvfile_y[handle][4 * p + 3] = 0;
            pvfile_z[handle][4 * p + 3] = 0;
        }

    // Prepare pointers for depth sorting management.
    if (depth_sort) {
        pv_mid_x[handle] = (float *) (pvfile + pvfile_datatop);
        pvfile_datatop += 4 * pvfile_npolygs[handle];
        pv_mid_y[handle] = (float *) (pvfile + pvfile_datatop);
        pvfile_datatop += 4 * pvfile_npolygs[handle];
        pv_mid_z[handle] = (float *) (pvfile + pvfile_datatop);
        pvfile_datatop += 4 * pvfile_npolygs[handle];
        pv_mid_d[handle] = (float *) (pvfile + pvfile_datatop);
        pvfile_datatop += 4 * pvfile_npolygs[handle];
        pv_dep_i[handle] = (int16_t *) (pvfile + pvfile_datatop);
        pvfile_datatop += 2 * pvfile_npolygs[handle];

        // Check available memory for newly added data.
        if (pvfile_datatop > pv_bytes) {
            pvfile_datatop = pvfile_dataptr[handle];
            return (0);
        }
    }

    // Scale, color, and translation adaptation.
    for (c = 0; c < 4 * pvfile_npolygs[handle]; c++) {
        pvfile_x[handle][c] *= xscale;
        pvfile_x[handle][c] += xmove;
        pvfile_y[handle][c] *= yscale;
        pvfile_y[handle][c] += ymove;
        pvfile_z[handle][c] *= zscale;
        pvfile_z[handle][c] += zmove;
        pvfile_c[handle][c] += base_color;
    }

    // Calculation of average points and preparation of depth-sorting indices.
    if (depth_sort) {
        for (p = 0; p < pvfile_npolygs[handle]; p++) {
            pv_dep_i[handle][p] = p;
            pv_mid_d[handle][p] = 0;
            pv_mid_x[handle][p] = 0;
            pv_mid_y[handle][p] = 0;
            pv_mid_z[handle][p] = 0;

            if (pv_n_vtx[handle][p]) {
                for (c = 0; c < pv_n_vtx[handle][p]; c++) {
                    pv_mid_x[handle][p] += pvfile_x[handle][4 * p + c];
                    pv_mid_y[handle][p] += pvfile_y[handle][4 * p + c];
                    pv_mid_z[handle][p] += pvfile_z[handle][4 * p + c];
                }

                pv_mid_x[handle][p] /= c;
                pv_mid_y[handle][p] /= c;
                pv_mid_z[handle][p] /= c;
            }
        }
    }

    // All done: Compute the memory used by this handle.
    pvfile_datalen[handle] = pvfile_datatop - pvfile_dataptr[handle];
    return (1);
}

/*
    Recursive sorting. To order the polygons quickly. It also takes care of
    other sorts by distance, the planets and moons for example.
*/

void quick_sort(int16_t *index, float *mdist, int16_t start, int16_t end) {
    int16_t tq;
    int16_t jq = end;
    int16_t iq = start;
    float xq   = mdist[index[(start + end) / 2]];

    while (iq <= jq) {
        while (mdist[index[iq]] > xq) {
            iq++;
        }

        while (mdist[index[jq]] < xq) {
            jq--;
        }

        if (iq <= jq) {
            tq        = index[iq];
            index[iq] = index[jq];
            index[jq] = tq;
            iq++;
            jq--;
        }
    }

    if (start < jq) {
        quick_sort(index, mdist, start, jq);
    }

    if (iq < end) {
        quick_sort(index, mdist, iq, end);
    }
}

/*
    Traccia una figura poligonale.
    handle: l'handle (da 0 a 15) che si � attribuito al file con "loadpv";
    mode: pu� essere -- 0 = tracciamento poligoni in tinta unita;
                1 = tracciamento con texture mapping;
                2 = rimappatura randomica ricorsiva dei poligoni.
    rm_iterations: viene usato solo se mode = 2, indica quante suddivisioni
            devono essere effettuate per ogni poligono rimappato;
    center_x/y/z: coordinate ove piazzare il centro dell'oggetto;
    use_depth_sort: flag per attivare il depth sort, che viene tuttavia
            effettivamente attivato solo se � stato incluso come
            opzione nella chiamata a "loadpv" per quell'handle.
*/

/*
    Traccia una figura poligonale.
    handle: l'handle (da 0 a 15) che si � attribuito al file con "loadpv";
    mode: pu� essere -- 0 = tracciamento poligoni in tinta unita;
                1 = tracciamento con texture mapping;
                2 = rimappatura randomica ricorsiva dei poligoni.
    rm_iterations: viene usato solo se mode = 2, indica quante suddivisioni
            devono essere effettuate per ogni poligono rimappato;
    center_x/y/z: coordinate ove piazzare il centro dell'oggetto;
    use_depth_sort: flag per attivare il depth sort, che viene tuttavia
            effettivamente attivato solo se � stato incluso come
            opzione nella chiamata a "loadpv" per quell'handle.
*/

/**
 * @brief Draw a polygonal figure.
 *
 * @param handle The handle (0-15) assigned to the file from `loadpv`
 * @param mode 0=Solid Polygons, 1=Tracking w/ Texture Mapping, 2=Random
 * Recursive Polygon Remapping
 * @param rm_iterations Only used if mode=2, indicates how many subdivisions
 * must be done for each remapped polygon
 * @param center_x Object center x coordinate
 * @param center_y Object center y coordinate
 * @param center_z Object center z coordinate
 * @param use_depth_sort Flag to activate depth sort, only actived for real if
 * `loadpv` was called with it activated.
 */
void drawpv(int16_t handle, int16_t mode, int16_t rm_iterations, float center_x, float center_y, float center_z,
            int8_t use_depth_sort) {
    float dx, dy, dz;
    uint16_t p, c, i, k;

    if (handle >= handles) {
        return;
    }

    if (!pvfile_datalen[handle]) {
        return;
    }

    // Entire space translation at the object's origin
    cam_x -= center_x;
    cam_y -= center_y;
    cam_z -= center_z;

    uint16_t mask;
    if (use_depth_sort && pv_mid_x[handle]) {
        // Tracking with depth sorting.
        // Stage 1: Midpoint distance calculation.
        for (p = 0; p < pvfile_npolygs[handle]; p++) {
            dx                  = pv_mid_x[handle][p] - cam_x;
            dy                  = pv_mid_y[handle][p] - cam_y;
            dz                  = pv_mid_z[handle][p] - cam_z;
            pv_mid_d[handle][p] = dx * dx + dy * dy + dz * dz;
        }

        // Stage 2: Sorting polygons by distance
        quick_sort(pv_dep_i[handle], pv_mid_d[handle], 0, pvfile_npolygs[handle] - 1);

        // Stage 3: Tracking, in the order specified above
        for (p = 0; p < pvfile_npolygs[handle]; p++) {
            c = pv_dep_i[handle][p];
            i = c * 4;

            switch (mode) {
            case 0:
                poly3d(pvfile_x[handle] + i, pvfile_y[handle] + i, pvfile_z[handle] + i, pv_n_vtx[handle][c],
                       pvfile_c[handle][c]);
                break;

            case 1:
                k = pvfile_c[handle][c];

                mask = k;
                mask &= 0x3Fu;
                k &= 0xC0u;
                mask >>= 1u;
                k |= mask;

                polymap(pvfile_x[handle] + i, pvfile_y[handle] + i, pvfile_z[handle] + i, pv_n_vtx[handle][c], k);
                break;

            case 2:
                map_color_a = pvfile_c[handle][c];
                map_color_b = map_color_a - 2;
                map_color_c = map_color_a - 1;
                map_color_d = map_color_a + 1;
                randomic_mapper(pvfile_x[handle][i + 0], pvfile_y[handle][i + 0], pvfile_z[handle][i + 0],
                                pvfile_x[handle][i + 1], pvfile_y[handle][i + 1], pvfile_z[handle][i + 1],
                                pvfile_x[handle][i + 2], pvfile_y[handle][i + 2], pvfile_z[handle][i + 2],
                                rm_iterations);

                if (pv_n_vtx[handle][p] == 4)
                    randomic_mapper(pvfile_x[handle][i + 2], pvfile_y[handle][i + 2], pvfile_z[handle][i + 2],
                                    pvfile_x[handle][i + 3], pvfile_y[handle][i + 3], pvfile_z[handle][i + 3],
                                    pvfile_x[handle][i + 0], pvfile_y[handle][i + 0], pvfile_z[handle][i + 0],
                                    rm_iterations);
                break;
            default:
                break;
            }
        }
    } else {
        // tracciamento senza depth sorting.
        // in queso caso traccia i poligoni nell'ordine in cui
        // sono stati salvati nel file di grafica di "PolyVert".
        for (p = 0, i = 0; p < pvfile_npolygs[handle]; p++, i += 4)
            switch (mode) {
            case 0:
                poly3d(pvfile_x[handle] + i, pvfile_y[handle] + i, pvfile_z[handle] + i, pv_n_vtx[handle][p],
                       pvfile_c[handle][p]);
                break;

            case 1:
                k = pvfile_c[handle][p];

                mask = k;
                mask &= 0x3Fu;
                k &= 0xC0u;
                mask >>= 1u;
                k |= mask;

                polymap(pvfile_x[handle] + i, pvfile_y[handle] + i, pvfile_z[handle] + i, pv_n_vtx[handle][p], k);
                break;
            case 2:
                map_color_a = pvfile_c[handle][p];
                map_color_b = map_color_a - 2;
                map_color_c = map_color_a - 1;
                map_color_d = map_color_a + 1;
                randomic_mapper(pvfile_x[handle][i + 0], pvfile_y[handle][i + 0], pvfile_z[handle][i + 0],
                                pvfile_x[handle][i + 1], pvfile_y[handle][i + 1], pvfile_z[handle][i + 1],
                                pvfile_x[handle][i + 2], pvfile_y[handle][i + 2], pvfile_z[handle][i + 2],
                                rm_iterations);

                if (pv_n_vtx[handle][p] == 4)
                    randomic_mapper(pvfile_x[handle][i + 2], pvfile_y[handle][i + 2], pvfile_z[handle][i + 2],
                                    pvfile_x[handle][i + 3], pvfile_y[handle][i + 3], pvfile_z[handle][i + 3],
                                    pvfile_x[handle][i + 0], pvfile_y[handle][i + 0], pvfile_z[handle][i + 0],
                                    rm_iterations);
                break;
            default:
                break;
            }
    }

    // traslazione intero spazio all'origine precedente.
    cam_x += center_x;
    cam_y += center_y;
    cam_z += center_z;
}

/*  Replica una forma poligonale, copiandola da un'handle gi� definito
    a uno di uguali dimensioni. In caso d'errore, non succede nulla. */

void copypv(int16_t dest_handle, int16_t src_handle) {
    if (src_handle >= handles) {
        return;
    }

    if (dest_handle >= handles) {
        return;
    }

    if (!pvfile_datalen[src_handle]) {
        return;
    }

    if (pvfile_datalen[dest_handle] != pvfile_datalen[src_handle]) {
        return;
    }

    memcpy(pv_n_vtx[dest_handle], pv_n_vtx[src_handle], pvfile_datalen[src_handle]);
}

/*  Ruota una forma poligonale rispetto a uno dei suoi vertici,
    che viene assunto come centro di rotazione, applicando anche
    un fattore di scalatura (che pu� essere 1 se non � necessario
    cambiare le dimensioni, come possono essere 0 gli angoli se
    si stanno cambiando le dimensioni senza ruotare).
    "vertexs_to_affect" � un puntatore a una serie di strutture "pvlist",
    nelle quali sono elencati i vertici che verranno effettivamente modificati:
    se il puntatore "vertexs_to_affect" � nullo, tutti i vertici lo sono.
    Gli angoli sono espressi in gradi. */

void modpv(int16_t handle, int16_t polygon_id, int16_t vertex_id, float x_scale, float y_scale, float z_scale,
           float x_angle, float y_angle, float z_angle, pvlist *vertexs_to_affect) {
    if (handle >= handles) {
        return;
    }

    if (!pvfile_datalen[handle]) {
        return;
    }

    float sin_x = sin(deg * x_angle);
    float cos_x = cos(deg * x_angle);
    float sin_y = sin(deg * y_angle);
    float cos_y = cos(deg * y_angle);
    float sin_z = sin(deg * z_angle);
    float cos_z = cos(deg * z_angle);
    int16_t c, p, v, i, j;
    float x1, y1, z1;
    float cx, cy, cz;

    if (polygon_id > -1 && vertex_id > -1) {
        i  = 4 * polygon_id + vertex_id;
        cx = pvfile_x[handle][i];
        cy = pvfile_y[handle][i];
        cz = pvfile_z[handle][i];
    } else {
        cx = 0;
        cy = 0;
        cz = 0;
    }

    if (!vertexs_to_affect) {
        for (p = 0; p < pvfile_npolygs[handle]; p++) {
            i = 4 * p;

            for (v = 0; v < pv_n_vtx[handle][p]; v++) {
                x1                  = (pvfile_x[handle][i] - cx) * cos_y + (pvfile_z[handle][i] - cz) * sin_y;
                z1                  = (pvfile_z[handle][i] - cz) * cos_y - (pvfile_x[handle][i] - cx) * sin_y;
                pvfile_z[handle][i] = z_scale * (z1 * cos_x + (pvfile_y[handle][i] - cy) * sin_x) + cz;
                y1                  = (pvfile_y[handle][i] - cy) * cos_x - z1 * sin_x;
                pvfile_x[handle][i] = x_scale * (x1 * cos_z + y1 * sin_z) + cx;
                pvfile_y[handle][i] = y_scale * (y1 * cos_z - x1 * sin_z) + cy;
                i++;
            }
        }
    } else {
        p = 0;

        while (vertexs_to_affect[p].polygon_id != 0xFFF) {
            c = vertexs_to_affect[p].polygon_id;
            i = 4 * c;
            v = 0;

            do {
                if (v == 0 && vertexs_to_affect[p].vtxflag_0) {
                    j = i;
                    goto perform;
                }

                if (v == 1 && vertexs_to_affect[p].vtxflag_1) {
                    j = i + 1;
                    goto perform;
                }

                if (v == 2 && vertexs_to_affect[p].vtxflag_2) {
                    j = i + 2;
                    goto perform;
                }

                if (v == 3 && vertexs_to_affect[p].vtxflag_3) {
                    j = i + 3;
                    goto perform;
                }

                goto next;
            perform:
                x1                  = (pvfile_x[handle][j] - cx) * cos_y + (pvfile_z[handle][j] - cz) * sin_y;
                z1                  = (pvfile_z[handle][j] - cz) * cos_y - (pvfile_x[handle][j] - cx) * sin_y;
                pvfile_z[handle][j] = z_scale * (z1 * cos_x + (pvfile_y[handle][j] - cy) * sin_x) + cz;
                y1                  = (pvfile_y[handle][j] - cy) * cos_x - z1 * sin_x;
                pvfile_x[handle][j] = x_scale * (x1 * cos_z + y1 * sin_z) + cx;
                pvfile_y[handle][j] = y_scale * (y1 * cos_z - x1 * sin_z) + cy;
            next:
                v++;
            } while (v < pv_n_vtx[handle][c]);

            p++;
        }
    }

    if (pv_mid_x[handle]) {
        for (p = 0; p < pvfile_npolygs[handle]; p++) {
            i  = 4 * p;
            cx = 0;
            cy = 0;
            cz = 0;

            for (v = 0; v < pv_n_vtx[handle][p]; v++) {
                cx += pvfile_x[handle][i];
                cy += pvfile_y[handle][i];
                cz += pvfile_z[handle][i];
                i++;
            }

            pv_mid_x[handle][p] = cx / v;
            pv_mid_y[handle][p] = cy / v;
            pv_mid_z[handle][p] = cz / v;
        }
    }
}

// Returns the alphabetic correspondent of integers and / or real numbers.
char *alphavalue(double value) {
    // Please Note: Different behavior on Windows & Linux. TODO; Fix
    gcvt(value, 15, dec);
    return (dec);
}

// Draws the background, with the map offsets.map.
void background(uint16_t start, uint8_t *target, uint8_t *background, uint8_t *offsetsmap, uint16_t total_map_bytes,
                uint16_t screenshift) {
    uint16_t tex_loc = start /*+ 4*/;

    for (uint16_t i = (total_map_bytes / 2), si = 0; i > 0; i--, si += 2) {
        uint16_t word = ((uint16_t) (((uint16_t) offsetsmap[si + 1]) << 8u)) | ((uint16_t) offsetsmap[si]);
        if (word >= 64000) {
            uint16_t offset =
                (((uint16_t) (((uint16_t) offsetsmap[si + 1]) << 8u)) | ((uint16_t) offsetsmap[si])) - 64000;

            tex_loc += offset;
        } else {
            uint16_t screen_loc = ((uint16_t) (((uint16_t) offsetsmap[si + 1]) << 8u)) | ((uint16_t) offsetsmap[si]);
            screen_loc += screenshift;
            uint8_t color = background[tex_loc];

            memset(&target[screen_loc], color, 5);
            memset(&target[screen_loc + 320], color, 5);
            memset(&target[screen_loc + 640], color, 5);
            memset(&target[screen_loc + 960], color, 5);
            memset(&target[screen_loc + 1280], color, 5);

            tex_loc += 1;
        }
    }
}

double get_id_code(double x, double y, double z);

/*
    Starry sky, three-of-a-kind. In the amplified view, it has 2,744 stars.
    Star magnitudes go from 0 to +13. Since the player is a cat, with scotopic
    vision, they can probably see more than normal.
    JORIS added on 2023-07-29: callback function to get the results
*/

void sky(uint16_t limits, bool use_callback, void (*callback)(double x, double y, double z, double id_code)) {
    uint16_t debug;

    auto min_xy            = (int32_t) (1E9);
    int8_t visible_sectors = 9;

    if (field_amplificator) {
        visible_sectors = 14; // note: if this changes, make sure to change stars_visible array's size as well
    }

    uint8_t sx, sy, sz;
    float xx, yy, zz, z2, rz, inv_rz, starneg;

    if (!ap_targetting) {
        starneg = 10000;
    } else {
        starneg = 1;
    }

    int32_t sect_x, sect_y, sect_z, rx, ry;
    int32_t advance = 100000, k = 100000 * visible_sectors;
    int32_t temp_x, temp_y, temp_z, temp;
    /*
        The following section changes the rarity factor of the stars as the
        distance from the galactic center increases. The scale on the Y-Axis is
        amplified 30 times, so that the galaxy has the shape of a crushed disk.
        Stars will be rarefied depending on the value of "distance_from_home".
        This is a table that provides the number of stars eliminated (each
        sector contains one star, and the number of sectors visible to the
        amplified field is 14 * 14 * 14 = 2744) as distance_from_home increases
        its value:
            0 - 400,000,000                 0% eliminated
            400,000,000 - 800,000,000       50% eliminated
            1,200,000,000 - 1,600,000,000   75% eliminated
            1,600,000,000 - 2,000,000,000   87.5% eliminated.
        Over 2 billion units, the player can no longer select stars: The rarity
        factor ratio would still be of 1 effective star every 16 sectors.
    */
    int16_t rarity_factor;
    double distance_from_home;

    distance_from_home = sqrt(dzat_x * dzat_x + dzat_z * dzat_z);
    distance_from_home += 30 * fabs(dzat_y);

    rarity_factor = (int16_t) (distance_from_home * 0.25e-8);
    rarity_factor = 1u << (uint16_t) rarity_factor;
    rarity_factor--;

    sect_x = (int32_t) ((dzat_x - visible_sectors * 50000) / 100000);
    sect_x *= 100000;

    sect_y = (int32_t) ((dzat_y - visible_sectors * 50000) / 100000);
    sect_y *= 100000;

    sect_z = (int32_t) ((dzat_z - visible_sectors * 50000) / 100000);
    sect_z *= 100000;

    uint32_t index = 0;
    uint16_t i = 0;

    // Loop over a 3D cube of l,w,h = visible_sectors.
    for (sx = 0; sx < visible_sectors; sx++) {
        for (sy = 0; sy < visible_sectors; sy++) {
            for (sz = 0; sz < visible_sectors; sz++, sect_z += advance) {
                uint16_t cutoff = 50000;

                temp_x = ((sect_x + sect_z) & 0x0001FFFFu) + sect_x;
                // Exclude stars with x coordinate = 0
                if (temp_x == cutoff) {
                    continue;
                }
                temp_x -= cutoff;

                int32_t abc123 = (sect_x + sect_z);
                int32_t accum  = 0;

                int32_t edx = temp_x;
                int32_t eax = abc123;

                // This replaced a very sketchy usage of imul.
                int64_t result = (int64_t) edx * (int64_t) eax;
                eax            = result & 0xFFFFFFFFu;
                edx            = result >> 32u;
                edx += eax;
                accum = edx;

                int32_t idkbro = (sect_x + sect_z) + accum;

                temp_y = (accum & 0x001FFFFu) + sect_y;
                // Exclude stars with y coordinate = 0
                if (temp_y == cutoff) {
                    continue;
                }
                temp_y -= cutoff;

                edx    = temp_y;
                eax    = idkbro;
                result = (int64_t) edx * (int64_t) eax;
                eax    = result & 0xFFFFFFFFu;
                edx    = result >> 32u;
                edx += eax;
                accum = edx;

                temp_z = (accum & 0x0001FFFFu) + sect_z;
                // Exclude stars with z coordinate = 0
                if (temp_z == cutoff) {
                    continue;
                }
                temp_z -= cutoff;

                uint32_t netpos = temp_x + temp_y + temp_z;
                if ((netpos & rarity_factor) != 0) {
                    continue;
                }

                zz = temp_z - dzat_z;
                xx = temp_x - dzat_x;
                yy = temp_y - dzat_y;

                if (use_callback) {
                    callback(temp_x,temp_y,temp_z, get_id_code(temp_x, temp_y, temp_z));
                } else {
                    stars_visible[i*3 + 0] = xx;
                    stars_visible[i*3 + 1] = yy;
                    stars_visible[i*3 + 2] = zz;
                    i += 1;
                }

                /*
                z2 = (zz * opt_tcosbeta) - (xx * opt_tsinbeta);
                rz = (z2 * opt_tcosalfa) + (yy * opt_tsinalfa);

                if (rz < starneg) {
                    continue;
                }

                inv_rz = uno / rz;
                rx     = (int32_t) round(((xx * opt_pcosbeta) + (zz * opt_psinbeta)) * inv_rz);

                index = rx + VIEW_X_CENTER;
                if (index <= 10 || index >= adapted_width - 10) {
                    continue;
                }

                ry = (int32_t) round((yy * opt_pcosalfa - z2 * opt_psinalfa) * inv_rz) - 2;

                uint32_t nety = ry + VIEW_Y_CENTER;
                if (nety <= 10 || nety >= adapted_height - 10) {
                    continue;
                }

                index += (uint32_t) (adapted_width * nety);

                if (ap_targetting != 1) {
                    uint8_t color = adapted[index];
                    if (color == 68 || color < (limits >> 8u) || color > (limits & 0xFFu)) {
                        continue;
                    }
                }

                temp              = (int32_t) rz;
                int32_t tempshift = temp >> (13u + field_amplificator);
                int8_t mask       = 63 - tempshift;
                if (mask >= 0) {
                    int8_t color = adapted[index];
                    adapted[index] &= 0xC0u;
                    color &= 0x3Fu;
                    mask += color;
                    if (mask > 63) {
                        mask = 63;
                    }
                    adapted[index] |= mask;
                }

                if (ap_targetting == 1) {
                    temp = (rx * rx) + (ry * ry);

                    if (temp < min_xy) {
                        min_xy      = temp;
                        ap_target_x = temp_x;
                        ap_target_y = temp_y;
                        ap_target_z = temp_z;
                    }
                }
                */
            }
            sect_z -= k;
            sect_y += advance;
        }
        sect_y -= k;
        sect_x += advance;
    }
    while (i < 2744) {
        // empty out the rest of the array
        // 0/0/0 elements are ignored on the front-end
        stars_visible[i*3 + 0] = 0;
        stars_visible[i*3 + 1] = 0;
        stars_visible[i*3 + 2] = 0;
        i += 1;
    }
}

// If set, draw a kind of bubble transparent around the globes drawn with the
// "globe" function. It is used to simulate the presence of the atmosphere, but
// only for planets with considerable quantities of gas.
uint8_t glass_bubble = 1;

/*  Come sopra, ma mentre quella di sopra traccia in 4x4 pixels,
    dimezzando la risoluzione per essere pi� veloce nel tracciamento
    di globi che possono coprire tutto lo schermo, questa traccia in 1x1,
    � pi� precisa ma applicabile solo alla visualizzazione delle stelle
    viste dai pianeti. */

double xsun_onscreen;

// Glow around the most intense lights.

/*  float far *lft_sin = (float far *) farmalloc (361*4);
    float far *lft_cos = (float far *) farmalloc (361*4);*/

float lft_sin[361];
float lft_cos[361];

int8_t lens_flares_init() {
    int16_t c;
    double a = 0, interval = M_PI / 180;

    for (c = 0; c <= 360; c++) {
        lft_cos[c] = (float) cos(a);
        lft_sin[c] = (float) sin(a);
        a += interval;
    }

    return (1);
}

/*
    Distant dots, for example, planets and moons. The two functions are
    integrated: a dot is drawn if the distance is big. As you approach, the dot
    becomes a disk of 5px diameter max. We then have to pass control to another
    function to draw the planet properly.
*/

const double pix_dst_scale = 0.384;
const double pix_rad_scale = 1228.8;

#define LIGHT_EMITTING 0
#define LIGHT_ABSORBING 1
#define MULTICOLOUR 2

int8_t pixilating_effect = LIGHT_EMITTING;
int8_t pixel_spreads     = 1;
uint8_t multicolourmask  = 0xC0;

// TODO; Might be offset from proper position. Verify against vanilla.
void single_pixel_at_ptr(uint16_t offset, uint8_t pixel_color) {
    // Add ptr shift to the offset.
    uint8_t *shifted = adapted + offset;
    uint8_t alow     = shifted[0];
    alow &= 0x3Fu;
    alow += pixel_color;

    uint8_t ahigh;
    switch (pixilating_effect) {
    case LIGHT_EMITTING:
        if (shifted[0] > 63) {
            ahigh = shifted[0];
            ahigh &= 0xC0u;
            if (alow > 0x3E) {
                alow = 0x3E;
            }
            alow |= ahigh;
            shifted[0] = alow;
        }
        break;
    case LIGHT_ABSORBING:
        if (shifted[0] > 63) {
            ahigh = shifted[0];
            ahigh &= 0xC0u;
            alow >>= 1u;
            alow |= ahigh;
            shifted[0] = alow;
        }
        break;
    case MULTICOLOUR:
        if (shifted[0] > 63) {
            ahigh = multicolourmask;
            ahigh &= 0xC0u;
            alow >>= 1u;
            alow |= ahigh;
            shifted[0] = alow;
        }
        break;
    default:
        break;
    }
}

/*
    The rest of the functions that operate on local data.
    Functions for: Setting the cosmic timer of Noctis, constructing the
    surroundings of the stars and planets, and visualization of the planets
    from orbit.

*/

// Measure the time elapsed from 1-1-1984 to today, accurate to one frame,
// which is 1/25 of a second on average.
void getsecs() {
    // January 1, 1984
    struct tm nepoch = {0};
    nepoch.tm_sec    = 0;
    nepoch.tm_min    = 0;
    nepoch.tm_hour   = 0;
    nepoch.tm_mday   = 1;
    nepoch.tm_mon    = 0;
    nepoch.tm_year   = 84;

    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo;
    timeinfo = localtime(&rawtime);
    // Have to extract info from timeinfo before doing difftime, or else
    // timeinfo will point to nepoch, for some reason...
    uint8_t currSecond = timeinfo->tm_sec;

    secs = difftime(rawtime, mktime(&nepoch));
    // Offsetting the clock by -82800 seconds is done to fudge it to match the
    // clock in the original. Don't really know where the discrepancy comes
    // from.
    secs -= 82800;
    isecs = currSecond;

    if (p_isecs != isecs) { // Frame timing.
        if (_delay >= 10) {
            _delay--;
        }

        p_isecs = isecs;
        gl_fps  = fps;
        fps     = 1;
    } else { // Fractions of a second
        if (gl_fps) {
            secs += (double) fps / (double) gl_fps;
        }

        fsecs = (double) fps / (double) gl_fps;
        fps++;
    }

    epoc = (int16_t) (6011 + secs / 1e9);
}

// Extracts from the pseudo table, from 74 trillion different elements,
// information about the chosen star.

void extract_ap_target_infos() {
    brtl_srand((uint16_t) (ap_target_x / 100000 * ap_target_y / 100000 * ap_target_z / 100000));
    ap_target_class = brtl_random(star_classes);
    ap_target_ray   = ((float) class_ray[ap_target_class] + (float) brtl_random(class_rayvar[ap_target_class])) * 0.001;
    ap_target_r     = class_rgb[3 * ap_target_class + 0];
    ap_target_g     = class_rgb[3 * ap_target_class + 1];
    ap_target_b     = class_rgb[3 * ap_target_class + 2];
    ap_target_spin  = 0;

    if (ap_target_class == 11) {
        ap_target_spin = brtl_random(30) + 1;
    }

    if (ap_target_class == 7) {
        ap_target_spin = brtl_random(12) + 1;
    }

    if (ap_target_class == 2) {
        ap_target_spin = brtl_random(4) + 1;
    }
}

// Extracts a whole-type pseudo-random number by converting it to f-p.
float zrandom(int16_t range) {
    return (float) (brtl_random(range) - brtl_random(range)); // NOLINT(misc-redundant-expression)
}


/* Added by JORIS on 2023-07-29
*/
double _star_id           = 12345;
int8_t _star_label[25]    = "UNKNOWN STAR / CLASS ...";
void update_star_label_by_offset(int32_t offset) {
    // NOTE: this is also used for planets as well (bit HACKY, I know!)
    FILE *smh = fopen(starmap_file, "rb");
    fseek(smh, offset, SEEK_SET);
    fread(&_star_id, 8, 1, smh);
    fread(&_star_label, 24, 1, smh);
    fclose(smh);
}
double get_id_code(double x, double y, double z) {
   return x / 100000 * y / 100000 * z / 100000;
}

/*  Part of the cartography management.
 *  It has been moved here to be called by "prepare_nearstar".
 *  --------------------------------------------------------------------------------
 *  Search for an identification code (for a planet or for a star) in the
 * stellar mapping file, and show the position of the record. If the result is
 * -1, the code does not exist, i.e. there is no name for the star or for the
 * planet that corresponds to that code. Type can be: 'P' = Planet, 'S' = Star.
 */

double idscale = 0.00001;

int32_t search_id_code(double id_code, int8_t type) {
    int32_t total_pos = 4;
    bool found        = false;
    uint16_t n, curr_pos, index;
    double id_low  = id_code - idscale;
    double id_high = id_code + idscale;
    FILE *smh      = fopen(starmap_file, "rb");

    if (smh != nullptr) {
        auto buffer = (int8_t *) malloc(ps_bytes);

        auto buffer_ascii  = (int8_t *) buffer;
        auto buffer_double = (double *) buffer;
        fseek(smh, 4, SEEK_SET);

        while ((n = fread(buffer_ascii, 1, ps_bytes, smh)) > 0) {
            curr_pos = 0;
            index    = 0;

            while (curr_pos < n) {
                if (buffer_ascii[curr_pos + 29] == type) {
                    if (buffer_double[index] > id_low && buffer_double[index] < id_high) {
                        found = true;
                        goto stop;
                    }
                }

                total_pos += 32;
                curr_pos += 32;
                index += 4;
            }
        }

    stop:
        fclose(smh);
        free(buffer);
    } else {
        godot::UtilityFunctions::print( "Starmap bin not found!" );
    }

    if (found) {
        return (total_pos);
    } else {
        return (-1);
    }
}


/* Prepare information on the nearby star, the one which the player has just
 * approached. Among other things, prepare the planets by extracting them from
 * the pseudo table.
 */

// Estimate the number of major planets associated with the coord. of a star.
int16_t starnop(double star_x, double star_y, double star_z) {
    int16_t r;
    brtl_srand((int32_t) star_x % 10000 * (int32_t) star_y % 10000 * (int32_t) star_z % 10000);
    r = brtl_random(class_planets[ap_target_class] + 1);
    r += brtl_random(2);
    r -= brtl_random(2);

    if (r < 0) {
        r = 0;
    }

    return (r);
    return 0;
}

void init() {
    lens_flares_init();
    getsecs();
    int32_t ir, ig, ib, ire = 0, ige = 0, ibe = 0;
    for (ir = 0; ir < 200; ir++) {
        m200[ir] = ir * 200;
    }

    n_offsets_map = (uint8_t *) malloc(om_bytes);
    n_globes_map  = (int8_t *) malloc((uint16_t) gl_bytes + (uint16_t) gl_brest);
    s_background  = (uint8_t *) malloc(st_bytes * sizeof(s_background));
    p_background  = (uint8_t *) malloc(pl_bytes * sizeof(p_background));
    memset(s_background, 0, st_bytes * sizeof(s_background));
    memset(p_background, 0, pl_bytes * sizeof(p_background));
    /* NOTE: This is set to at least 65k because polymap keeps running over the
     * end. It happens in the original source too, and somehow isn't a problem
     * there, but we can't have it running over into random memory. The bug is
     * present in the original source.
     */
    p_surfacemap = (uint8_t *) malloc(ps_bytes | 65536);
    objectschart = (quadrant *) malloc(oc_bytes * sizeof(quadrant));
#ifdef WITH_GODOT
    ruinschart   = (uint8_t *) malloc(oc_bytes);
#else
    ruinschart   = (uint8_t *) objectschart; // oc alias
#endif
    pvfile       = (uint8_t *) malloc(pv_bytes);
    adapted      = (uint8_t *) malloc(sc_bytes);
    txtr         = (uint8_t *) p_background;             // txtr alias

    // initialize on Balastrackonastreya
    dzat_x = -18928;
    dzat_y = -29680;
    dzat_z = -67336;

    ap_target_x = -18928;
    ap_target_y = -29680;
    ap_target_z = -67336;
    extract_ap_target_infos();
}

int16_t calculate_planet_spin(int16_t logical_id, double seedval);

// Modified by JORIS on 2023-07-30: added callback for planets
void prepare_nearstar(void (*onPlanetFound)(int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t viewpoint, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist)) {
    int16_t n, c, q, r, s, t;
    double key_radius;

    // if (!_delay) { // Modified by JORIS on 2023-07-30: we don't rely on Noctis' own timing system, so this has been removed to ensure that whenever prepare_nearstar gets called, stars are actually calculated
        nearstar_class = ap_target_class;
        nearstar_x     = ap_target_x;
        nearstar_y     = ap_target_y;
        nearstar_z     = ap_target_z;
        nearstar_ray   = ap_target_ray;
        nearstar_spin  = ap_target_spin;
        nearstar_r     = ap_target_r;
        nearstar_g     = ap_target_g;
        nearstar_b     = ap_target_b;
    // } // see note above

    s_m               = qt_M_PI * nearstar_ray * nearstar_ray * nearstar_ray * 0.01e-7;
    nearstar_identity = nearstar_x / 100000 * nearstar_y / 100000 * nearstar_z / 100000;
    brtl_srand((int32_t) nearstar_x % 10000 * (int32_t) nearstar_y % 10000 * (int32_t) nearstar_z % 10000);
    nearstar_nop = brtl_random(class_planets[nearstar_class] + 1);

    // First draw (almost random, unrealistic);
    for (n = 0; n < nearstar_nop; n++) {
        nearstar_p_owner[n]      = -1;
        nearstar_p_orb_orient[n] = (double) deg * (double) brtl_random(360);
        nearstar_p_orb_seed[n] =
            3.0 * (n * n + 1) * nearstar_ray + (float) brtl_random((int16_t) (300 * nearstar_ray)) / 100.0;
        nearstar_p_tilt[n]     = zrandom((int16_t) (10 * nearstar_p_orb_seed[n])) / 500.0;
        nearstar_p_orb_tilt[n] = zrandom((int16_t) (10 * nearstar_p_orb_seed[n])) / 5000.0;
        nearstar_p_orb_ecc[n] =
            1 - (double) brtl_random((int16_t) (nearstar_p_orb_seed[n] + 10 * fabs(nearstar_p_orb_tilt[n]))) / 2000;
        nearstar_p_ray[n]  = (double) brtl_random((int16_t) (nearstar_p_orb_seed[n])) * 0.001 + 0.01;
        nearstar_p_ring[n] = zrandom((int16_t) nearstar_p_ray[n]) * (1 + (double) brtl_random(1000) / 100.0);

        if (nearstar_class != 8) {
            nearstar_p_type[n] = brtl_random(planet_types);
        } else {
            if (brtl_random(2)) {
                nearstar_p_type[n] = 10;
                nearstar_p_orb_tilt[n] *= 100;
            } else {
                nearstar_p_type[n] = brtl_random(planet_types);
            }
        }

        if (nearstar_class == 2 || nearstar_class == 7 || nearstar_class == 15) {
            nearstar_p_orb_seed[n] *= 10;
        }
    }

    /* Aumento delle probabilit� di pianeti abitabili su classe zero. */

    if (!nearstar_class) {
        if (brtl_random(4) == 2) {
            nearstar_p_type[2] = 3;
        }

        if (brtl_random(4) == 2) {
            nearstar_p_type[3] = 3;
        }

        if (brtl_random(4) == 2) {
            nearstar_p_type[4] = 3;
        }
    }

    /*  Eliminazione di pianeti impossibili attorno a certe stelle.
        Fase 1: solo quelli impossibili per tipo di stella. */

    for (n = 0; n < nearstar_nop; n++) {
        switch (nearstar_class) {
        case 2:
            while (nearstar_p_type[n] == 3) {
                nearstar_p_type[n] = brtl_random(10);
            }

            break;

        case 5:
            while (nearstar_p_type[n] == 6 || nearstar_p_type[n] == 9) {
                nearstar_p_type[n] = brtl_random(10);
            }

            break;

        case 7:
            nearstar_p_type[n] = 9;
            break;

        case 9:
            while (nearstar_p_type[n] != 0 && nearstar_p_type[n] != 6 && nearstar_p_type[n] != 9) {
                nearstar_p_type[n] = brtl_random(10);
            }

            break;

        case 11:
            while (nearstar_p_type[n] != 1 && nearstar_p_type[n] != 7) {
                nearstar_p_type[n] = brtl_random(10);
            }
            break;
        default:
            break;
        }
    }

    /*  Eliminazione di pianeti impossibili attorno a certe stelle.
        Fase 2: solo quelli impossibili per distanza dalla stella. */

    for (n = 0; n < nearstar_nop; n++) {
        switch (nearstar_p_type[n]) {
        case 0:
            if (brtl_random(8)) {
                nearstar_p_type[n]++;
            }

            break;

        case 3:
            if ((n < 2) || (n > 6) || (nearstar_class && brtl_random(4))) {
                if (brtl_random(2)) {
                    nearstar_p_type[n]++;
                } else {
                    nearstar_p_type[n]--;
                }
            }

            break;

        case 7:
            if (n < 7) {
                if (brtl_random(2)) {
                    nearstar_p_type[n]--;
                } else {
                    nearstar_p_type[n] -= 2;
                }
            }

            break;
        }
    }

    /* Estrazione dei satelliti naturali (lune). */
    nearstar_nob = nearstar_nop;

    if (nearstar_class == 2 || nearstar_class == 7 || nearstar_class == 15) {
        goto no_moons;
    }

    for (n = 0; n < nearstar_nop; n++) {
        // (t=) Numero di satelliti per pianeta.
        s = nearstar_p_type[n];

        if (n < 2) {
            t = 0;

            if (s == 10) {
                t = brtl_random(3);
            }
        } else {
            t = brtl_random(planet_possiblemoons[s] + 1);
        }

        if (nearstar_nob + t > maxbodies) {
            t = maxbodies - nearstar_nob;
        }

        // Caratteristiche dei satelliti.
        for (c = 0; c < t; c++) {
            q                        = nearstar_nob + c;
            nearstar_p_owner[q]      = n;
            nearstar_p_moonid[q]     = c;
            nearstar_p_orb_orient[q] = (double) deg * (double) brtl_random(360);
            nearstar_p_orb_seed[q] =
                (c * c + 4) * nearstar_p_ray[n] + (float) zrandom((int16_t) (300 * nearstar_p_ray[n])) / 100;
            nearstar_p_tilt[q]     = zrandom((int16_t) (10 * nearstar_p_orb_seed[q])) / 50;
            nearstar_p_orb_tilt[q] = zrandom((int16_t) (10 * nearstar_p_orb_seed[q])) / 500;
            nearstar_p_orb_ecc[q] =
                1 - (double) brtl_random((int16_t) (nearstar_p_orb_seed[q] + 10 * fabs(nearstar_p_orb_tilt[q]))) / 2000;
            nearstar_p_ray[q]  = (double) brtl_random((int16_t) nearstar_p_orb_seed[n]) * 0.05 + 0.1;
            nearstar_p_ring[q] = 0;
            nearstar_p_type[q] = brtl_random(planet_types);
            // Estrazione tipologia di satellite:
            r = nearstar_p_type[q];

            // Un oggetto substellare come luna?
            // Ce lo pu� avere solo una stella compagna.
            if (r == 9 && s != 10) {
                r = 2;
            }

            // Un gigante gassoso come luna?
            // Ce lo pu� avere solo un oggetto substellare,
            // o una stella compagna in un sistema multiplo.
            if (r == 6 && s < 9) {
                r = 5;
            }

            // "Raffreddamento" satelliti esterni, lontani sia
            // dal pianeta che dalla stella, in genere congelati.
            if (n > 7 && brtl_random(c)) {
                r = 7;
            }

            if (n > 9 && brtl_random(c)) {
                r = 7;
            }

            // Lune relativamente grandi possono esistere solo
            // attorno a pianeti gassosi ed oggetti substellari.
            // Invece, i simil-lunari(1), i simil-marziani(5),
            // le lune come Io(0), e quelle come Europa(7),
            // possono esistere anche attorno ad altri tipi di
            // pianeti, ma di certo in scala piuttosto ridotta.
            if (r == 2 || r == 3 || r == 4 || r == 8) {
                if (s != 6 && s < 9) {
                    r = 1;
                }
            }

            // Attorno ai giganti gassosi, se il test precedente
            // � passato (s = 6/9/10, gassoso/substellare/stella),
            // b�, possono anche esserci, a certe condizioni,
            // delle lune abitabili. Per queste, per�, la stella
            // dev'essere in genere di classe zero ed il pianeta
            // gigante non dev'essere troppo lontano dalla stella.
            // C'� invece uguale probabilit� di trovare mondi
            // abitabili attorno agli oggetti substellari: al di
            // l� della distanza dalla stella, tali lune possono
            // essere scaldate abbastanza da una stella mancata.
            if (r == 3 && s < 9) {
                if (n > 7) {
                    r = 7;
                }

                if (nearstar_class && brtl_random(4)) {
                    r = 5;
                }

                if (nearstar_class == 2 || nearstar_class == 7 || nearstar_class == 11) {
                    r = 8;
                }
            }

            // Una luna ghiacciata � esclusa, prima di arrivare
            // almeno alla sesta orbita planetaria, perch� fa
            // comunque troppo caldo.
            if (r == 7 && n <= 5) {
                r = 1;
            }

            // Ma lune ghiacciate sono comunque molto pi�
            // frequenti se la stella � molto piccola e fredda:
            // un pianeta in genere pu� avere meccanismi interni
            // che lo scaldano. Una luna no.
            if ((nearstar_class == 2 || nearstar_class == 5 || nearstar_class == 7 || nearstar_class == 11) &&
                brtl_random(n)) {
                r = 7;
            }

            // Fine estrazione tipologia di satellite.
            nearstar_p_type[q] = r;
        }

        nearstar_nob += t;
    }

    /*  Ri-Normalizzazione delle dimensioni dei pianeti,
        normalizzazione delle orbite in base al principio di Keplero.
        Il principio di Keplero stabilisce che il raggio dell'orbita di
        un pianeta tende ad essere simile alla sommatoria dei raggi delle
        orbite di tutti i pianeti interni ad esso. Per�, per un numero di
        pianeti maggiore di 8, il principio non � pi� valido. Noctis
        rinormalizza le orbite oltre l'ottava, aggiungendo a tali orbite
        il 22% circa della sommatoria delle precedenti. Ovvero:

        SE si applica il principio di Keplero
        per (ad esempio) 12 pianeti, e per Raggio Prima Orbita = 1,
        allora i raggi delle altre orbite sarebbero:
            1  2  3  6  12  24  48  96  192  384  768  1536

        SE si applica l'organizzazione di Noctis, il tutto diventerebbe:
            1  2  3  6  12  24  48  96  117  143  174  212

        Il 22% non � un valore a caso: rappresenta all'incirca il rapporto
        fra i raggi delle orbite di Plutone e di Urano. Plutone � circa del
        22% pi� lontano dal Sole di Urano, cio� l'ottava orbita. Ovvio che
        non significa che un sistema planetario pi� vasto debba per forza
        avere orbite organizzate in questo modo, anche perch� Plutone non
        � certo un pianeta "naturalmente" formatosi assieme agli altri, ma
        pi� probabilmente un satellite sfuggito o un corpo della nube di
        Oort catturato dal Sole. Per� bisogna dire che le influenze delle
        orbite dei pianeti interni, col proseguire della successione di
        Keplero, diventano sempre meno significative. Penso che tale
        successione, semplicemente, debba essere in qualche modo limitata,
        a un certo punto: � improbabile che ci siano pianeti in orbita
        stabile a distanze come quelle risultanti per le orbite oltre
        l'ottava. Noctis annovera anche stelle con ben 20 pianeti!

        Come ultima annotazione, il raggio delle orbite � influenzato
        anche dalla massa dei pianeti. Pianeti che hanno all'interno delle
        loro orbite giganti gassosi saranno un po' pi� lontani della media
        perch� altrimenti le loro orbite potrebbero essere troppo
        destabilizzate dalla massa dei giganti. */
no_moons:
    key_radius = nearstar_ray * planet_orb_scaling;

    if (nearstar_class == 8) {
        key_radius *= 2;
    }

    if (nearstar_class == 2) {
        key_radius *= 16;
    }

    if (nearstar_class == 7) {
        key_radius *= 18;
    }

    if (nearstar_class == 11) {
        key_radius *= 20;
    }

    for (n = 0; n < nearstar_nop; n++) {
        nearstar_p_ray[n] =
            avg_planet_ray[nearstar_p_type[n]] + avg_planet_ray[nearstar_p_type[n]] * zrandom(100) / 200;
        nearstar_p_ray[n] *= avg_planet_sizing;
        nearstar_p_orb_ray[n] = key_radius + key_radius * zrandom(100) / 500;
        nearstar_p_orb_ray[n] += key_radius * avg_planet_ray[nearstar_p_type[n]];

        if (n < 8) {
            key_radius += nearstar_p_orb_ray[n];
        } else {
            key_radius += 0.22 * nearstar_p_orb_ray[n];
        }
    }

    /*  Ri-Normalizzazione delle dimensioni delle lune,
        normalizzazione orbite lunari in base al principio di Keplero,
        a sua volta rielaborato come nelle precedenti annotazioni,
        solo che la limitazione avviene per orbite oltre la terza al 12%,
        ed � molto pi� effettiva oltre l'ottava orbita (al 2.5%). */
    n = nearstar_nop;

    while (n < nearstar_nob) {
        q          = 0;
        c          = nearstar_p_owner[n];
        key_radius = nearstar_p_ray[c] * moon_orb_scaling;

        while (n < nearstar_nob && nearstar_p_owner[n] == c) {
            nearstar_p_ray[n] =
                avg_planet_ray[nearstar_p_type[n]] + avg_planet_ray[nearstar_p_type[n]] * zrandom(100) / 200;
            nearstar_p_ray[n] *= avg_moon_sizing;
            nearstar_p_orb_ray[n] = key_radius + key_radius * zrandom(100) / 250;
            nearstar_p_orb_ray[n] += key_radius * avg_planet_ray[nearstar_p_type[n]];

            if (q < 2) {
                key_radius += nearstar_p_orb_ray[n];
            }

            if (q >= 2 && q < 8) {
                key_radius += 0.12 * nearstar_p_orb_ray[n];
            }

            if (q >= 8) {
                key_radius += 0.025 * nearstar_p_orb_ray[n];
            }

            q++;
            n++;
        }
    }

    /* Eliminazione di anelli improbabili. */

    for (n = 0; n < nearstar_nop; n++) {
        // A meno di un raggio e mezzo dal centro del pianeta,
        // sar� un po' difficile trovarci un anello stabile.
        nearstar_p_ring[n] = 0.75 * nearstar_p_ray[n] * (2 + brtl_random(3));
        // I pianeti piccoli raramente hanno degli anelli.
        // Non hanno abbastanza massa per frantumare
        // una luna che arrivi troppo vicina.
        s = nearstar_p_type[n];

        if (s != 6 && s != 9) {
            if (brtl_random(5)) {
                nearstar_p_ring[n] = 0;
            }
        } else {
            if (brtl_random(2)) {
                nearstar_p_ring[n] = 0;
            }
        }
    }

    /* Conteggio degli oggetti che hanno un nome (suggerimento di Ryan) */
    nearstar_labeled = 0;

    for (n = 1; n <= nearstar_nob; n++) {
        if (search_id_code(nearstar_identity + n, 'P') != -1) {
            nearstar_labeled++;
        }
    }

    /*  Reset dei periodi di rotazione
        (vengono calcolati con la superficie) */

    for (n = 0; n < nearstar_nob; n++) {
        nearstar_p_rtperiod[n] = 0;
    }

    // We now have all planets and can call the callbacks
    for (n = 0; n < nearstar_nob; n++) {
        double seedval;
        if (nearstar_p_owner[n] > -1) {
            if (nearstar_p_type[n]) {
                seedval = 1000000.0 * nearstar_ray * nearstar_p_type[n] * nearstar_p_orb_orient[n];
            } else {
                seedval = 2000000.0 * n * nearstar_ray * nearstar_p_orb_orient[n];
            }
        } else {
            if (nearstar_p_type[n]) {
                seedval = 1000000.0 * nearstar_p_type[n] * nearstar_p_orb_seed[n] * nearstar_p_orb_tilt[n] *
                                        nearstar_p_orb_ecc[n] * nearstar_p_orb_orient[n];
            } else {
                seedval = 2000000.0 * n * nearstar_p_orb_seed[n] * nearstar_p_orb_tilt[n] *
                                        nearstar_p_orb_ecc[n] * nearstar_p_orb_orient[n];
            }
        }

        nearstar_p_seedval[n] = seedval;
        nearstar_p_identity[n] = nearstar_identity + n + 1; // note; a planet's ID code is determined by the in-game body number, which starts at 1 (NOT zero)
        planet_xyz(n);
        calculate_planet_spin(n, seedval);
        // int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t viewpoint, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist
        if (onPlanetFound != nullptr) {
            onPlanetFound(
                n,
                nearstar_p_identity[n],
                nearstar_p_seedval[n],
                nearstar_p_plx[n],
                nearstar_p_ply[n],
                nearstar_p_plz[n],
                nearstar_p_type[n],
                nearstar_p_owner[n],
                nearstar_p_moonid[n],
                nearstar_p_ring[n],
                nearstar_p_tilt[n],
                nearstar_p_ray[n],
                nearstar_p_orb_ray[n],
                nearstar_p_orb_tilt[n],
                nearstar_p_orb_orient[n],
                nearstar_p_orb_ecc[n],
                nearstar_p_rtperiod[n],
                nearstar_p_rotation[n],
                nearstar_p_viewpoint[n],
                nearstar_p_term_start[n],
                nearstar_p_term_end[n],
                nearstar_p_qsortindex[n],
                nearstar_p_qsortdist[n]
            );
        }
    }
}

// Smooth the surface of a planet: fast 4x4 average.

void ssmooth(uint8_t *target) {
    uint32_t limit = ((uint32_t) QUADWORDS << 2u) - (360u << 2u);

    for (uint32_t i = 0; i < limit; i++) {
        // 4 columns of 4 pixels each.
        uint8_t col1, col2, col3, col4, average;

        col1 = target[i] + target[i + 360] + target[i + 720] + target[i + 1080];

        col2 = target[i + 1] + target[i + 361] + target[i + 721] + target[i + 1081];

        col3 = target[i + 2] + target[i + 362] + target[i + 722] + target[i + 1082];

        col4 = target[i + 3] + target[i + 363] + target[i + 723] + target[i + 1083];

        col1 = (col1 & 0xFCu) / 4;
        col2 = (col2 & 0xFCu) / 4;
        col3 = (col3 & 0xFCu) / 4;
        col4 = (col4 & 0xFCu) / 4;

        average = col1 + col2 + col3 + col4;
        average /= 4;
        target[i + 360] = average;
    }
}

/* Smooth the surface of a planet slightly: 2x2 average. */
void lssmooth(uint8_t *target) {
    // Offset limit by additional -1 because valgrind told me this ran over.
    uint32_t limit = ((((uint32_t) QUADWORDS) - 80) << 2u) - 1;

    for (uint32_t i = 0; i < limit; i++) {
        uint8_t sample = target[i] & 0xC0u;

        uint8_t dl = target[i] & 0x3Fu;
        uint8_t dh = target[i + 1] & 0x3Fu;

        uint8_t bl = target[i + 360] & 0x3Fu;
        uint8_t bh = target[i + 361] & 0x3Fu;

        uint8_t average = (dl + dh + bl + bh) / 4;

        sample |= average;
        target[i] = sample;
    }
}

int16_t gr, r, g, b, cr, cx, cy;
float a, kfract = 2;
int8_t lave, crays;
uint16_t px, py;

/* Modular functions to detail surfaces. Called only from surface. The
 * parameters are passed in used variables: c, gr, r, g, b, cr, cx, cy, lave,
 * crays, px, py, and a.
 *
 * kfract is the density of fractures on the planets. It is placed at 1 to make
 * lightning in the sky.
 *
 * They all operate on p_background even though, at the time of definition of
 * the surface of a moon, p_background will be exchanged with s_background,
 * which initially is the map of the star surface. The fact that a moon is
 * likely to see the planet around which it orbits means we must separate the
 * planetary and lunar map. We are not worried however, about having more than
 * two visible bodies, because:
 *
 *      - The first and second planet of any non-star cannot, by convention,
 *      have any moons. On the other hand, since the star surface is not visible
 *      in detail from the third planet on, it can be approximated by a white
 *      globe.
 *
 *      - The moons visible from another moon always appear rather small, so it
 *      is not possible to see the details of the surface of one moon from the
 *      point of view of another.
 *
 * Finally, the same considerations on surface maps apply to color maps: The
 * color map of the star is swapped out for that of the moon.
 */

// A small light spot on the surface.
void spot() {
    uint8_t color = p_background[(uint16_t) (py + px)] + gr;
    if (color > 0x3E) {
        color = 0x3E;
    }
    p_background[(uint16_t) (py + px)] = color;
}

// Permanent storm (a colossal stain).
void permanent_storm() {
    for (g = 1; g < cr; g++) {
        for (a = 0; a < 2 * M_PI; a += 4 * deg) {
            px = (uint16_t) (cx + g * cos((double) a));
            py = (uint16_t) (cy + g * sin((double) a));
            py *= 360;
            spot();
        }
    }
}

// A crater.
void crater() {
    uint16_t temp_vptr = 0;

    for (a = 0; a < 2 * M_PI; a += 4 * deg) {
        for (gr = 0; gr < cr; gr++) {
            px        = cx + cos((double) a) * gr;
            py        = cy + sin((double) a) * gr;
            temp_vptr = px + 360 * py;

            uint8_t color       = p_background[temp_vptr];
            uint8_t colorOffset = gr >> lave;
            if (color >= colorOffset) {
                p_background[temp_vptr] = color - colorOffset;
            } else {
                p_background[temp_vptr] = 0;
            }
        }

        p_background[temp_vptr]     = 0x3E;
        p_background[temp_vptr + 1] = 0x01;

        if (crays && !brtl_random(crays)) {
            b = (2 + brtl_random(2)) * cr;

            if (cy - b > 0 && cy + b < 179) {
                for (gr = cr + 1; gr < b; gr++) {
                    px        = cx + cos((double) a) * gr;
                    py        = cy + sin((double) a) * gr;
                    temp_vptr = px + 360 * py;

                    uint8_t color = p_background[temp_vptr] + ((uint8_t) cr);
                    if (color > 0x3E) {
                        color = 0x3E;
                    }
                    p_background[temp_vptr] = color;
                }
            }
        }
    }
}

// Horizontal dark band: Can be made light be negating the surface from 0x3E
void band() {
    for (uint16_t i = cr, j = py; i > 0; i--, j++) {
        uint8_t color = p_background[j];
        if (color >= g) {
            color -= g;
        } else {
            color = 0;
        }
        p_background[j] = color;
    }
}

// A band, like the above but wavy.
void wave() {
    for (uint16_t i = 360; i > 0; i--) {
        py                  = ((uint16_t) round(cr * sin(a * i))) + cy;
        uint16_t index      = ((uint16_t) (py * 360)) + i;
        p_background[index] = 0;
    }
}

void fracture(uint8_t *target, float max_latitude) {
    /* Dark furrow: like the lines on Europe. Has parameters because it is also
     * used to simulate lightning when it rains on the surface of habitable
     * planets.
     */
    int16_t rand0 = brtl_random(360);
    a             = (float) (rand0 * deg);
    gr++;
    float px = cx;
    float py = cy;

    do {
        a += (float) ((brtl_random(g) - brtl_random(g)) * deg); // NOLINT(misc-redundant-expression)
        px += kfract * cos(a);

        if (px > 359) {
            px -= 360;
        }

        if (px < 0) {
            px += 360;
        }

        py += kfract * sin((double) a);

        if (py > max_latitude - 1) {
            py -= max_latitude;
        }

        if (py < 0) {
            py += max_latitude;
        }

        uint16_t temp_vptr = px + (float) (360 * (uint16_t) py);
        target[temp_vptr] >>= (uint8_t) b;
        gr--;
    } while (gr);
}

void volcano() { // un krakatoa volcano con Gedeone il gigante coglione.
    for (a = 0; a < 2 * M_PI; a += 4 * deg) {
        b = gr;

        for (g = cr / 2; g < cr; g++) {
            px = (uint16_t) (cx + cos((double) a) * g);
            py = (uint16_t) (cy + sin((double) a) * g);
            py *= 360;
            spot();
            gr--;

            if (gr < 0) {
                gr = 0;
            }
        }

        gr = b;
    }
}

void contrast(float kt, float kq, float thrshld) {
    uint16_t c;

    for (c = 0; c < 64800; c++) {
        a = p_background[c];
        a -= thrshld;

        if (a > 0) {
            a *= kt;
        } else {
            a *= kq;
        }

        a += thrshld;

        if (a < 0) {
            a = 0;
        }

        if (a > 63) {
            a = 63;
        }

        p_background[c] = (uint8_t) a;
    }
}

void randoface(int16_t range, int16_t upon) {
    uint16_t c;

    for (c = 0; c < 64800; c++) {
        gr = p_background[c];

        if ((upon > 0 && gr >= upon) || (upon < 0 && gr <= -upon)) {
            gr += brtl_random(range);
            gr -= brtl_random(range);

            if (gr > 63) {
                gr = 63;
            }

            if (gr < 0) {
                gr = 0;
            }

            p_background[c] = gr;
        }
    }
}

void negate() {
    for (uint16_t i = 64800, j = 0; i > 0; i--, j++) {
        p_background[j] = 0x3E - p_background[j];
    }
}

void crater_juice() {
    lave  = brtl_random(3);
    crays = brtl_random(3) * 2;

    for (int16_t c = 0; c < r; c++) {
        cx = brtl_random(360);
        cr = 2 + brtl_random(1 + r - c);

        while (cr > 20) {
            cr -= 10;
        }

        cy = brtl_random(178 - 2 * cr) + cr;
        crater();

        if (cr > 15) {
            lssmooth(p_background);
        }
    }
}

/* Atmospheric mapping functions. They work like the previous ones, but operate
 * on "objectschart" instead of on "p_background", and at halved resolution.
 * Moreover, the existence of albedo is VERY IMPORTANT if the clouds do not go
 * from 0x00 to 0x3E but rather from 0x00 to 0x1F. That is because at the time
 * of descent on the surface, the average albedo of p_background is used to
 * determine, on habitable planets, if the player is landing on the sea or not.
 * Since the albedo of p_background is altered by that of the clouds contained
 * in objectschart, it is restored when the landing position is chosen, from the
 * "planets" function, by subtracting the clouds albedo from that of the landing
 * position's p_background. If, however, the alteration due to the clouds is
 * left, and the albedo is based on 0x00 to 0x3E, there is a risk of having an
 * overflow, which could result in improper ocean landing calculations. The
 * result would be that in an area normally covered with earth, a large cloud
 * passing over would cause it to become sea.
 */

// A small light spot (bright cloud).
void cirrus() {
    uint16_t index = ((uint16_t) (py + px)) / 2;
    uint8_t val    = ((uint8_t *) objectschart)[index] + gr;
    if (val > 0x1F) {
        val = 0x1F;
    }

    ((uint8_t *) objectschart)[index] = val;
}

// Atmospheric cyclone: a cluster of spiral clouds.
void atm_cyclon() {
    b = 0;

    while (cr > 0) {
        px = cx + cr * cos(a);
        py = cy + cr * sin(a);
        py *= 360;
        cirrus();
        px += brtl_random(4);
        cirrus();
        py += 359;
        cirrus();
        px -= brtl_random(4);
        cirrus();
        py += 361;
        cirrus();
        px += brtl_random(4);
        cirrus();
        b++;
        b %= g;

        if (!b) {
            cr--;
        }

        a += 6 * deg;
    }
}

void storm() { // tempesta (una grande macchia chiara sull'atmosfera).
    for (g = 1; g < cr; g++) {
        for (a = 0; a < 2 * M_PI; a += 4 * deg) {
            px = (uint16_t) (cx + (int16_t) (g * cos((double) a)));
            py = (uint16_t) (cy + (int16_t) (g * sin((double) a)));
            py *= 360;
            cirrus();
        }
    }
}

/*
 * Calculate the planet spin
 * Added by JORIS on 2023-07-31; used to be part of void surface() only...
 * Note that this sets the random seed!
 * 
 * Returns 'plwp'..
 */
int16_t calculate_planet_spin(int16_t logical_id, double seedval) {
    int16_t plwp;
    /* Setting of the rotation period. "rotation" represents the current
     * rotation of the planet, in degrees, from 0 to 359. The rotation period is
     * extracted in a very wide range, with 1 second resolution.
     */
    fast_srand(((int32_t) (uint32_t) seedval) + 4112);

    /* "rtperiod" is the time, in seconds, that it takes the planet to rotate
     * one degree on its axis. The time taken for a complete rotation is
     * therefore 360 * rtperiod.
     */
    nearstar_p_rtperiod[logical_id] =
        10.0 * (ranged_fast_random(50) + 1) + 10.0 * ranged_fast_random(25) + ranged_fast_random(250) + 41;
    nearstar_p_rotation[logical_id] = int16_t(fmod(secs / nearstar_p_rtperiod[logical_id], 360)); // JORIS FIX: this is now no longer occasionally negative

    /* Calculation of the planet's current orientation for the masking of the
     * dark side (with respect to the position of the star).
     */
    nearstar_p_viewpoint[logical_id] = cplx_planet_viewpoint(logical_id);
    plwp = 89 - nearstar_p_viewpoint[logical_id];
    plwp += nearstar_p_rotation[logical_id];
    plwp %= 360;

    if (plwp < 0) {
        plwp += 360;
    }

    nearstar_p_term_start[logical_id] = plwp + 35;

    if (nearstar_p_term_start[logical_id] >= 360) {
        nearstar_p_term_start[logical_id] -= 360;
    }

    nearstar_p_term_end[logical_id] = nearstar_p_term_start[logical_id] + 130;

    if (nearstar_p_term_end[logical_id] >= 360) {
        nearstar_p_term_end[logical_id] -= 360;
    }

    return plwp;
}

/* Calculate the surface by extapolating it from the data on the planet and from
 * the pseudo-random table assigned to it. Includes the day-night terminator by
 * darkening the night hemisphere for an angle of 130 (not 180 due to the
 * diffused light and the reduced field at the edges of the globes). "colorbase"
 * is assigned to 192 for the planets, to 128 for the moons.
 * 
 * 'lighting' boolean was added by JORIS on 2023-07-28, to allow enabling/disabling 
 * of day-night terminator application.
 * 'include_atmosphere' boolean was added by JORIS on 2023-07-28, to allow turning off
 * the inclusion of the atmosphere in the p_surfacemap (thus allowing separate textures
 * to be retrieved for surface and atmosphere)
 */

void surface(int16_t logical_id, int16_t type, double seedval, uint8_t colorbase, bool lighting, bool include_atmosphere) {
#ifdef WITH_GODOT
    godot::UtilityFunctions::print("Seed val: ", seedval);
#endif
    int16_t plwp, c;
    uint16_t seed = 0;
    int8_t knot1  = 0, brt;
    int16_t QW    = QUADWORDS;
    float r1, r2, r3, g1, g2, g3, b1, b2, b3;
    uint8_t *overlay = (uint8_t *) objectschart;
    if (type == 10) {
        return; // Companion star: has star surface...
    }

    plwp = calculate_planet_spin(logical_id, seedval);

    /* Selection of the pseudo-table relative to this planet. The pseudo-table
     * of the "random" function in C++ has a fair probability of recurrence, but
     * with "ranged_fast_random", the recurrence is cancelled
     * ("ranged_fast_random" has a huge amount of tables).
     *
     * NOTE: The above comment is preserved for posterity, but its statements on
     * the standard library random functions are not necessarily accurate.
     */
    fast_srand((int32_t) (uint32_t) (seedval * 10));
    seed = fast_random(0xFFFF);

    /* Preparation of a standard surface (random pattern 0 .. 62): it is then
     * processed according to the type of planet.
     */
    brtl_srand(seed);
    int16_t currSeed = seed;
    for (uint16_t i = 64800, j = 0; i > 0; i--, j++) {
        currSeed += i;
        int32_t result  = ((int32_t) currSeed) * ((int32_t) currSeed);
        auto resultHigh = static_cast<int16_t>(result >> 16u);
        auto resultLow  = static_cast<int16_t>(result & 0xFFFFu);
        currSeed        = resultLow + resultHigh;
        uint8_t color   = currSeed & 0xFFu;
        color &= 0x3Eu;
        p_background[j] = color;
    }


    /* Preparation of the overlay for mapping the changes in the atmosphere: it
     * is initially reset, as it must be subsequently reworked according to the
     * type of planet. Obviously the planet is not reworked if it has no
     * atmosphere.
     */
    memset(overlay, 0, 32400);
    /* Specific surface processing. Atmosphere overlay processing, if there is
     * a need for it, is also done.
     */
    brtl_srand(seed);
    QUADWORDS = 16200;

    int16_t rVal0 = 0, rVal1 = 0, rVal2 = 0;
    switch (type) {
    case 0:
        r = ranged_fast_random(3) + 5;

        for (c = 0; c < r; c++) {
            ssmooth(p_background);
        }

        for (uint16_t i = 64800, j = 0; i > 0; i--, j++) {
            if (p_background[j] >= 28) {
                p_background[j] = 62;
            }
        }

        r = ranged_fast_random(5) + 5;

        for (c = 0; c < r; c++) {
            ssmooth(p_background);
        }

        r = 5 + ranged_fast_random(26);

        for (c = 0; c < r; c++) {
            cr = 5 + ranged_fast_random(20);
            cx = ranged_fast_random(360);
            cy = ranged_fast_random(130) + 25;
            gr = ranged_fast_random(cr / 2) + cr / 2 + 2;
            volcano();
        }

        r = 100 + ranged_fast_random(100);
        b = ranged_fast_random(3) + 1;
        g = 360;

        for (c = 0; c < r; c++) {
            cx = ranged_fast_random(360);
            cy = ranged_fast_random(180);
            gr = ranged_fast_random(100);
            fracture(p_background, 180);
        }

        lssmooth(p_background);
        break;

    case 1:
        if (ranged_fast_random(2)) {
            ssmooth(p_background);
        }

        r = 10 + ranged_fast_random(41);
        crater_juice();
        lssmooth(p_background);

        if (!ranged_fast_random(5)) {
            negate();
        }

        break;

    case 2:
        r = 5 + ranged_fast_random(25);

        for (c = 0; c < r; c++) {
            cr = ranged_fast_random(20) + 1;
            cy = ranged_fast_random(178 - 2 * cr) + cr;

            switch (brtl_random(2)) {
            case 0:
                cx = (((int32_t) (uint32_t) (10 * secs)) / (ranged_fast_random(3600) + 180)) % 360;
                gr = ranged_fast_random(12) + 2;
                storm();
                break;

            case 1:
                gr = ranged_fast_random(15) + 3;
                py = cy * 360;
                cr *= 360;
                g = 1 + ranged_fast_random(gr);
                band();
            }
        }

        if (!ranged_fast_random(3)) {
            negate();
        }

        break;

    case 3: {
        r = ranged_fast_random(3) + 4;
        g = 26 + ranged_fast_random(3) - ranged_fast_random(5);

        for (c = 0; c < r; c++) {
            ssmooth(p_background);
        }

        uint16_t ax = seed;
        uint16_t* di = (uint16_t*)p_background;
        for (uint16_t cx = 64000; cx > 0; cx--) {
            if (*((uint8_t*)di) < g) {
                *((uint8_t*)di) = 16;
            } else {
                ax = ax + cx;
                int32_t temp = (int16_t)ax * (int16_t)ax;  // Signed multiplication!
                uint16_t dx = temp >> 16;        // High word goes to dx
                ax = (uint16_t)temp;             // Low word goes to ax
                ax = (uint16_t)((uint32_t)ax + dx);  // Add with wrap at 16 bits
                uint8_t bl = (uint8_t)ax;        // Take low byte
                bl &= 0x3E;                      // Mask with 0x3E
                uint8_t& byte_at_di = *((uint8_t*)di);
                byte_at_di += bl;
                if (byte_at_di >= 0x3E) {
                    *di = 0x3E;
                }
            }
            di = (uint16_t*)((uint8_t*)di + 1);
        }

        r = 20 + ranged_fast_random(40);

        for (c = 0; c < r; c++) {
            gr = ranged_fast_random(5) + 1;
            cr = ranged_fast_random(10) + 10;

            if (ranged_fast_random(3)) {
                cy = ranged_fast_random(172 - 2 * cr) + cr + 2;
            } else {
                cy = 60 + ranged_fast_random(10) - ranged_fast_random(10);
            }

            cx = ((int32_t) (secs) / (ranged_fast_random(360) + 180)) % 360;
            g  = ranged_fast_random(5) + 7;
            a  = ranged_fast_random(360) * deg;
            atm_cyclon();
        }
    } break;

    case 4:
        ssmooth(p_background);

        if (ranged_fast_random(2)) {
            ssmooth(p_background);
        }

        for (uint16_t i = 64000, j = 0; i > 0; i--, j++) {
            if (p_background[j] == 32) {
                p_background[j]       = 0x01;
                p_background[j + 1]   = 0x3E;
                p_background[j + 360] = 0x01;
            }
        }

        r = ranged_fast_random(30);

        if (r > 20) {
            r *= 10;
        }

        b = ranged_fast_random(3) + 1;
        g = 200 + ranged_fast_random(300);

        for (c = 0; c < r; c++) {
            cx = ranged_fast_random(360);
            cy = ranged_fast_random(180);
            gr = 50 + ranged_fast_random(100);
            fracture(p_background, 180);
        }

        r = ranged_fast_random(25) + 1;
        crater_juice();
        lssmooth(p_background);

        if (ranged_fast_random(2)) {
            lssmooth(p_background);
        }

        break;

    case 5:
        r = ranged_fast_random(3) + 4;

        for (c = 0; c < r; c++) {
            ssmooth(p_background);
        }

        /* These have been pulled out of the function call b/c the borland
         * compiler evaluated them the other way around.
         */
        rVal0 = ranged_fast_random(3);
        rVal1 = ranged_fast_random(350);
        rVal2 = ranged_fast_random(200);
        contrast((float) ((float) rVal2 / 900 + 0.6), (float) ((float) rVal1 / 100 + 4.0), (float) (25 + rVal0));
        rVal0 = ranged_fast_random(3);
        rVal1 = ranged_fast_random(3);
        randoface(5 + rVal1, -20 * (rVal0 + 1));
        r = 5 + ranged_fast_random(5);

        for (c = 0; c < r; c++) {
            cr = 5 + ranged_fast_random(10);
            cx = ranged_fast_random(360);
            cy = ranged_fast_random(145) + 15;
            gr = ranged_fast_random(cr / 2) + 2;
            volcano();
        }

        r = 5 + ranged_fast_random(5);

        for (c = 0; c < r; c++) {
            cr = ranged_fast_random(30) + 1;
            cy = ranged_fast_random(178 - 2 * cr) + cr;
            cx = ((int32_t) (60 * secs) / (ranged_fast_random(3600) + 360)) % 360;
            gr = ranged_fast_random(2) + 1;
            permanent_storm();
        }

        for (c = 0; c < 10000; c++) {
            gr = ranged_fast_random(10) + 10;
            px = ranged_fast_random(360);
            py = ranged_fast_random(10);
            py *= 360;
            spot();
            px = ranged_fast_random(360);
            py = 125 - ranged_fast_random(10);
            py *= 360;
            spot();
        }

        if (ranged_fast_random(2)) {
            ssmooth(p_background);
        } else {
            lssmooth(p_background);
        }

        break;

    case 6:
        r = 3 + ranged_fast_random(5);

        for (c = 0; c < r; c++) {
            ssmooth(p_background);
        }

        r = 50 + ranged_fast_random(100);

        for (c = 0; c < r; c++) {
            cr = ranged_fast_random(10) + 1;
            cy = ranged_fast_random(178 - 2 * cr) + cr;

            if (ranged_fast_random(8)) {
                gr = ranged_fast_random(5) + 2;
                g  = 1 + ranged_fast_random(gr);
                py = cy * 360;
                cr *= 360;
                band();
            } else {
                a  = (float) (5 + ranged_fast_random(10)) / 30;
                cr = cr / 4 + 1;
                wave();
            }
        }

        r = 50 + ranged_fast_random(100);

        for (c = 0; c < r; c++) {
            cr = ranged_fast_random(15) + 1;
            cy = ranged_fast_random(178 - 2 * cr) + cr;
            cx = ((int32_t) (60 * secs) / (ranged_fast_random(8000) + 360)) % 360;
            gr = ranged_fast_random(2) + 1;

            if (ranged_fast_random(10)) {
                cr = cr / 2 + 1;
            } else {
                gr *= 3;
            }

            storm();
        }

        lssmooth(p_background);

        if (!ranged_fast_random(3)) {
            negate();
        }

        break;

    case 7:
        r = 5 + ranged_fast_random(5);

        for (c = 0; c < r; c++) {
            ssmooth(p_background);
        }

        r = 10 + ranged_fast_random(50);
        g = 5 + ranged_fast_random(20);
        b = ranged_fast_random(2) + 1;

        for (c = 0; c < r; c++) {
            cx = ranged_fast_random(360);
            cy = ranged_fast_random(180);
            gr = ranged_fast_random(300);
            fracture(p_background, 180);
        }

        if (ranged_fast_random(2)) {
            lssmooth(p_background);
        }

        randoface(1 + ranged_fast_random(10), 1);

        if (ranged_fast_random(2)) {
            negate();
        }

        break;

    case 8:
        r = ranged_fast_random(10) + 1;

        for (c = 0; c < r; c++) {
            lssmooth(p_background);
        }

        r = 100 + ranged_fast_random(50);

        for (c = 0; c < r; c++) {
            cr = ranged_fast_random(5) + 1;
            gr = ranged_fast_random(5) + 1;
            cx = ranged_fast_random(360);
            cy = ranged_fast_random(178 - 2 * cr) + cr;
            permanent_storm();
        }

        if (ranged_fast_random(2)) {
            negate();
        }

        break;

    case 9:
        memset(adapted, 0x1F, QUADWORDS * 4);

        for (px = 0; px < 32400; px++) {
            overlay[px] = 0x1F;
        }

        break;
    default:
        break;
    }
    /* Surface renormalization at interval 0x00..0x1F: Only if the planet's
     * atmosphere doesn't have an effect on the appearance of the underlying
     * surface, then for Felisian / Mars-like planets.
     */
    if (type == 3 || type == 5) {
        for (px = 0; px < 64800; px++) {
            p_background[px] >>= 1u;
        }
    }

    // Specific final touches to the surface for Felisian planets.
    if (type == 3) {
        if (ranged_fast_random(2)) {
            lssmooth(p_background);
        } else {
            ssmooth(p_background);
        }
    }

    /* Terrain map + atmosphere overlay fusion and renormalization of the
     * terrain map so that it adapts to the range 0x00..0x1F.
     */
    for (px = 0, py = 0; px < 32400; py += 2, px++) {
        if (include_atmosphere) {
            p_background[py] += overlay[px];
            p_background[py + 1] += overlay[px];
        }

        if (p_background[py] > 0x3E) {
            p_background[py] = 0x3E;
        }


        if (p_background[py + 1] > 0x3E) {
            p_background[py + 1] = 0x3E;
        }
    }

    // Specific final touches to the surface for Venusian planets.
    if (type == 2) {
        if (!brtl_random(3)) {
            psmooth_grays(p_background);
            knot1 = 1;
        }
    }

    // Day-night terminator application.
    if (lighting) {
        for (uint16_t i = 179, j = (plwp + 35); i > 0; i--, j += 230) {
            for (uint16_t k = 130; k > 0; k--) {
                p_background[j] >>= 2u;
                j++;
            }
        }
    }


    // Specific final touches to the surface for other planets.
    if (type == 2) {
        if (knot1) {
            ssmooth(p_background);
        } else {
            r = 3 + ranged_fast_random(5);

            for (c = 0; c < r; c++) {
                ssmooth(p_background);
            }
        }
    }

    if (type == 6) {
        for (c = 0; c < 3; c++)
            if (ranged_fast_random(2)) {
                ssmooth(p_background);
            }
    }

    if (type == 9)
        for (c = 0; c < 6; c++) {
            ssmooth(p_background);
        }

    // Color table processing (redefines from 192 to 255).
    if (colorbase == 255) {
        QUADWORDS = QW;
        return;
    }


    type = ((uint16_t) type) << 2;
    r    = planet_rgb_and_var[type + 0];
    g    = planet_rgb_and_var[type + 1];
    b    = planet_rgb_and_var[type + 2];
    c    = planet_rgb_and_var[type + 3];
    r <<= 1;
    r += nearstar_r;
    r >>= 1;
    g <<= 1;
    g += nearstar_g;
    g >>= 1;
    b <<= 1;
    b += nearstar_b;
    b >>= 1;
    r1 = r + brtl_random(c);
    r1 = r1 - brtl_random(c);
    g1 = g + brtl_random(c);
    g1 = g1 - brtl_random(c);
    b1 = b + brtl_random(c);
    b1 = b1 - brtl_random(c);
    r2 = r + brtl_random(c);
    r2 = r2 - brtl_random(c);
    g2 = g + brtl_random(c);
    g2 = g2 - brtl_random(c);
    b2 = b + brtl_random(c);
    b2 = b2 - brtl_random(c);
    r3 = r + brtl_random(c);
    r3 = r3 - brtl_random(c);
    g3 = g + brtl_random(c);
    g3 = g3 - brtl_random(c);
    b3 = b + brtl_random(c);
    b3 = b3 - brtl_random(c);
    r1 *= 0.25;
    g1 *= 0.25;
    b1 *= 0.25;
    r2 *= 0.75;
    g2 *= 0.75;
    b2 *= 0.75;
    r3 *= 1.25;
    g3 *= 1.25;
    b3 *= 1.25;
    type >>= 2;
    shade(tmppal, colorbase + 00, 16, 00, 00, 00, r1, g1, b1);
    shade(tmppal, colorbase + 16, 16, r1, g1, b1, r2, g2, b2);
    shade(tmppal, colorbase + 32, 16, r2, g2, b2, r3, g3, b3);
    shade(tmppal, colorbase + 48, 16, r3, g3, b3, 64, 64, 64);
    brt = nearstar_p_owner[logical_id];

    if (brt == -1) {
        brt = logical_id;
    }

    if (brt <= 4) {
        brt = 64;
    } else {
        brt = 64 - (4 * (brt - 4));
    }

    tavola_colori(tmppal + 3 * colorbase, colorbase, 64, brt, brt, brt);
    QUADWORDS = QW;

}

/* Tracciamento degli anelli (eventuali). */

void ring(int16_t planet_id, double ox, double oy, double oz, int16_t start, int16_t layers) {
    int16_t a, b, c, n, m, partn, partcls;
    double sx, sy, sz;
    double ringray  = nearstar_p_ring[planet_id];
    double ringtilt = 0.1 * ringray * nearstar_p_tilt[planet_id];
    double interval = 0.0075 * ringray;
    fast_srand((int32_t) (10000 * ringray + planet_id));
    b       = 1 + fast_random(0x1F) - layers;
    partcls = 1 + fast_random(3);
    //godot::UtilityFunctions::printt("ring(", planet_id, ") b is", b, ", layers is ", layers, "  ---- xyz: ", ox , ", ", oy, ", ", oz);
    //godot::UtilityFunctions::printt("partcls is", partcls);

    while (b > 0) {
        if (!fast_random(7)) {
            c             = 1 + fast_random(3);
            pixel_spreads = 0;
        } else {
            c             = 1 + fast_random(7);
            pixel_spreads = 1;
        }

        a                 = start - (start % c);
        pixilating_effect = fast_random(1) + fast_random(1);

        if (a < 0) {
            a += 360;
        }

        n = c;

        while (n + c < 180) {
            m = c + fast_random(1);
            n += m;
            a += m;

            if (a > 360) {
                a -= 360;
            }

            sy    = oy /*nearstar_p_plx[planet_id]*/ - ringtilt * lft_sin[a];
            sx    = ox /*nearstar_p_ply[planet_id]*/ + ringray * lft_sin[a];
            sz    = oz /*nearstar_p_plz[planet_id]*/ + ringray * lft_cos[a];
            partn = partcls;

            while (partn) {
                sz += interval - (fast_flandom() * interval);
                sx += interval - (fast_flandom() * interval);
                #ifndef WITH_GODOT
                far_pixel_at(sx, sy, sz, -0.042, 0);
                #else
                cb_RingParticleFound(sx, sy, sz, -0.042, 0);
                #endif
                partn--;
            }
        }

        ringray += interval;

        if (!fast_random(7)) {
            ringray += 5 * interval;
        }

        ringtilt = 0.1 * ringray * nearstar_p_tilt[planet_id];
        b--;
    }

    pixilating_effect = LIGHT_EMITTING;
    pixel_spreads     = 1;
}


/*  Visualizza appopriatamente i pianeti, come punti, barlumi di luce
    o globi ben visibili, a seconda di distanza e raggio. C'� un terzo
    modo in cui un corpo planetario pu� rendersi visibile: con una falce.
    L'effetto falce viene realizzato da "glowing_globe". */

/*  Appropriately display the planets, as points, glimmers of light,
    or clearly visible globes, depending on distance and radius. There is
    a third way in which a planetary body can make itself visible: with a
    sickle. The scythe effect is done by "glowing_globe". */

/* Galactic Mapping
 *
 * Each star is assigned a unique identification code, resulting from its
 * coordinates. This code is referred to for the association of the names of the
 * stars. The identification codes of planets are calculated starting from their
 * star, plus the (progressive) number of the planet in order of distance.
 */

double laststar_x, laststar_y, laststar_z;

/* Determines whether a certain star, of which the id is specified, is currently
 * in sensor range. Return 1 when visible, 0 if not. When visible, the
 * coordinates of the star are reported in the variables laststar_x/y/z.
 */
int8_t isthere(double star_id) {
    int8_t visible_sectors = 9;
    int32_t sect_x, sect_y, sect_z;
    int32_t k, advance = 100000;
    double sidlow  = star_id - idscale;
    double sidhigh = star_id + idscale;
    uint8_t sx, sy, sz;
    double laststar_id;
    sect_x = (dzat_x - visible_sectors * 50000) / 100000;
    sect_x *= 100000;
    sect_y = (dzat_y - visible_sectors * 50000) / 100000;
    sect_y *= 100000;
    sect_z = (dzat_z - visible_sectors * 50000) / 100000;
    sect_z *= 100000;

    if (field_amplificator) {
        visible_sectors = 14;
    }

    k = 100000 * visible_sectors;

    for (sx = visible_sectors; sx > 0; sx--, sect_y -= k, sect_x += advance) {
        for (sy = visible_sectors; sy > 0; sy--, sect_z -= k, sect_y += advance) {
            for (sz = visible_sectors; sz > 0; sz--, sect_z += advance) {
                // TODO; Cleanup, rename properly. No teletubby names.
                int32_t eax = sect_x;
                int32_t edx = sect_z;

                eax += edx;
                int32_t ecx = eax;
                edx         = eax;
                edx &= 0x0001FFFF;

                edx += sect_x;
                edx -= 0xC350;
                laststar_x = edx;

                int64_t result = (int64_t) edx * (int64_t) eax;
                eax            = result & 0xFFFFFFFF;
                edx            = result >> 32;

                edx += eax;
                ecx += edx;
                edx &= 0x0001FFFF;

                edx += sect_y;
                edx -= 0xC350;
                laststar_y = edx;
                eax        = ecx;

                result = (int64_t) edx * (int64_t) eax;
                eax    = result & 0xFFFFFFFF;
                edx    = result >> 32;

                edx += eax;
                edx &= 0x0001FFFF;

                edx += sect_z;
                edx -= 0xC350;
                laststar_z = edx;

                laststar_x = round(laststar_x);
                laststar_y = round(laststar_y);
                laststar_z = round(laststar_z);

                laststar_id = (laststar_x * idscale) * (laststar_y * idscale) * (laststar_z * idscale);

                if (laststar_id > sidlow && laststar_id < sidhigh) {
                    return 1;
                }
            }
        }
    }

    return 0;
}

/*  Search all visible known stars, up to 50 at once. Fill out a table
    containing the ID of each star and its location in Parsis coordinates. */

const int16_t tgt_bytes_per_scan = 5 * 32;
int32_t tgt_collect_lastpos      = 0;

int16_t targets           = 0;
int16_t topmost_target    = 0;
int16_t target_line       = 0;
int8_t update_targets     = 0;
int8_t collecting_targets = 0;

double targets_table_id[50];
double targets_table_px[50];
double targets_table_py[50];
double targets_table_pz[50];

void status(const char *status_description, int16_t message_delay) {
    if (message_delay >= fcs_status_delay) {
        strcpy((char *) fcs_status, status_description);
        fcs_status_delay = message_delay;
    }
}

// Character map for the hud, 3x5 pixels.
int8_t digimap[65 * 5] = {
    0, 0, 0, 0, 0, // 32. ' '
    2, 2, 2, 0, 2, // 33. '!'
    5, 0, 0, 0, 0, // 34. '"'
    0, 0, 3, 5, 5, // 35. '#'
    2, 2, 6, 2, 2, // 36. sistro
    1, 4, 2, 1, 4, // 37. '%'
    0, 0, 2, 0, 0, // 38. e commericale (non � possibile visualizzarla)
    0, 2, 2, 0, 0, // 39. apice
    4, 2, 2, 2, 4, // 40. '('
    1, 2, 2, 2, 1, // 41. ')'
    0, 0, 7, 2, 2, // 42. '*'
    0, 2, 7, 2, 0, // 43. segno pi�
    0, 0, 0, 2, 1, // 44. ','
    0, 0, 7, 0, 0, // 45. '-'
    0, 0, 0, 0, 2, // 46. '.'
    0, 4, 2, 1, 0, // 47. barra destrorsa
    7, 5, 5, 5, 7, // 48. '0'
    3, 2, 2, 2, 7, // 49. '1'
    7, 4, 7, 1, 7, // 50. '2'
    7, 4, 6, 4, 7, // 51. '3'
    4, 6, 5, 7, 4, // 52. '4'
    7, 1, 7, 4, 7, // 53. '5'
    7, 1, 7, 5, 7, // 54. '6'
    7, 4, 4, 4, 4, // 55. '7'
    7, 5, 7, 5, 7, // 56. '8'
    7, 5, 7, 4, 4, // 57. '9'
    0, 2, 0, 2, 0, // 58. duepunti
    0, 2, 0, 2, 1, // 59. ';'
    4, 2, 1, 2, 4, // 60. '<'
    0, 7, 0, 7, 0, // 61. '='
    1, 2, 4, 2, 1, // 62. '>'
    7, 4, 6, 0, 2, // 63. '?'
    0, 2, 0, 0, 0, // 64. a commerciale (non visualizzabile)
    7, 5, 7, 5, 5, // 65. 'A'
    7, 5, 3, 5, 7, // 66. 'B'
    7, 1, 1, 1, 7, // 67. 'C'
    3, 5, 5, 5, 3, // 68. 'D'
    7, 1, 3, 1, 7, // 69. 'E'
    7, 1, 3, 1, 1, // 70. 'F'
    7, 1, 5, 5, 7, // 71. 'G'
    5, 5, 7, 5, 5, // 72. 'H'
    2, 2, 2, 2, 2, // 73. 'I'
    4, 4, 4, 5, 7, // 74. 'J'
    5, 5, 3, 5, 5, // 75. 'K'
    1, 1, 1, 1, 7, // 76. 'L'
    7, 7, 5, 5, 5, // 77. 'M'
    5, 7, 7, 5, 5, // 78. 'N'
    7, 5, 5, 5, 7, // 79. 'O'
    7, 5, 7, 1, 1, // 80. 'P'
    7, 5, 5, 1, 5, // 81. 'Q'
    7, 5, 3, 5, 5, // 82. 'R'
    7, 1, 7, 4, 7, // 83. 'S'
    7, 2, 2, 2, 2, // 84. 'T'
    5, 5, 5, 5, 7, // 85. 'U'
    5, 5, 5, 5, 2, // 86. 'V'
    5, 5, 7, 7, 5, // 87. 'W'
    5, 5, 2, 5, 5, // 88. 'X'
    5, 5, 7, 2, 2, // 89. 'Y'
    7, 4, 2, 1, 7, // 90. 'Z'
    0, 0, 6, 2, 2, // 91. '['
    1, 3, 7, 3, 1, // 92. barra sinistrorsa
    2, 2, 6, 0, 0, // 93. ']'
    2, 2, 2, 2, 2, // 94. ordinale femminile
    0, 0, 0, 0, 7, // 95. sottolinea
    1, 2, 0, 0, 0  // 96. accento
};

/*
    Character map for the computer and operating system. The resolution is
    32x36 pixels. It is loaded in n_globes_map + gl_bytes and occupies 10800
    bytes. These 10182 bytes (gl_brest) are practically a tiny extension of
    the n_globes_map area which by itself, containing a semi-texture of 32Kb
    for the sea, has been extended from 22586 to 32768.
*/

uint32_t *digimap2; // Will be assigned to n_globes_map + gl_bytes.

// Panoramic dome.

const float cupsize   = 1800;
const float cupheight = 1667;

// Synchronizer: up to 18 frames per second on any PC.

clock_t gtime;

void sync_start() { gtime = clock(); }

void sync_stop() {
    while (clock() == gtime)
        ;
}

int32_t global_surface_seed;
float ppos_x, ppos_y, ppos_z;
double dsd; // To measure distances.

// Load the bitmap for the star surface.
// Implementation is sketchy at best.
void load_starface() {
    auto seed  = static_cast<uint16_t>(nearstar_identity * 12345);
    int16_t ax = seed;
    for (int i = 0; i < 64800; i++) {
        ax += (64800 - i);
        int32_t result    = ax * ax;
        auto resultHigh   = static_cast<int16_t>(result >> 16u);
        auto resultLow    = static_cast<int16_t>(result & 0xFFFFu);
        int16_t netResult = resultHigh + resultLow;
        auto blarg        = static_cast<uint8_t>(netResult & 0xFFu);
        blarg &= 0x3Eu;
        s_background[i] = blarg;
    }

    int16_t smoothcount;
    fast_srand(seed);
    smoothcount = static_cast<int16_t>(fast_random(3));

    if (nearstar_class == 11 || nearstar_class == 7 || nearstar_class == 2) {
        smoothcount += fast_random(3) + 2;
    }

    while (smoothcount) {
        ssmooth(s_background);
        smoothcount--;
    }
}

void load_QVRmaps() {
    FILE *fh = sa_open(offsets_map);

    if (fh != nullptr) {
        fread(n_offsets_map, 1, om_bytes, fh);
        fclose(fh);
    }

    fh = sa_open(globes_map);

    if (fh != nullptr) {
        fread(n_globes_map, 1, gl_bytes, fh);
        fclose(fh);
    }
}

void load_digimap2() {
    FILE *fh = sa_open(off_digimap2);

    if (fh != nullptr) {
        fread(digimap2, 1, dm2_bytes, fh);
        fclose(fh);
    }
}

int8_t outhudbuffer[81];
const char *compass = "N.........E.........S.........W.........N.........E.........S.......";

float tp_gravity = 1, pp_gravity = 1;
float tp_temp = 22, pp_temp = 22;
float tp_pressure = 1, pp_pressure = 1;
float tp_pulse = 118, pp_pulse = 118;

void wrouthud(uint16_t x, uint16_t y, uint16_t l, const char *text) {
    int32_t j, i, n;
    uint32_t spot;
    n = 0;

    if (!l) {
        l = 32767;
    }

    spot = y * adapted_width + x;

    while (text[n] && n < l) {
        j = (text[n] - 32) * 5;

        for (i = 0; i < 5; i++) {
            if (digimap[j + i] & 1) {
                adapted[spot + 0] = 191 - adapted[spot + 0];
            }

            if (digimap[j + i] & 2) {
                adapted[spot + 1] = 191 - adapted[spot + 1];
            }

            if (digimap[j + i] & 4) {
                adapted[spot + 2] = 191 - adapted[spot + 2];
            }

            spot += adapted_width;
        }

        spot -= adapted_width * 5;
        spot += 4;
        n++;
    }
}

void surrounding(int8_t compass_on, int16_t openhudcount) {
    int16_t cpos, crem;
    int32_t lsecs, lptr;
    float pp_delta, ccom;

    for (lptr = 0; lptr < 04; lptr++) {
        area_clear(adapted, 10, openhudcount + 9 - lptr, 0, 0, adapted_width - 20, 1, 54 + surlight + 3 * lptr);
    }

    for (lptr = 0; lptr < 10; lptr++) {
        area_clear(adapted, 0, 9 - lptr, 0, 0, adapted_width, 1, 64 + surlight - lptr);
    }

    for (lptr = 0; lptr < 10; lptr++) {
        area_clear(adapted, 0, (adapted_height - 10) + lptr, 0, 0, adapted_width, 1, 64 + surlight - lptr);
    }

    for (lptr = 0; lptr < 10; lptr++) {
        area_clear(adapted, 9 - lptr, 10, 0, 0, 1, adapted_height - 20, 64 + surlight - lptr);
    }

    for (lptr = 0; lptr < 10; lptr++) {
        area_clear(adapted, (adapted_width - 10) + lptr, 10, 0, 0, 1, adapted_height, 64 + surlight - lptr);
    }

    lptr = 64 + 3 * surlight;

    if (lptr > 127) {
        lptr = 127;
    }

    area_clear(adapted, 9, 9, 0, 0, 4, 4, lptr);
    smootharound_64(adapted, 9, 9, 5, 1);
    area_clear(adapted, adapted_width - 12, 9, 0, 0, 4, 4, lptr);
    smootharound_64(adapted, adapted_width - 12, 9, 5, 1);
    area_clear(adapted, 9, adapted_height - 12, 0, 0, 4, 4, lptr);
    smootharound_64(adapted, 9, adapted_height - 12, 5, 1);
    area_clear(adapted, adapted_width - 12, adapted_height - 12, 0, 0, 4, 4, lptr);
    smootharound_64(adapted, adapted_width - 12, adapted_height - 12, 5, 1);
    // Print time on outer HUD.
    sprintf((char *) outhudbuffer, "EPOC %d & ", epoc);

    auto sinisters = (uint16_t) (fmod(secs, 1e9) / 1e6);
    // Pad with a 0.
    if (sinisters < 100) {
        strcat((char *) outhudbuffer, "0");
    }
    snprintf(dec, 4, "%hu", sinisters);
    strcat((char *) outhudbuffer, dec);
    strcat((char *) outhudbuffer, ".");

    auto medii = (uint16_t) (fmod(secs, 1e6) / 1e3);
    if (medii < 100) {
        strcat((char *) outhudbuffer, "0");
    }
    snprintf(dec, 4, "%hu", medii);
    strcat((char *) outhudbuffer, dec);
    strcat((char *) outhudbuffer, ".");

    auto dexters = (uint16_t) (fmod(secs, 1e3));
    if (dexters < 100) {
        strcat((char *) outhudbuffer, "0");
    }
    snprintf(dec, 4, "%hu", dexters);
    strcat((char *) outhudbuffer, dec);

    if (compass_on) {
        strcat((char *) outhudbuffer, " & SQC ");
        strcat((char *) outhudbuffer, alphavalue(landing_pt_lon));
        strcat((char *) outhudbuffer, ".");
        strcat((char *) outhudbuffer, alphavalue(landing_pt_lat));
        strcat((char *) outhudbuffer, ":");
        strcat((char *) outhudbuffer, alphavalue((((int32_t) (pos_x)) >> 14u) - 100));
        strcat((char *) outhudbuffer, ".");
        strcat((char *) outhudbuffer, alphavalue((((int32_t) (pos_z)) >> 14u) - 100));
        area_clear(adapted, 254, 1, 0, 0, 5, 7, 64 + 0);
        area_clear(adapted, 256, 8, 0, 0, 1, 1, 64 + 63);
        ccom = 360 - user_beta;

        if (ccom > 359) {
            ccom -= 360;
        }

        cpos = (int16_t) (ccom / 9);
        crem = (int16_t) (ccom * 0.44444);
        wrouthud(200 - (crem % 4), 2, 28, (char *) (compass + cpos));
    } else {
        if (!ontheroof) {
            strcat((char *) outhudbuffer, " & ");

            if (sys == 4) {
                strcat((char *) outhudbuffer, "(5\\FLIGHTCTR R\\DEVICES    P\\PREFS      X\\SCREEN OFF)");
            } else {
                cpos                    = strlen((char *) outhudbuffer);
                outhudbuffer[cpos + 00] = '6';
                outhudbuffer[cpos + 01] = '\\';
                memcpy(outhudbuffer + cpos + 02, ctb + 20 + 27 * 0, 10);
                outhudbuffer[cpos + 12] = ' ';
                outhudbuffer[cpos + 13] = '7';
                outhudbuffer[cpos + 14] = '\\';
                memcpy(outhudbuffer + cpos + 15, ctb + 20 + 27 * 1, 10);
                outhudbuffer[cpos + 25] = ' ';
                outhudbuffer[cpos + 26] = '8';
                outhudbuffer[cpos + 27] = '\\';
                memcpy(outhudbuffer + cpos + 28, ctb + 20 + 27 * 2, 10);
                outhudbuffer[cpos + 38] = ' ';
                outhudbuffer[cpos + 39] = '9';
                outhudbuffer[cpos + 40] = '\\';
                memcpy(outhudbuffer + cpos + 41, ctb + 20 + 27 * 3, 10);
                outhudbuffer[cpos + 51] = 0;
                brtl_strupr((char *) outhudbuffer);
            }
        }
    }

    wrouthud(2, 2, 0, (char *) outhudbuffer);
    pp_delta = (pp_gravity - tp_gravity) * 0.25;
    tp_gravity += pp_delta;
    pp_delta = (pp_temp - tp_temp) * 0.05;
    tp_temp += pp_delta;
    pp_delta = (pp_pressure - tp_pressure) * 0.02;
    tp_pressure += pp_delta;
    pp_delta = (pp_pulse - tp_pulse) * 0.01;
    tp_pulse += pp_delta;
    // unit� di debugging dell'albedo:
    // sprintf (outhudbuffer, "GRAVITY %2.3f FG & TEMPERATURE %+3.1f@C &
    // PRESSURE %2.3f ATM & PULSE %3.0f PPS", tp_gravity, tp_temp, tp_pressure,
    // (float)albedo);
    sprintf((char *) outhudbuffer,
            "GRAVITY %2.3f FG & TEMPERATURE %+3.1f@C & PRESSURE %2.3f ATM & PULSE "
            "%3.0f PPS",
            tp_gravity, tp_temp, tp_pressure, tp_pulse);
    wrouthud(2, adapted_height - 8, 0, (char *) outhudbuffer);
}

/* REMOVED BY JORIS 2023-07-28
extern int32_t star_label_pos;
extern int8_t star_label[25];
extern int32_t planet_label_pos;
extern int8_t planet_label[25];
*/

int8_t snapfilename[24];
/* Save a photo of the screen into the file "SNAP[XXXX].BMP", where [XXXX] is a
 * progressive disambiguation number.
 */
void snapshot(int16_t forcenumber, int8_t showdata) {
#ifndef WITH_GODOT
    int16_t prog;
    uint16_t pqw;
    double parsis_x, parsis_y, parsis_z;
    uint16_t ptr, c;
    int8_t a, b, t[54];
    FILE *ih = sa_open(header_bmp);

    if (ih == nullptr) {
        return;
    }

    fread(t, 1, 54, ih);
    fclose(ih);

    if (!forcenumber) {
        prog = -1;

        do {
            prog++;

            if (prog == 9999) {
                return;
            }

            sprintf((char *) snapfilename, "gallery/SNAP%04d.BMP", prog);
            ih = fopen((char *) snapfilename, "rb");

            if (ih != nullptr) {
                fclose(ih);
            }
        } while (ih != nullptr);
    } else {
        sprintf((char *) snapfilename, "gallery/SNAP%04d.BMP", forcenumber);
    }

    if (showdata) {
        area_clear(adapted, 2, 191, 0, 0, 316, 7, 64 + 63);

        parsis_x = round(dzat_x);
        parsis_y = round(dzat_y);
        parsis_z = round(dzat_z);

        strcpy((char *) outhudbuffer, "LOCATION PARSIS: ");
        strcat((char *) outhudbuffer, alphavalue(parsis_x));
        strcat((char *) outhudbuffer, ";");
        strcat((char *) outhudbuffer, alphavalue(-parsis_y));
        strcat((char *) outhudbuffer, ";");
        strcat((char *) outhudbuffer, alphavalue(parsis_z));

        if (ip_targetted > -1) {
            if (nearstar_p_owner[ip_targetted] > -1) {
                strcat((char *) outhudbuffer, " & TGT: MOON N@");
                strcat((char *) outhudbuffer, alphavalue(nearstar_p_moonid[ip_targetted] + 1));
                strcat((char *) outhudbuffer, " OF PLANET N@");
                strcat((char *) outhudbuffer, alphavalue(nearstar_p_owner[ip_targetted] + 1));
            } else {
                strcat((char *) outhudbuffer, " & TGT: PLANET N@");
                strcat((char *) outhudbuffer, alphavalue(ip_targetted + 1));
            }
        }

        wrouthud(3, 192, 0, (char *) outhudbuffer);

        if (ap_targetted == 1 && star_label_pos != -1) {
            area_clear(adapted, 14, 14, 0, 0, 102, 7, 64 + 63);
            wrouthud(15, 15, 20, (char *) star_label);
        }

        if (ip_targetted != -1 && planet_label_pos != -1) {
            area_clear(adapted, 14, 23, 0, 0, 102, 7, 64 + 63);
            wrouthud(15, 24, 20, (char *) planet_label);
        }
    }

    ih = fopen((char *) snapfilename, "wb+");

    if (ih != nullptr) {
        a = 0;
        fwrite(t, 1, 54, ih);

        for (c = 0; c < 768; c += 3) {
            b = tmppal[c + 2] * 4;
            fwrite(&b, 1, 1, ih);
            b = tmppal[c + 1] * 4;
            fwrite(&b, 1, 1, ih);
            b = tmppal[c + 0] * 4;
            fwrite(&b, 1, 1, ih);
            fwrite(&a, 1, 1, ih);
        }

        for (ptr = 63680; ptr < 64000; ptr -= 320) {
            fwrite(adapted + ptr, 1, 320, ih);
        }

        fclose(ih);
    }
#endif
}

/*
    Consumi supplementari di litio, dal pi� dispendioso al pi� economico:
    - orbita vimana:            1 KD ogni 7 secondi.
    - inseguimento a punto lontano:     1 KD ogni 18 secondi.
    - inseguimento a punto fisso:       1 KD ogni 29 secondi.
    - inseguimento a punto vicino:      1 KD ogni 33 secondi.
    - amplificatore di campo stellare:  1 KD ogni 41 secondi.
    - orbita geosincrona:           1 KD ogni 58 secondi.
    - lampada interna:          1 KD ogni 84 secondi.
    - cercapianeti:             1 KD ogni 155 secondi.
*/

int32_t iqsecs = 0;

void cupola(float y_or, float brk) {
    float xx, yy, zz;
    float lat, lon, dlat, dlon, dlon_2, k, clon, slon, ck, sk;
    dlat   = M_PI / 20;
    dlon   = M_PI / 10;
    dlon_2 = dlon / 2;

    for (lon = 0; lon < 2 * M_PI - dlon_2; lon += dlon) {
        k    = lon + dlon;
        ck   = cos(k);
        sk   = sin(k);
        clon = cos(lon);
        slon = sin(lon);

        for (lat = dlat; lat < brk * dlat; lat += dlat) {
            xx = cupsize * sin(lat + dlat);
            yy = -cupheight * cos(lat) * y_or;
            zz = cupsize * sin(lat);
            stick3d(zz * clon, yy, zz * slon, xx * clon, -cupheight * cos(lat + dlat) * y_or, xx * slon);
            stick3d(zz * clon, yy, zz * slon, zz * ck, yy, zz * sk);
        }

        if (gburst > 1) {
            lat = (M_PI / 20) * 8 * ((float) gburst / 63);
            //lens_flares_for(cam_x, cam_y, cam_z, +cupsize * clon * sin(lat), -cupheight * cos(lat),
            //                +cupsize * slon * sin(lat), -50000, 10, 1, 0, 1, 1);
            //flares = 0;
        }
    }
}

void polycupola(float y_or, int8_t textured) {
    float d1, d2, d3, dd;
    float x[4], y[4], z[4];
    float lat, lon, dlat, dlon, dlon_2, k, clon, slon, ck, sk;
    dlat   = M_PI / 20;
    dlon   = M_PI / 10;
    dlon_2 = dlon / 2;

    for (lon = 0; lon < 2 * M_PI - dlon_2; lon += dlon) {
        k    = lon + dlon;
        ck   = cos(k);
        sk   = sin(k);
        clon = cos(lon);
        slon = sin(lon);

        for (lat = dlat; lat < 8 * dlat; lat += dlat) {
            float xx = cupsize * sin(lat + dlat);
            float yy = -cupheight * cos(lat) * y_or;
            float zz = cupsize * sin(lat);
            x[0]     = zz * clon;
            y[0]     = yy;
            z[0]     = zz * slon;
            x[1]     = zz * ck;
            y[1]     = yy;
            z[1]     = zz * sk;
            x[2]     = xx * ck;
            y[2]     = -cupheight * cos(lat + dlat) * y_or;
            z[2]     = xx * sk;
            x[3]     = xx * clon;
            y[3]     = -cupheight * cos(lat + dlat) * y_or;
            z[3]     = xx * slon;

            if (ontheroof && y_or == 1) {
                d1 = 0.5 * (x[0] + x[1]) - cam_x;
                d2 = 0.5 * (z[0] + z[1]) - cam_z;
                dd = 1000 - sqrt(d1 * d1 + d2 * d2);

                if (dd > 600) {
                    dd = 600;
                }

                if (dd < 0) {
                    dd = 0;
                }

                cam_y += dd;
                poly3d(x, y, z, 4, 64);
                cam_y -= dd;
            } else {
                if (textured) {
                    d1 = 0.5 * (x[0] + x[1]) - cam_x;
                    d2 = 0.5 * (y[0] + y[2]) - cam_y;
                    d3 = 0.5 * (z[0] + z[1]) - cam_z;
                    dd = 500 - sqrt(d1 * d1 + d2 * d2 + d3 * d3);

                    if (dd > 500) {
                        dd = 500;
                    }

                    if (dd < 0) {
                        dd = 0;
                    }

                    cam_y += 4 * dd * y_or;
                    xx   = x[3];
                    yy   = y[3];
                    zz   = z[3];
                    x[3] = x[2];
                    y[3] = y[2];
                    z[3] = z[2];
                    x[2] = x[1];
                    y[2] = y[1];
                    z[2] = z[1];
                    x[1] = x[0];
                    y[1] = y[0];
                    z[1] = z[0];
                    x[0] = xx;
                    y[0] = yy;
                    z[0] = zz;
                    polymap(x, y, z, 4, 0);
                    cam_y -= 4 * dd * y_or;
                } else {
                    poly3d(x, y, z, 4, 64);
                }
            }
        }
    }

    resetfx();
}

/*
    Consumi supplementari di litio, dal pi� dispendioso al pi� economico:
	- orbita vimana:			1 KD ogni 7 secondi.
	- inseguimento a punto lontano: 	1 KD ogni 18 secondi.
	- inseguimento a punto fisso:		1 KD ogni 29 secondi.
	- inseguimento a punto vicino:		1 KD ogni 33 secondi.
	- amplificatore di campo stellare:	1 KD ogni 41 secondi.
	- orbita geosincrona:			1 KD ogni 58 secondi.
	- lampada interna:			1 KD ogni 84 secondi.
	- cercapianeti:				1 KD ogni 155 secondi.
*/

void additional_consumes()
{
	if (iqsecs < (long)secs)
		iqsecs = secs;
	//
	if (ip_targetted > -1 && pwr > 15000) {
		if (ip_reached/* && sync*/) { // We don't track the chase mode within feltyrion-godot at all, so assume a static powerusage instead
			/*if (sync==1) // fixed-point chase
				if (!(iqsecs % 29)) { pwr--; iqsecs++; }
			if (sync==2) // far chase
				if (!(iqsecs % 18)) { pwr--; iqsecs++; }
			if (sync==3) // syncrone orbit
				if (!(iqsecs % 58)) { pwr--; iqsecs++; }
			if (sync==4) // vimana orbit
				if (!(iqsecs %  7)) { pwr--; iqsecs++; }
			if (sync==5) // near chase
				if (!(iqsecs % 33)) { pwr--; iqsecs++; }*/
            if (!(iqsecs % 27)) { pwr--; iqsecs++; } // static powerusage regardless of which sync mode might be used
		}
	}
	//
	if (pl_search		&& !(iqsecs % 155)) { pwr--; iqsecs++; }
	if (ilightv == 1	&& !(iqsecs %  84)) { pwr--; iqsecs++; }
	if (field_amplificator	&& !(iqsecs %  41)) { pwr--; iqsecs++; }
	//
	if (pwr <= 15000) {
		if (charge>0) {
			charge--;
			pwr = 20000;
			status ("FCS: READY", 100);
		} else if (charge<0) { // OMEGA DRIVE - infinite fuel!
			pwr = 20000;
		} else {
			stspeed 	= 0;
			ip_reaching 	= 0;
			ip_reached 	= 1;
			ip_targetted 	= -1;
			if (pwr != 15000) {
				status ("POWER LOSS", 100);
				pwr = 15000;
			}
		}
	}
}
