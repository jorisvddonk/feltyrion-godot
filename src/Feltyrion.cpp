#include "godot_cpp/classes/image.hpp"
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

// Used to mark unused parameters to indicate intent and suppress warnings.
#define UNUSED( expr ) (void)( expr )

namespace
{
}

Feltyrion *instance;
extern void init();
extern void extract_ap_target_infos();
extern void prepare_nearstar(void (*onPlanetFound)(int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist));
extern void surface(int16_t logical_id, int16_t type, double seedval, uint8_t colorbase, bool lighting, bool include_atmosphere);
extern void sky(uint16_t limits, void (*callback)(float x, float y, float z, double id_code));
extern void save_models();
extern int16_t nearstar_nob;
extern int32_t search_id_code(double id_code, int8_t type);
extern uint8_t *p_background;
extern uint8_t tmppal[768];
extern uint8_t currpal[768];
extern quadrant *objectschart;
extern void update_star_label_by_offset(int32_t offset);
extern double get_id_code(double x, double y, double z);
extern double _star_id;
extern int8_t _star_label[25];
extern double ap_target_x;
extern double ap_target_y;
extern double ap_target_z;
extern int16_t nearstar_class;
extern int16_t nearstar_nop;
extern int8_t nearstar_spin;
extern int8_t nearstar_r;
extern int8_t nearstar_g;
extern int8_t nearstar_b;
extern double nearstar_x;
extern double nearstar_y;
extern double nearstar_z;
extern float nearstar_ray;
extern int16_t nearstar_labeled;
extern int16_t ap_target_class;
extern int8_t ap_target_spin;
extern int8_t ap_target_r;
extern int8_t ap_target_g;
extern int8_t ap_target_b;
extern float ap_target_ray;
extern int8_t ap_targetted;

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

void cb_Planet(int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist)
{
    instance->onPlanetFound(index, planet_id, seedval, x, y, z, type, owner, moonid, ring, tilt, ray, orb_ray, orb_tilt, orb_orient, orb_ecc, rtperiod, rotation, term_start, term_end, qsortindex, qsortdist);
}

void Feltyrion::setAPTarget(godot::Vector3 ap_target)
{
    ap_target_x = ap_target.x;
    ap_target_y = ap_target.y;
    ap_target_z = ap_target.z;
    ap_targetted = 1;
    extract_ap_target_infos();
}

godot::Vector3 Feltyrion::getAPTarget()
{
    return godot::Vector3(ap_target_x, ap_target_y, ap_target_z);
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
    godot::UtilityFunctions::print( " ReturnImage" );
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


void cb_Star(float x, float y, float z, double id_code)
{
    instance->onStarFound(x, y, z, id_code);
}

void Feltyrion::scanStars()
{
    // NOTE: this is *not* thread safe!!!
    instance = this;
    sky(0x405C, cb_Star);
}

void Feltyrion::onStarFound(float x, float y, float z, double id_code) {
    godot::Object::emit_signal("found_star", x, y, z, id_code);
}

void Feltyrion::onPlanetFound(int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist) {
    godot::Object::emit_signal("found_planet", index, planet_id, seedval, x, y, z, type, owner, moonid, ring, tilt, ray, orb_ray, orb_tilt, orb_orient, orb_ecc, rtperiod, rotation, term_start, term_end, qsortindex, qsortdist);
}

godot::String Feltyrion::getStarName(double x, double y, double z) const
{
    double id = get_id_code(x, y, z);
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
    double id = get_id_code(star_x, star_y, star_z) + index + 1; // note; a planet's ID code is determined by the in-game body number, which starts at 1 (NOT zero)
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

godot::Dictionary Feltyrion::getCurrentStarInfo() {
    godot::Dictionary ret = godot::Dictionary();
    ret["nearstar_class"] = nearstar_class;
    ret["nearstar_x"] = nearstar_x;
    ret["nearstar_y"] = nearstar_y;
    ret["nearstar_z"] = nearstar_z;
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
        ret["ap_target_x"] = ap_target_x;
        ret["ap_target_y"] = ap_target_y;
        ret["ap_target_z"] = ap_target_z;
        ret["ap_target_id_code"] = get_id_code(ap_target_x, ap_target_y, ap_target_z);
    }
    return ret;
}

void Feltyrion::saveModels() const
{
    save_models();
}

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

    godot::ClassDB::bind_method( godot::D_METHOD( "save_models" ), &Feltyrion::saveModels );

    godot::ClassDB::bind_method( godot::D_METHOD( "lock" ), &Feltyrion::lock );
    godot::ClassDB::bind_method( godot::D_METHOD( "unlock" ), &Feltyrion::unlock );

    // Properties
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ap_target"), &Feltyrion::getAPTarget);
    godot::ClassDB::bind_method( godot::D_METHOD( "set_ap_target", "ap_target" ), &Feltyrion::setAPTarget );
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::VECTOR3, "ap_target"), "set_ap_target", "get_ap_target");

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
        godot::PropertyInfo( godot::Variant::INT, "term_start" ),
        godot::PropertyInfo( godot::Variant::INT, "term_end" ),
        godot::PropertyInfo( godot::Variant::INT, "qsortindex" ),
        godot::PropertyInfo( godot::Variant::FLOAT, "qsortdist" ) ) );
}
