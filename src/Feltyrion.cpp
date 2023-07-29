#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/label.hpp"
#include <stdio.h>
#include <map>
#include <functional>

#include "PlanetTexture.h"
#include "noctis-d.h"

// Used to mark unused parameters to indicate intent and suppress warnings.
#define UNUSED( expr ) (void)( expr )

namespace
{
}

extern void init();
extern void prepare_nearstar();
extern void surface(int16_t logical_id, int16_t type, double seedval, uint8_t colorbase, bool lighting, bool include_atmosphere);
extern void sky(uint16_t limits, void (*callback)(float x, float y, float z));
extern uint8_t *p_background;
extern uint8_t tmppal[768];
extern uint8_t currpal[768];
extern quadrant *objectschart;

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

godot::Ref<godot::Image> Feltyrion::returnAtmosphereImage() const
{
    uint8_t *overlay = (uint8_t *) objectschart;
    uint8_t colorbase = 192 + 32; // + 32 because in Noctis the planet base colour background is added
    auto pba = godot::PackedByteArray();
    int a = 0;
    for (uint16_t i = 0; i < 32400; i++) {
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
    auto image = godot::Image::create_from_data(360, 180, false, godot::Image::FORMAT_RGBA8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

void Feltyrion::prepareStar() const
{
    init();
    prepare_nearstar();
}

void Feltyrion::loadPlanet(int logical_id, int type, double seedval, bool lighting, bool include_atmosphere) const
{
    godot::UtilityFunctions::print( "  LoadPlanet",godot::String::num_int64( type ) );
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
    godot::UtilityFunctions::print( "Constructor." );
}


godot::Ref<godot::Image> Feltyrion::returnImage(bool raw__one_bit) const
{
    godot::UtilityFunctions::print( " ReturnImage" );
    uint8_t colorbase = 192;

    auto pba = godot::PackedByteArray();
    for (uint16_t y = 0; y < 180; y++) {
        for (uint16_t x = 0; x < 360; x++) {
            uint8_t val = p_background[(y*360)+x];
            if (raw__one_bit) {
                pba.append(val);
            } else {
                pba.append(currpal[(colorbase+val) * 3] * 4);
                pba.append(currpal[(colorbase+val) * 3 + 1] * 4);
                pba.append(currpal[(colorbase+val) * 3 + 2] * 4);
            }
        }
    }
    auto image = godot::Image::create_from_data(360, 180, false, raw__one_bit ? godot::Image::FORMAT_L8 : godot::Image::FORMAT_RGB8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

Feltyrion *instance;

void cb(float x, float y, float z)
{
    instance->onStarFound(x, y, z);
}

void Feltyrion::scanStars()
{
    // NOTE: this is *not* thread safe!!!
    instance = this;
    sky(0x405C, cb);
}

void Feltyrion::onStarFound(float x, float y, float z) {
    godot::Object::emit_signal("found_star", x, y, z);
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

    godot::ClassDB::bind_method( godot::D_METHOD( "lock" ), &Feltyrion::lock );
    godot::ClassDB::bind_method( godot::D_METHOD( "unlock" ), &Feltyrion::unlock );

    // Signals
    ADD_SIGNAL( godot::MethodInfo( "found_star", godot::PropertyInfo( godot::Variant::FLOAT, "x" ),  godot::PropertyInfo( godot::Variant::FLOAT, "y" ), godot::PropertyInfo( godot::Variant::FLOAT, "z" ) ) );
}
