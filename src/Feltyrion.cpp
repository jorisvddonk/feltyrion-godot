#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/label.hpp"
#include <stdio.h>
#include <map>
#include <functional>

#include "Feltyrion.h"
#include "noctis-d.h"
#include "noctis-0.h"
#include "noctis.h"

// Used to mark unused parameters to indicate intent and suppress warnings.
#define UNUSED( expr ) (void)( expr )

namespace
{
}

Feltyrion *instance;
extern void loop();
extern void planetary_main();

godot::Ref<godot::Image> Feltyrion::getPaletteAsImage() const
{
    auto pba = godot::PackedByteArray();
    for (uint16_t i = 0; i < 256; i++) {
        uint8_t val = currpal[i];
        pba.append(currpal[i * 3] * 4);
        pba.append(currpal[i * 3 + 1] * 4);
        pba.append(currpal[i * 3 + 2] * 4);
    }
    auto image = godot::Image::create_from_data(256, 1, false, godot::Image::FORMAT_RGB8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

/**
 * @brief Get an image (RGBA8) for the atmosphere.
 * 
 * @param accurate_height get the 'Noctis IV' accurate height, instead of the ENTIRE texture. Noctis clips the entire texture to only 119 lines of latitude.
 * @return godot::Ref<godot::Image> 
 */
godot::Ref<godot::Image> Feltyrion::returnAtmosphereImage(bool accurate_height) const
{
    uint8_t *overlay = (uint8_t *) objectschart;
    uint8_t colorbase = 192 + 32; // + 32 because in Noctis the planet base colour background is added
    auto pba = godot::PackedByteArray();
    int a = 0;
    int vlines = 180;
    if (accurate_height) {
        vlines = 119;
    }
    for (uint16_t i = (accurate_height ? 360 : 0); i < (180 * (vlines + (accurate_height ? 2 : 0))); i++) {
        uint8_t val = overlay[i];
        for (a = 0; a <= 1; a++) {
            // NOTE: in Noctis this is actually ADDED to the base planet background image!
            pba.append(currpal[(colorbase+val) * 3] * 4);
            pba.append(currpal[(colorbase+val) * 3 + 1] * 4);
            pba.append(currpal[(colorbase+val) * 3 + 2] * 4);
            if (val > 0) {
                pba.append((uint8_t) ((val * 255) / 32));
            } else {
                pba.append(0);
            }
        }
    }
    auto image = godot::Image::create_from_data(360, vlines, false, godot::Image::FORMAT_RGBA8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

void cb_Planet(int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t viewpoint, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist)
{
    instance->onPlanetFound(
        index,
        planet_id,
        seedval,
        x * PARSIS_X_MULTIPLIER,
        y * PARSIS_Y_MULTIPLIER,
        z * PARSIS_Z_MULTIPLIER,
        type,
        owner,
        moonid,
        ring,
        tilt,
        ray,
        orb_ray,
        orb_tilt,
        orb_orient,
        orb_ecc,
        rtperiod,
        rotation,
        viewpoint,
        term_start,
        term_end,
        qsortindex,
        qsortdist
    );
}

void Feltyrion::setAPTargetted(int i)
{
    ap_targetted = i;
    extract_ap_target_infos();
}

void Feltyrion::setAPTargetX(double x)
{
    ap_target_x = x * PARSIS_X_MULTIPLIER;
}
void Feltyrion::setAPTargetY(double y)
{
    ap_target_y = y * PARSIS_Y_MULTIPLIER;
}
void Feltyrion::setAPTargetZ(double z)
{
    ap_target_z = z * PARSIS_Z_MULTIPLIER;
}

double Feltyrion::getAPTargetX()
{
    return ap_target_x * PARSIS_X_MULTIPLIER;
}
double Feltyrion::getAPTargetY()
{
    return ap_target_y * PARSIS_Y_MULTIPLIER;
}
double Feltyrion::getAPTargetZ()
{
    return ap_target_z * PARSIS_Z_MULTIPLIER;
}

void Feltyrion::setDzatX(double x)
{
    dzat_x = x * PARSIS_X_MULTIPLIER;
}
void Feltyrion::setDzatY(double y)
{
    dzat_y = y * PARSIS_Y_MULTIPLIER;
}
void Feltyrion::setDzatZ(double z)
{
    dzat_z = z * PARSIS_Z_MULTIPLIER;
}

double Feltyrion::getDzatX()
{
    return dzat_x * PARSIS_X_MULTIPLIER;
}
double Feltyrion::getDzatY()
{
    return dzat_y * PARSIS_Y_MULTIPLIER;
}
double Feltyrion::getDzatZ()
{
    return dzat_z * PARSIS_Z_MULTIPLIER;
}

void Feltyrion::setIPTargetted(int new_target)
{
    ip_targetted = new_target;
}

int8_t Feltyrion::getIPTargetted()
{
    return ip_targetted;
}
double Feltyrion::getIPTargettedX()
{
    return nearstar_p_plx[ip_targetted] * PARSIS_X_MULTIPLIER;
}
double Feltyrion::getIPTargettedY()
{
    return nearstar_p_ply[ip_targetted] * PARSIS_Y_MULTIPLIER;
}
double Feltyrion::getIPTargettedZ()
{
    return nearstar_p_plz[ip_targetted] * PARSIS_Z_MULTIPLIER;
}

double Feltyrion::getNearstarX()
{
    return nearstar_x * PARSIS_X_MULTIPLIER;
}
double Feltyrion::getNearstarY()
{
    return nearstar_y * PARSIS_Y_MULTIPLIER;
}
double Feltyrion::getNearstarZ()
{
    return nearstar_z * PARSIS_Z_MULTIPLIER;
}

void Feltyrion::prepareStar()
{
    // NOTE: this is NOT thread safe!
    instance = this;
    // NOTE: expect init() and extract_ap_target_infos() to have been called before!
    prepare_nearstar(cb_Planet);
}

void Feltyrion::loadPlanet(int logical_id, int type, double seedval, bool lighting, bool include_atmosphere) const
{
    uint8_t colorbase = 192;

    surface(logical_id, type, seedval, colorbase, lighting, include_atmosphere);
}

void Feltyrion::lock()
{
    godot::UtilityFunctions::print( "Locking mutex...." );
    Feltyrion::mutex.lock();
    godot::UtilityFunctions::print( "Locked mutex!" );
}

void Feltyrion::unlock()
{
    godot::UtilityFunctions::print( "Unlocking mutex...." );
    Feltyrion::mutex.unlock();
    godot::UtilityFunctions::print( "Unlocked mutex!" );
}

Feltyrion::Feltyrion()
{
    init();
    godot::UtilityFunctions::print( "Constructor." );
}


/**
 * @brief Get an image (RGBA8) for the planet texture.
 * 
 * @param accurate_height get the 'Noctis IV' accurate height, instead of the ENTIRE texture. Noctis clips the entire texture to only 119 lines of latitude.
 * @param raw__one_byte get the image as a one-byte-per-pixel image (FORMAT_L8), without colormap applied
 * @return godot::Ref<godot::Image> 
 */
godot::Ref<godot::Image> Feltyrion::returnImage(bool accurate_height, bool raw__one_byte) const
{
    uint8_t colorbase = 192;

    int vlines = 180;
    if (accurate_height) {
        vlines = 119;
    }
    auto pba = godot::PackedByteArray();
    for (uint16_t y = (accurate_height ? 1 : 0); y < vlines + (accurate_height ? 1 : 0); y++) {
        for (uint16_t x = 0; x < 360; x++) {
            uint8_t val = p_background[(y*360)+x];
            if (raw__one_byte) {
                pba.append(val);
            } else {
                pba.append(currpal[(colorbase+val) * 3] * 4);
                pba.append(currpal[(colorbase+val) * 3 + 1] * 4);
                pba.append(currpal[(colorbase+val) * 3 + 2] * 4);
            }
        }
    }
    auto image = godot::Image::create_from_data(360, vlines, false, raw__one_byte ? godot::Image::FORMAT_L8 : godot::Image::FORMAT_RGB8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}


void cb_Star(double x, double y, double z, double id_code)
{
    instance->onStarFound(
        x * PARSIS_X_MULTIPLIER,
        y * PARSIS_Y_MULTIPLIER,
        z * PARSIS_Z_MULTIPLIER,
        id_code
    );
}

void Feltyrion::scanStars()
{
    // NOTE: this is *not* thread safe!!!
    instance = this;
    sky(0x405C, true, cb_Star);
}

void Feltyrion::setDzat(double parsis_x, double parsis_y, double parsis_z)
{
    dzat_x = parsis_x * PARSIS_X_MULTIPLIER;
    dzat_y = parsis_y * PARSIS_Y_MULTIPLIER;
    dzat_z = parsis_z * PARSIS_Z_MULTIPLIER;
}

void Feltyrion::setNearstar(double parsis_x, double parsis_y, double parsis_z)
{
    nearstar_x = parsis_x * PARSIS_X_MULTIPLIER;
    nearstar_y = parsis_y * PARSIS_Y_MULTIPLIER;
    nearstar_z = parsis_z * PARSIS_Z_MULTIPLIER;
}

void Feltyrion::updateStarParticles(double parsis_x, double parsis_y, double parsis_z, godot::NodePath nodePath)
{
    // thread safe as callback is not used
    dzat_x = parsis_x * PARSIS_X_MULTIPLIER;
    dzat_y = parsis_y * PARSIS_Y_MULTIPLIER;
    dzat_z = parsis_z * PARSIS_Z_MULTIPLIER;
    sky(0x405C, false, cb_Star);
    auto t = get_tree();
    auto r = t->get_root();
    auto n = r->get_node<godot::Node3D>(nodePath);
    int csize = n->get_children().size();
    for (int i = 0; i < csize; i++) {
        auto v = n->get_child(i);
        auto x = Object::cast_to<godot::Node3D>(v);
        auto vector = godot::Vector3(
            stars_visible[i*3]   * FAR_STAR_PARSIS_SCALING_FACTOR * PARSIS_X_MULTIPLIER, 
            stars_visible[i*3+1] * FAR_STAR_PARSIS_SCALING_FACTOR * PARSIS_Y_MULTIPLIER, 
            stars_visible[i*3+2] * FAR_STAR_PARSIS_SCALING_FACTOR * PARSIS_Z_MULTIPLIER
        );
        if (vector.x == 0 && vector.y == 0 && vector.z == 0) {
            x->hide();
        } else {
            x->show();
            x->set_position(vector);
        }
    }
}

/**
 * Recalculates planet positions and updates their position vectors.
 * Note: time is not updated within this function - you'd have to call update_time() or set_secs() manually prior!
 * 
 * @param nodePath path to the Node3D that's the parent of each Planet scene instance. The planets are expected to be in index order.
 */
void Feltyrion::updateCurrentStarPlanets(godot::NodePath nodePath)
{
    godot::SceneTree* t = get_tree();
    godot::Window* r = t->get_root();
    godot::Node3D* n = r->get_node<godot::Node3D>(nodePath);
    for (int i = 0; i < nearstar_nob; i++) {
        planet_xyz(i);
        godot::Node* node = n->get_child(i);
        godot::Node3D* planet = Object::cast_to<godot::Node3D>(node);
        planet->set_position(godot::Vector3(
            (nearstar_p_plx[i] - nearstar_x) * PARSIS_X_MULTIPLIER,
            (nearstar_p_ply[i] - nearstar_y) * PARSIS_Y_MULTIPLIER,
            (nearstar_p_plz[i] - nearstar_z) * PARSIS_Z_MULTIPLIER
        ));
    }
}

void Feltyrion::onStarFound(double x, double y, double z, double id_code) {
    godot::Object::emit_signal(
        "found_star",
        x * PARSIS_X_MULTIPLIER,
        y * PARSIS_Y_MULTIPLIER,
        z * PARSIS_Z_MULTIPLIER,
        id_code
    );
}

void Feltyrion::onPlanetFound(int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t viewpoint, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist) {
    godot::Object::emit_signal(
        "found_planet",
        index,
        planet_id,
        seedval,
        x * PARSIS_X_MULTIPLIER,
        y * PARSIS_Y_MULTIPLIER,
        z * PARSIS_Z_MULTIPLIER,
        type,
        owner,
        moonid,
        ring,
        tilt,
        ray,
        orb_ray,
        orb_tilt,
        orb_orient,
        orb_ecc,
        rtperiod,
        rotation,
        viewpoint,
        term_start,
        term_end,
        qsortindex,
        qsortdist
    );
}

godot::String Feltyrion::getStarName(double x, double y, double z) const
{
    double id = get_id_code(x * PARSIS_X_MULTIPLIER, y * PARSIS_Y_MULTIPLIER, z * PARSIS_Z_MULTIPLIER);
    int32_t offset = search_id_code(id, 'S');
    if (offset > -1) {
        update_star_label_by_offset(offset);
        return (char*)_star_label;
    } else {
        return "";
    }
}

godot::String Feltyrion::getPlanetName(double star_x, double star_y, double star_z, int index) const
{
    double id = get_id_code(star_x * PARSIS_X_MULTIPLIER, star_y * PARSIS_Y_MULTIPLIER, star_z * PARSIS_Z_MULTIPLIER) + index + 1; // note; a planet's ID code is determined by the in-game body number, which starts at 1 (NOT zero)
    return getPlanetNameById(id);
}

godot::String Feltyrion::getPlanetNameById(double planet_id) const
{
    int32_t offset = search_id_code(planet_id, 'P');
    if (offset > -1) {
        update_star_label_by_offset(offset);
        return (char*)_star_label;
    } else {
        return "";
    }
}

godot::String Feltyrion::getFCSStatus()
{
    return (char*)fcs_status;
}

godot::Dictionary Feltyrion::getPlanetInfo(int n) {
    godot::Dictionary ret = godot::Dictionary();
    ret["n"] = n;
    ret["nearstar_p_seedval"] = nearstar_p_seedval[n];
    ret["nearstar_p_identity"] = nearstar_p_identity[n];
    ret["nearstar_p_plx"] = nearstar_p_plx[n] * PARSIS_X_MULTIPLIER;
    ret["nearstar_p_ply"] = nearstar_p_ply[n] * PARSIS_Y_MULTIPLIER;
    ret["nearstar_p_plz"] = nearstar_p_plz[n] * PARSIS_Z_MULTIPLIER;
    ret["nearstar_p_type"] = nearstar_p_type[n];
    ret["nearstar_p_owner"] = nearstar_p_owner[n];
    ret["nearstar_p_moonid"] = nearstar_p_moonid[n];
    ret["nearstar_p_ring"] = nearstar_p_ring[n];
    ret["nearstar_p_tilt"] = nearstar_p_tilt[n];
    ret["nearstar_p_ray"] = nearstar_p_ray[n];
    ret["nearstar_p_orb_ray"] = nearstar_p_orb_ray[n];
    ret["nearstar_p_orb_tilt"] = nearstar_p_orb_tilt[n];
    ret["nearstar_p_orb_orient"] = nearstar_p_orb_orient[n];
    ret["nearstar_p_orb_ecc"] = nearstar_p_orb_ecc[n];
    ret["nearstar_p_rtperiod"] = nearstar_p_rtperiod[n];
    ret["nearstar_p_rotation"] = nearstar_p_rotation[n];
    ret["nearstar_p_viewpoint"] = nearstar_p_viewpoint[n];
    ret["nearstar_p_term_start"] = nearstar_p_term_start[n];
    ret["nearstar_p_term_end"] = nearstar_p_term_end[n];
    ret["nearstar_p_qsortindex"] = nearstar_p_qsortindex[n];
    ret["nearstar_p_qsortdist"] = nearstar_p_qsortdist[n];
    return ret;
}

godot::Dictionary Feltyrion::getCurrentStarInfo() {
    godot::Dictionary ret = godot::Dictionary();
    ret["nearstar_class"] = nearstar_class;
    ret["nearstar_x"] = nearstar_x * PARSIS_X_MULTIPLIER;
    ret["nearstar_y"] = nearstar_y * PARSIS_Y_MULTIPLIER;
    ret["nearstar_z"] = nearstar_z * PARSIS_Z_MULTIPLIER;
    ret["nearstar_ray"] = nearstar_ray;
    ret["nearstar_spin"] = nearstar_spin;
    ret["nearstar_r"] = nearstar_r;
    ret["nearstar_g"] = nearstar_g;
    ret["nearstar_b"] = nearstar_b;
    ret["nearstar_nop"] = nearstar_nop;
    ret["nearstar_nob"] = nearstar_nob;
    ret["nearstar_labeled"] = nearstar_labeled;
    ret["nearstar_id_code"] = get_id_code(nearstar_x, nearstar_y, nearstar_z);
    return ret;
}

godot::Dictionary Feltyrion::getAPTargetInfo() {
    godot::Dictionary ret = godot::Dictionary();
    ret["ap_targetted"] = ap_targetted;
    if (ap_targetted) {
        ret["ap_target_class"] = ap_target_class;
        ret["ap_target_ray"] = ap_target_ray;
        ret["ap_target_r"] = ap_target_r;
        ret["ap_target_g"] = ap_target_g;
        ret["ap_target_b"] = ap_target_b;
        ret["ap_target_spin"] = ap_target_spin;
        ret["ap_target_x"] = ap_target_x * PARSIS_X_MULTIPLIER;
        ret["ap_target_y"] = ap_target_y * PARSIS_Y_MULTIPLIER;
        ret["ap_target_z"] = ap_target_z * PARSIS_Z_MULTIPLIER;
        ret["ap_target_id_code"] = get_id_code(ap_target_x, ap_target_y, ap_target_z);
    }
    return ret;
}

void Feltyrion::updateTime()
{
    getsecs();
}

void Feltyrion::setSecs(double s)
{
    secs = s;
}

double Feltyrion::getSecs()
{
    return secs;
}

void Feltyrion::loopOneIter()
{
    loop(); // only one iteration here!
}

void Feltyrion::preparePlanetSurface() {
    planetary_main();
}


godot::Ref<godot::Image> Feltyrion::returnSkyImage() {
    auto pba = godot::PackedByteArray();
    int a = 0;
    for (uint16_t i = 0; i < (120 * 360); i++) {
        uint8_t val = s_background[i];
        pba.append(surface_palette[(val) * 3] * 4);
        pba.append(surface_palette[(val) * 3 + 1] * 4);
        pba.append(surface_palette[(val) * 3 + 2] * 4);
    }
    auto image = godot::Image::create_from_data(360, 120, false, godot::Image::FORMAT_RGB8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

godot::Ref<godot::Image> Feltyrion::returnSurfacemapImage() {
    auto pba = godot::PackedByteArray();
    int a = 0;
    for (uint16_t i = 0; i < (200 * 200); i++) {
        uint8_t val = p_surfacemap[i];
        pba.append(val);
    }
    auto image = godot::Image::create_from_data(200, 200, false, godot::Image::FORMAT_L8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

godot::Ref<godot::Image> Feltyrion::returnTxtrImage() {
    auto pba = godot::PackedByteArray();
    int a = 0;
    for (uint16_t y = 0; y < 256; y++) {
        for (uint16_t x = 0; x < 256; x++) {
            uint8_t val = txtr[(y*256)+x];
            pba.append(surface_palette[(val) * 3] * 4);
            pba.append(surface_palette[(val) * 3 + 1] * 4);
            pba.append(surface_palette[(val) * 3 + 2] * 4);
        }
    }
    auto image = godot::Image::create_from_data(256, 256, false, godot::Image::FORMAT_RGB8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ip_reached, getIPReached, setIPReached); // 1 if we're orbiting a planet
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ip_reaching, getIPReaching, setIPReaching); // 1 if we're approaching a local target
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, nsync, getNSync, setNSync); // drive tracking mode (i.e. how the stardrifter orbits around a planet)
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, stspeed, getSTSpeed, setSTSpeed); // 1 if we're in Vimana flight
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ap_reached, getAPReached, setAPReached); // 1 if we're in a solar system

DEFINE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, landing_pt_lat, getLandingPtLat, setLandingPtLat);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, landing_pt_lon, getLandingPtLon, setLandingPtLon);

void Feltyrion::_bind_methods()
{
    // Methods.
    godot::ClassDB::bind_method( godot::D_METHOD( "prepare_star" ), &Feltyrion::prepareStar );
    godot::ClassDB::bind_method( godot::D_METHOD( "load_planet" ), &Feltyrion::loadPlanet );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_image" ), &Feltyrion::returnImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_palette_as_image" ), &Feltyrion::getPaletteAsImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_atmosphere_image" ), &Feltyrion::returnAtmosphereImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "scan_stars" ), &Feltyrion::scanStars );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_star_name" ), &Feltyrion::getStarName );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_planet_name" ), &Feltyrion::getPlanetName );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_planet_name_by_id" ), &Feltyrion::getPlanetNameById );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_current_star_info" ), &Feltyrion::getCurrentStarInfo );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ap_target_info" ), &Feltyrion::getAPTargetInfo );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_planet_info" ), &Feltyrion::getPlanetInfo );
    godot::ClassDB::bind_method( godot::D_METHOD( "set_dzat" ), &Feltyrion::setDzat );
    godot::ClassDB::bind_method( godot::D_METHOD( "set_nearstar" ), &Feltyrion::setNearstar );

    godot::ClassDB::bind_method( godot::D_METHOD( "lock" ), &Feltyrion::lock );
    godot::ClassDB::bind_method( godot::D_METHOD( "unlock" ), &Feltyrion::unlock );

    godot::ClassDB::bind_method( godot::D_METHOD( "update_star_particles" ), &Feltyrion::updateStarParticles );
    godot::ClassDB::bind_method( godot::D_METHOD( "update_current_star_planets" ), &Feltyrion::updateCurrentStarPlanets );

    godot::ClassDB::bind_method( godot::D_METHOD( "set_secs" ), &Feltyrion::setSecs );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_secs" ), &Feltyrion::getSecs );
    godot::ClassDB::bind_method( godot::D_METHOD( "update_time" ), &Feltyrion::updateTime );

    godot::ClassDB::bind_method( godot::D_METHOD( "get_ip_targetted_x" ), &Feltyrion::getIPTargettedX );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ip_targetted_y" ), &Feltyrion::getIPTargettedY );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ip_targetted_z" ), &Feltyrion::getIPTargettedZ );

    godot::ClassDB::bind_method( godot::D_METHOD( "get_nearstar_x" ), &Feltyrion::getNearstarX );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_nearstar_y" ), &Feltyrion::getNearstarY );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_nearstar_z" ), &Feltyrion::getNearstarZ );

    godot::ClassDB::bind_method( godot::D_METHOD( "set_ap_targetted" ), &Feltyrion::setAPTargetted );

    godot::ClassDB::bind_method( godot::D_METHOD( "loop_iter" ), &Feltyrion::loopOneIter );

    godot::ClassDB::bind_method( godot::D_METHOD( "get_fcs_status" ), &Feltyrion::getFCSStatus );

    godot::ClassDB::bind_method( godot::D_METHOD( "prepare_planet_surface" ), &Feltyrion::preparePlanetSurface );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_sky_image" ), &Feltyrion::returnSkyImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_surfacemap_image" ), &Feltyrion::returnSurfacemapImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_txtr_image" ), &Feltyrion::returnTxtrImage );

    // Properties
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ap_target_x"), &Feltyrion::getAPTargetX);
    godot::ClassDB::bind_method( godot::D_METHOD( "set_ap_target_x", "ap_target_x" ), &Feltyrion::setAPTargetX );
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "ap_target_x"), "set_ap_target_x", "get_ap_target_x");
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ap_target_y"), &Feltyrion::getAPTargetY);
    godot::ClassDB::bind_method( godot::D_METHOD( "set_ap_target_y", "ap_target_y" ), &Feltyrion::setAPTargetY );
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "ap_target_y"), "set_ap_target_y", "get_ap_target_y");
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ap_target_z"), &Feltyrion::getAPTargetZ);
    godot::ClassDB::bind_method( godot::D_METHOD( "set_ap_target_z", "ap_target_z" ), &Feltyrion::setAPTargetZ );
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "ap_target_z"), "set_ap_target_z", "get_ap_target_z");

    godot::ClassDB::bind_method( godot::D_METHOD( "get_dzat_x"), &Feltyrion::getDzatX);
    godot::ClassDB::bind_method( godot::D_METHOD( "set_dzat_x", "dzat_x" ), &Feltyrion::setDzatX );
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "dzat_x"), "set_dzat_x", "get_dzat_x");
    godot::ClassDB::bind_method( godot::D_METHOD( "get_dzat_y"), &Feltyrion::getDzatY);
    godot::ClassDB::bind_method( godot::D_METHOD( "set_dzat_y", "dzat_y" ), &Feltyrion::setDzatY );
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "dzat_y"), "set_dzat_y", "get_dzat_y");
    godot::ClassDB::bind_method( godot::D_METHOD( "get_dzat_z"), &Feltyrion::getDzatZ);
    godot::ClassDB::bind_method( godot::D_METHOD( "set_dzat_z", "dzat_z" ), &Feltyrion::setDzatZ );
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "dzat_z"), "set_dzat_z", "get_dzat_z");

    godot::ClassDB::bind_method( godot::D_METHOD( "get_ip_targetted" ), &Feltyrion::getIPTargetted );
    godot::ClassDB::bind_method( godot::D_METHOD( "set_ip_targetted" ), &Feltyrion::setIPTargetted );
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "ip_targetted"), "set_ip_targetted", "get_ip_targetted");

    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, ip_reached, getIPReached, setIPReached);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, ip_reaching, getIPReaching, setIPReaching);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, nsync, getNSync, setNSync);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, stspeed, getSTSpeed, setSTSpeed);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, ap_reached, getAPReached, setAPReached);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, landing_pt_lat, getLandingPtLat, setLandingPtLat);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, landing_pt_lon, getLandingPtLon, setLandingPtLon);

    // Signals
    ADD_SIGNAL( godot::MethodInfo( "found_star", godot::PropertyInfo( godot::Variant::FLOAT, "x" ),  godot::PropertyInfo( godot::Variant::FLOAT, "y" ), godot::PropertyInfo( godot::Variant::FLOAT, "z" ), godot::PropertyInfo( godot::Variant::FLOAT, "id_code" ) ) );
    ADD_SIGNAL( godot::MethodInfo( "found_planet", 
        godot::PropertyInfo( godot::Variant::INT, "index" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "planet_id" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "seedval" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "x" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "y" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "z" ), 
        godot::PropertyInfo( godot::Variant::INT, "type" ),  
        godot::PropertyInfo( godot::Variant::INT, "owner" ), 
        godot::PropertyInfo( godot::Variant::INT, "moonid" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "ring" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "tilt" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "ray" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "orb_ray" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "orb_tilt" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "orb_orient" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "orb_ecc" ),
        godot::PropertyInfo( godot::Variant::INT, "rtperiod" ),
        godot::PropertyInfo( godot::Variant::INT, "rotation" ),
        godot::PropertyInfo( godot::Variant::INT, "viewpoint" ),
        godot::PropertyInfo( godot::Variant::INT, "term_start" ),
        godot::PropertyInfo( godot::Variant::INT, "term_end" ),
        godot::PropertyInfo( godot::Variant::INT, "qsortindex" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "qsortdist" ) ) );
}
