#include "godot_cpp/variant/utility_functions.hpp"

#pragma once

uint8_t escrescenze = 0xE0; // primo colore dei bumps (escrescenze)
int32_t _x_, _y_;

float EMU_K       = 16;                         // Cost of FPU emulation
int32_t H_MATRIXS = 16;                         // Number of repetitions. 16-128
int32_t V_MATRIXS = 16;                         // Number of repetitions. 16-128
int32_t XSIZE     = TEXTURE_X_SIZE * H_MATRIXS; // Calibrate dimensions.
int32_t YSIZE     = TEXTURE_Y_SIZE * V_MATRIXS; // Calibrate dimensions.

int32_t lbxl = lbx;
int32_t ubxl = ubx;
int32_t lbyl = lby;
int32_t ubyl = uby;

float lbxf = (float) ((int32_t) lbx);
float ubxf = (float) ((int32_t) ubx);
float lbyf = (float) ((int32_t) lby);
float ubyf = (float) ((int32_t) uby);

float x_centro_f = VIEW_X_CENTER;
float y_centro_f = VIEW_Y_CENTER;

float dpp = 200;
float uneg = 100;
float alfa = 0, beta = 0, ngamma = 0;
float cam_x = 0, cam_y = 0, cam_z = 0;

float pnx, pny, pnz; // valori di ritorno della funzione successiva. */

float opt_pcosbeta = dpp;
float opt_psinbeta = 0;
float opt_tcosbeta = 1;
float opt_tsinbeta = 0;
float opt_pcosalfa = dpp;
float opt_psinalfa = 0;
float opt_tcosalfa = 1;
float opt_tsinalfa = 0;

float opt_tcosngamma = 1;
float opt_tsinngamma = 0;

void forward(float delta) {}

bool facing(float *x, float *y, float *z) {
  return true;
}

/* Calculate the coordinates on the screen of a point in space,
 * using the same calculation nucleus(unit? group?) of poly3d and polymap,
 * if the point re-enters the screen it returns 1, otherwise it returns 0.
 * The coordinates are then transferred in the variables _x_ and _y_.
 * The point should not be traced(drawn?) so it is not visible,
 * because the coordinates would be, in this(that?) case, indeterminable. */
int8_t get_coords(float x, float y, float z) {
    float rx, ry, rz, my, xx, yy, zz, z2;

    zz = z - cam_z;
    xx = x - cam_x;

    rx = (xx * opt_tcosbeta) + (zz * opt_tsinbeta);
    z2 = (zz * opt_tcosbeta) - (xx * opt_tsinbeta);

    yy = y - cam_y;

    rz = (yy * opt_tsinalfa) + (z2 * opt_tcosalfa);

    if (ngamma != 0) {
        my = (yy * opt_tcosalfa) - (z2 * opt_tsinalfa);
        ry = (my * opt_tcosngamma) - (rx * opt_tsinngamma);
        rx = (rx * opt_tcosngamma) + (my * opt_tsinngamma);
    } else {
        ry = (yy * opt_tcosalfa) - (z2 * opt_tsinalfa);
    }

    if (rz < uneg) {
        return 0;
    }

    my  = dpp / rz;
    _x_ = round(my * rx + x_centro_f);
    _y_ = round(my * ry + y_centro_f);

    if (_x_ > lbxl && _x_ < ubxl && _y_ > lbyl && _y_ < ubyl) {
        return 1;
    } else {
        return 0;
    }
}


void change_txm_repeating_mode() {}

void change_camera_lens() {}


void change_angle_of_view() {
    opt_pcosbeta   = (float) cos(beta * deg) * dpp;
    opt_psinbeta   = (float) sin(beta * deg) * dpp;
    opt_tcosbeta   = (float) cos(beta * deg);
    opt_tsinbeta   = (float) sin(beta * deg);
    opt_pcosalfa   = (float) cos(alfa * deg) * dpp; // 0.833
    opt_psinalfa   = (float) sin(alfa * deg) * dpp; // 0.833
    opt_tcosalfa   = (float) cos(alfa * deg);
    opt_tsinalfa   = (float) sin(alfa * deg);
    opt_tcosngamma = (float) cos(ngamma * deg);
    opt_tsinngamma = (float) sin(ngamma * deg);
}

void stick3d (float p_x, float p_y, float p_z, float x, float y, float z) {
    /*
   FILE *fp;

   fp = fopen("../temp.txt", "a");
   fprintf(fp, "%f %f %f %f %f %f\n", p_x, p_y, p_z, x, y, z);
   fclose(fp);
   */
}

extern void cb_SurfacePolygon3Found(double x0, double x1, double x2, double y0, double y1, double y2, double z0, double z1, double z2, int colore);
extern bool capture_poly3d;

void poly3d(const float *x, const float *y, const float *z, uint16_t nrv, uint8_t colore) {
    if (!capture_poly3d) {
        return;
    }
    if (nrv == 3) {
        cb_SurfacePolygon3Found(x[0], x[1], x[2], y[0], y[1], y[2], z[0], z[1], z[2], colore);
    } else if (nrv == 4) {
        godot::UtilityFunctions::printt("WARNING: unverified implementation of poly3d(...) for nrv==4");
        cb_SurfacePolygon3Found(x[0], x[1], x[2], y[0], y[1], y[2], z[0], z[1], z[2], colore);
        cb_SurfacePolygon3Found(x[0], x[3], x[2], y[0], y[3], y[2], z[0], z[3], z[2], colore);
    }
   // assuming nrv is always 4...
   /*
   FILE *fp;

   fp = fopen("../temp2.obj", "a");
   fprintf(fp, "v %f %f %f\n", x[0] / 100, y[0] / 100, z[0] / 100);
   fprintf(fp, "v %f %f %f\n", x[1] / 100, y[1] / 100, z[1] / 100);
   fprintf(fp, "v %f %f %f\n", x[2] / 100, y[2] / 100, z[2] / 100);
   fprintf(fp, "v %f %f %f\n", x[3] / 100, y[3] / 100, z[3] / 100);

   fprintf(fp, "f %i %i %i\n", p_tracker + 0, p_tracker + 1, p_tracker + 2);
   fprintf(fp, "f %i %i %i\n", p_tracker + 0, p_tracker + 2, p_tracker + 3);

   p_tracker += 4;
   fclose(fp);
   */
}

void polymap(float *x, float *y, float *z, int8_t nv, uint8_t tinta) {
    if (!capture_poly3d) {
        return;
    }
    if (nv == 3) {
        cb_SurfacePolygon3Found(x[0], x[1], x[2], y[0], y[1], y[2], z[0], z[1], z[2], tinta);
    } else if (nv == 4) {
        cb_SurfacePolygon3Found(x[0], x[1], x[2], y[0], y[1], y[2], z[0], z[1], z[2], tinta);
        cb_SurfacePolygon3Found(x[0], x[3], x[2], y[0], y[3], y[2], z[0], z[3], z[2], tinta);
    }
}

uint8_t *txtr; /* Area della texture (FLS a livelli di intensit�,
                 64 livelli per pixel, senza header).*/

int8_t flares = 0;
int8_t culling_needed  = 0; // flag: traccia due punti per volta.
int8_t halfscan_needed = 0; // flag: traccia due linee per volta.


void pnorm(const float *x, const float *y, const float *z)
// Calcola i coefficenti x/y/z della normale ad un poligono.
// Valori calcolati in pnx, pny, pnz.
// Richiesti almeno tre vertici.
// N.B. I vertici sono scanditi in senso orario,
//      il vettore risultante rispetta la regola della mano destra.
{
    float x1, y1, z1;
    float x2, y2, z2;
    float xr, yr, zr;
    float ln;
    x1 = x[0] - x[2];
    y1 = y[0] - y[2];
    z1 = z[0] - z[2];
    x2 = x[1] - x[2];
    y2 = y[1] - y[2];
    z2 = z[1] - z[2];
    xr = y1 * z2 - y2 * z1;
    yr = z1 * x2 - z2 * x1;
    zr = x1 * y2 - x2 * y1;
    ln = sqrt(xr * xr + yr * yr + zr * zr);

    if (ln > 0) {
        ln  = 1 / ln;
        pnx = xr * ln;
        pny = yr * ln;
        pnz = zr * ln;
    } else {
        pnx = 0;
        pny = 0;
        pnz = 0;
    }
}

uint16_t ptr;