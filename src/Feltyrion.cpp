#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/sprite3d.hpp"
#include "godot_cpp/classes/mesh_instance3d.hpp"
#include "godot_cpp/classes/mesh.hpp"
#include "godot_cpp/classes/scene_tree.hpp"
#include "godot_cpp/classes/window.hpp"
#include "godot_cpp/core/math.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/mutex.hpp"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/label.hpp"
#include <stdio.h>
#include <map>
#include <functional>
#include <filesystem>

#include "Feltyrion.h"
#include "noctis-d.h"
#include "noctis-0.h"
#include "noctis.h"
#include "brtl.h"

// Used to mark unused parameters to indicate intent and suppress warnings.
#define UNUSED( expr ) (void)( expr )

namespace
{
}

Feltyrion *instance;
extern void loop();
extern void planetary_main();
extern void not_actually_draw_planet(int16_t target_body);
extern void freeze();
extern void unfreeze();
extern void process_comm_bin_file();
extern void iperficie(int16_t additional_quadrants);
extern void prep_iperficie();
extern void getAllFragments();

const double _a = -1.857e-13;
const double _b = 4.362e-10;
const double _c = -3.697e-07;
const double _d = 1.173e-04;

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

godot::Ref<godot::Image> Feltyrion::getSurfacePaletteAsImage() const
{
    auto pba = godot::PackedByteArray();
    for (uint16_t i = 0; i < 256; i++) {
        uint8_t val = surface_palette[i];
        pba.append(surface_palette[i * 3] * 4);
        pba.append(surface_palette[i * 3 + 1] * 4);
        pba.append(surface_palette[i * 3 + 2] * 4);
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

int Feltyrion::getAPTargetted()
{
    return ap_targetted;
}

void Feltyrion::setAPTargettedWithoutExtractingTargetInfos(int i)
{
    // used for setting direct parsis targets - otherwise when extracting ap target infos you'd likely select the Westos star by accident!
    ap_targetted = i;
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

int Feltyrion::getStarnopEstimate(double x, double y, double z)
{
    return starnop(x, y, z);
}

float Feltyrion::getAPStarMass() {
    return get_starmass(ap_target_ray, ap_target_class, ap_target_x);
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
    if (nearstar_p_owner[logical_id] != -1) {
        colorbase = 128;
    }

    surface(logical_id, type, seedval, colorbase, lighting, include_atmosphere);
}

void Feltyrion::loadPlanetAtCurrentSystem(int logical_id)
{
    not_actually_draw_planet(logical_id);
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
 * @param is_moon if true, uses the color palette for moons
 * @return godot::Ref<godot::Image> 
 */
godot::Ref<godot::Image> Feltyrion::returnImage(bool accurate_height, bool raw__one_byte, bool is_moon) const
{
    uint8_t colorbase = 192;
    if (is_moon) {
        colorbase    = 128;
    }

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


void cb_RingParticleFound(double xlight, double ylight, double zlight, double radii, int unconditioned_color, int body_index)
{
    //godot::UtilityFunctions::printt("FOUND RING PARTICLE");
    instance->onRingParticleFound(
        xlight,
        ylight,
        zlight,
        radii,
        unconditioned_color,
        body_index
    );
}

void cb_SurfacePolygon3Found(int8_t what, double x0, double x1, double x2, double y0, double y1, double y2, double z0, double z1, double z2, int colore)
{
    if (what == POLY3D_CAPTURE_SCATTERING) {
        instance->onScatteringPolygon3Found(
            x0,
            x1,
            x2,
            y0,
            y1,
            y2,
            z0,
            z1,
            z2,
            colore
        );
    } else if (what == POLY3D_CAPTURE_SURFACE) {
        instance->onSurfacePolygon3Found(
            x0,
            x1,
            x2,
            y0,
            y1,
            y2,
            z0,
            z1,
            z2,
            colore
        );
    }
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

void Feltyrion::updateStarParticles(double parsis_x, double parsis_y, double parsis_z, double distanceMultiplier, godot::NodePath nodePath)
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
        auto x = Object::cast_to<godot::SpriteBase3D>(v);
        auto vector = godot::Vector3(
            stars_visible[i*3]   * FAR_STAR_PARSIS_SCALING_FACTOR * PARSIS_X_MULTIPLIER * distanceMultiplier, 
            stars_visible[i*3+1] * FAR_STAR_PARSIS_SCALING_FACTOR * PARSIS_Y_MULTIPLIER * distanceMultiplier, 
            stars_visible[i*3+2] * FAR_STAR_PARSIS_SCALING_FACTOR * PARSIS_Z_MULTIPLIER * distanceMultiplier
        );
        if (vector.x == 0 && vector.y == 0 && vector.z == 0) {
            x->hide();
        } else {
            x->show();
            x->set_position(vector);
            auto l = vector.length();
            x->set_pixel_size(_a * godot::Math::pow(l, 3) + _b * godot::Math::pow(l, 2) + _c * l + _d);
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
    int csize = n->get_children().size();
    for (int i = 0; i < nearstar_nob && i < csize; i++) {
        planet_xyz(i);
        godot::Node* node = n->get_child(i);
        godot::Node3D* planet = Object::cast_to<godot::Node3D>(node);
        if (planet != nullptr) {
            planet->set_position(godot::Vector3(
                (nearstar_p_plx[i] - nearstar_x) * PARSIS_X_MULTIPLIER,
                (nearstar_p_ply[i] - nearstar_y) * PARSIS_Y_MULTIPLIER,
                (nearstar_p_plz[i] - nearstar_z) * PARSIS_Z_MULTIPLIER
            ));
        }
    }
}

void Feltyrion::onRingParticleFound(double xlight, double ylight, double zlight, double radii, int unconditioned_color, int body_index) {
    godot::Object::emit_signal(
        "found_ring_particle",
        xlight, ylight, zlight, radii, unconditioned_color, body_index
    );
}

void Feltyrion::onSurfacePolygon3Found(double x0, double x1, double x2, double y0, double y1, double y2, double z0, double z1, double z2, int colore) {
   #ifdef EMIT_POLYGON3_SIGNALS
   godot::Object::emit_signal(
        "found_surface_polygon3",
        x0, x1, x2,
        y0, y1, y2,
        z0, z1, z2,
        colore
   );
   #endif
   Feltyrion::addSurfaceToolPolygon3(x0, x1, x2, y0, y1, y2, z0, z1, z2, colore);
}

void Feltyrion::prepareSurfaceMesh(godot::Node3D* target, godot::String scenePath)
{
    Feltyrion::surfaceMeshNode = target;
    godot::ResourceLoader *reLo = godot::ResourceLoader::get_singleton();
    surfaceMeshScene = reLo->load(scenePath);
    Feltyrion::surfacePaletteTxtr = godot::Ref<godot::ImageTexture>(); // empty ref assignment
    Feltyrion::surfaceAlbedoL8Txtr = godot::Ref<godot::ImageTexture>(); // empty ref assignment
}

void Feltyrion::prepareSurfaceScattering(godot::Node3D* target, godot::String scenePath, bool singleMesh)
{
    Feltyrion::scatteringSingleMesh = singleMesh;
    Feltyrion::surfaceScatteringNode = target;
    godot::ResourceLoader *reLo = godot::ResourceLoader::get_singleton();
    scatteringObjectScene = reLo->load(scenePath);
    Feltyrion::surfacePaletteTxtr = godot::Ref<godot::ImageTexture>(); // empty ref assignment
    Feltyrion::surfaceAlbedoL8Txtr = godot::Ref<godot::ImageTexture>(); // empty ref assignment
}

void cb_ScatteringItemBegin()
{
    instance->onScatteringItemBegin();
}

void cb_ScatteringItemEnd()
{
    instance->onScatteringItemEnd();
}

void cb_ScatteringBegin()
{
    instance->onScatteringBegin();
}

void cb_ScatteringEnd()
{
    instance->onScatteringEnd();
}

void cb_SurfaceBegin()
{
    instance->onSurfaceBegin();
}

void cb_SurfaceEnd()
{
    instance->onSurfaceEnd();
}


void Feltyrion::onScatteringItemBegin() {
    if (Feltyrion::scatteringSingleMesh == false) {
        Feltyrion::surfaceTool.instantiate();
        Feltyrion::surfaceTool->begin(godot::Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
        godot::Color col;
        col.set_r8(255);
        col.set_g8(255);
        col.set_b8(255);
        Feltyrion::surfaceTool->set_color(col);
        Feltyrion::surfaceTool->set_smooth_group(-1);
    }
}

void Feltyrion::onScatteringItemEnd() {
    if (Feltyrion::scatteringSingleMesh == false) {
        Feltyrion::commitSurfaceTool(SURFACETOOL_USECASE_SCATTERING);
    }
}

void Feltyrion::onScatteringBegin() {
    godot::UtilityFunctions::printt("Scattering begin");
    if (Feltyrion::scatteringSingleMesh == true) {
        Feltyrion::instantiateSurfaceTool();
    } else {
        // clear all children now from the scattering node
        if (Feltyrion::surfaceScatteringNode != nullptr && Feltyrion::scatteringObjectScene != nullptr) {
            for (int16_t c = 0; c < Feltyrion::surfaceScatteringNode->get_child_count(); c++) {
                auto child = Feltyrion::surfaceScatteringNode->get_child(c);
                Feltyrion::surfaceScatteringNode->remove_child(child);
                child->queue_free();
            }
        }
    }
}

void Feltyrion::onScatteringEnd() {
    godot::UtilityFunctions::printt("Scattering end");
    if (Feltyrion::scatteringSingleMesh == true) {
        Feltyrion::commitSurfaceTool(SURFACETOOL_USECASE_SCATTERING);
    }
}

void Feltyrion::onSurfaceBegin() {
    godot::UtilityFunctions::printt("Surface begin");
    Feltyrion::instantiateSurfaceTool();
}

void Feltyrion::onSurfaceEnd() {
    godot::UtilityFunctions::printt("Surface end");
    Feltyrion::commitSurfaceTool(SURFACETOOL_USECASE_SURFACEMESH);
}

void Feltyrion::instantiateSurfaceTool() {
    Feltyrion::surfaceTool.instantiate();
    Feltyrion::surfaceTool->begin(godot::Mesh::PrimitiveType::PRIMITIVE_TRIANGLES);
    godot::Color col;
    col.set_r8(255);
    col.set_g8(255);
    col.set_b8(255);
    Feltyrion::surfaceTool->set_color(col);
    Feltyrion::surfaceTool->set_smooth_group(-1);
}

void Feltyrion::commitSurfaceTool(int8_t whichUsecase) {
    Feltyrion::surfaceTool->generate_normals();
    godot::Ref<godot::ArrayMesh> mesh = Feltyrion::surfaceTool->commit();

    if (Feltyrion::surfaceAlbedoL8Txtr == nullptr) {
        godot::UtilityFunctions::printt("Loading surfaceAlbedoL8Txtr");
        Feltyrion::surfaceAlbedoL8Txtr = godot::ImageTexture::create_from_image(Feltyrion::returnTxtrImage(true));
    }
    if (Feltyrion::surfacePaletteTxtr == nullptr) {
        godot::UtilityFunctions::printt("Loading surfacePaletteTxtr");
        Feltyrion::surfacePaletteTxtr = godot::ImageTexture::create_from_image(Feltyrion::getSurfacePaletteAsImage());
    }

    if (whichUsecase == SURFACETOOL_USECASE_SCATTERING) {
        if (Feltyrion::surfaceScatteringNode != nullptr && Feltyrion::scatteringObjectScene != nullptr && Feltyrion::scatteringObjectScene->can_instantiate())
        {
            if (Feltyrion::scatteringSingleMesh == true) {
                for (int16_t c = 0; c < Feltyrion::surfaceScatteringNode->get_child_count(); c++) {
                    auto child = Feltyrion::surfaceScatteringNode->get_child(c);
                    Feltyrion::surfaceScatteringNode->remove_child(child);
                    child->queue_free();
                }
            }
            auto scatteringObject = Feltyrion::scatteringObjectScene->instantiate();
            godot::MeshInstance3D *asMID3d = godot::Object::cast_to<godot::MeshInstance3D>(scatteringObject);
            asMID3d->set_mesh(mesh);
            godot::ShaderMaterial *shaderMaterial = godot::Object::cast_to<godot::ShaderMaterial>(asMID3d->call("get_material_override"));
            shaderMaterial->set_shader_parameter("albedo_texture_format_l8", Feltyrion::surfaceAlbedoL8Txtr);
            shaderMaterial->set_shader_parameter("surface_palette", Feltyrion::surfacePaletteTxtr);
            Feltyrion::surfaceScatteringNode->add_child(scatteringObject);
        }
    } else if (whichUsecase == SURFACETOOL_USECASE_SURFACEMESH) {
        if (Feltyrion::surfaceMeshNode != nullptr && Feltyrion::surfaceMeshScene != nullptr && Feltyrion::surfaceMeshScene->can_instantiate())
        {
            for (int16_t c = 0; c < Feltyrion::surfaceMeshNode->get_child_count(); c++) {
                auto child = Feltyrion::surfaceMeshNode->get_child(c);
                Feltyrion::surfaceMeshNode->remove_child(child);
                child->queue_free();
            }
            auto surfaceObject = Feltyrion::surfaceMeshScene->instantiate();
            godot::MeshInstance3D *asMID3d = godot::Object::cast_to<godot::MeshInstance3D>(surfaceObject);
            asMID3d->set_mesh(mesh);
            asMID3d->create_trimesh_collision();
            godot::ShaderMaterial *shaderMaterial = godot::Object::cast_to<godot::ShaderMaterial>(asMID3d->call("get_material_override"));
            shaderMaterial->set_shader_parameter("albedo_texture_format_l8", Feltyrion::surfaceAlbedoL8Txtr);
            shaderMaterial->set_shader_parameter("surface_palette", Feltyrion::surfacePaletteTxtr);
            Feltyrion::surfaceMeshNode->add_child(surfaceObject);
        }
    }
}

void Feltyrion::onScatteringPolygon3Found(double x0, double x1, double x2, double y0, double y1, double y2, double z0, double z1, double z2, int colore) {
   #ifdef EMIT_POLYGON3_SIGNALS
   godot::Object::emit_signal(
        "found_scattering_polygon3",
        x0, x1, x2,
        y0, y1, y2,
        z0, z1, z2,
        colore
   );
   #endif
   Feltyrion::addSurfaceToolPolygon3(x0, x1, x2, y0, y1, y2, z0, z1, z2, colore);
}

void Feltyrion::addSurfaceToolPolygon3(double x0, double x1, double x2, double y0, double y1, double y2, double z0, double z1, double z2, int colore) {
   godot::Color col;
   col.set_r8(colore);
   col.set_g8(0);
   col.set_b8(0);

   Feltyrion::surfaceTool->set_color(col);
   Feltyrion::surfaceTool->set_uv(godot::Vector2(0, 0));
   Feltyrion::surfaceTool->add_vertex(godot::Vector3((x0 - 1638400) * -0.002, (-y0 - 0) * 0.002, (z0 - 1638400) * 0.002));

   Feltyrion::surfaceTool->set_color(col);
   Feltyrion::surfaceTool->set_uv(godot::Vector2(1, 0));
   Feltyrion::surfaceTool->add_vertex(godot::Vector3((x1 - 1638400) * -0.002, (-y1 - 0) * 0.002, (z1 - 1638400) * 0.002));

   Feltyrion::surfaceTool->set_color(col);
   Feltyrion::surfaceTool->set_uv(godot::Vector2(0, 1));
   Feltyrion::surfaceTool->add_vertex(godot::Vector3((x2 - 1638400) * -0.002, (-y2 - 0) * 0.002, (z2 - 1638400) * 0.002));
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
        brtl_srand((uint16_t) id);
        strcpy((char *) _star_label, "UNKNOWN STAR / CLASS ");
        sprintf((char *) (_star_label + 21), "S%02d", brtl_random(star_classes));
        return (char*)_star_label;
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
        int16_t n = 0;
        while (n < nearstar_nob) {
            if (nearstar_p_identity[n] == planet_id) {
                // found it!
                if (nearstar_p_owner[n] == -1) {
                    strcpy((char *) _star_label, "NAMELESS PLANET / N. ...");
                } else {
                    memcpy(_star_label, "NAMELESS MOON #../../...", 24);
                    sprintf((char *) (_star_label + 15), "%02d", nearstar_p_moonid[n] + 1);
                    sprintf((char *) (_star_label + 18), "%02d", nearstar_p_owner[n] + 1);
                    _star_label[17] = '/';
                    _star_label[20] = '&';
                }
                sprintf((char *) (_star_label + 21), "P%02d", n + 1);
                return (char*)_star_label;
            }
            n++;
        }
        return "";
    }
}

godot::String Feltyrion::getFCSStatus()
{
    return (char*)fcs_status;
}

void Feltyrion::setFCSStatus(godot::String status)
{
    strcpy((char *)fcs_status, status.ascii().get_data());
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
    if (ap_targetted == 1 && ap_reached == 1) {
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
    }
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

double Feltyrion::getPosX() {
    return pos_x;
}

double Feltyrion::getPosY() {
    return pos_y;
}

double Feltyrion::getPosZ() {
    return pos_z;
}

double Feltyrion::getUserAlfa() {
    return user_alfa;
}

double Feltyrion::getUserBeta() {
    return user_beta;
}

void Feltyrion::updateTime()
{
    ::getsecs();
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
    prep_iperficie();
    planetary_main();
}

void Feltyrion::freeze() {
    ::freeze();
}

void Feltyrion::unfreeze() {
    ::unfreeze();
}

void Feltyrion::generateSurfacePolygons() {
    prep_iperficie();
    getAllFragments();
}

void Feltyrion::processCommBinFile() {
    process_comm_bin_file();
}

godot::String Feltyrion::getCWD() const
{
    return std::filesystem::current_path().string().c_str();
}

void Feltyrion::additionalConsumes() {
    additional_consumes();
}

godot::Ref<godot::Image> Feltyrion::returnSkyImage() {
    auto pba = godot::PackedByteArray();
    for (uint16_t i = 0; i < (120 * 360); i++) {
        uint8_t val = s_background[i] + 64; // don't ask me why the +64 here.... I couldn't find any indication for this in the background() function... But +64 fixes the colour here!
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
    for (uint16_t i = 0; i < (200 * 200); i++) {
        uint8_t val = p_surfacemap[i];
        pba.append(val);
    }
    auto image = godot::Image::create_from_data(200, 200, false, godot::Image::FORMAT_L8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

godot::Ref<godot::Image> Feltyrion::returnRuinschartImage() {
    auto pba = godot::PackedByteArray();
    for (uint16_t i = 0; i < (200 * 200); i++) {
        uint8_t val = ruinschart[i];
        pba.append(val);
    }
    auto image = godot::Image::create_from_data(200, 200, false, godot::Image::FORMAT_L8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

/**
 * @brief Get an image (RGBA8/FORMAT_L8) for the planet surface texture.
 * 
 * @param raw__one_byte get the image as a one-byte-per-pixel image (FORMAT_L8), without colormap applied
 * @return godot::Ref<godot::Image> 
 */
godot::Ref<godot::Image> Feltyrion::returnTxtrImage(bool raw__one_byte) {
    auto pba = godot::PackedByteArray();
    int a = 0;
    for (uint16_t y = 0; y < 256; y++) {
        for (uint16_t x = 0; x < 256; x++) {
            uint8_t val = p_background[(y*256)+x];
            if (raw__one_byte) {
                pba.append(val);
            } else {
                pba.append(surface_palette[(val) * 3] * 4);
                pba.append(surface_palette[(val) * 3 + 1] * 4);
                pba.append(surface_palette[(val) * 3 + 2] * 4);
            }
        }
    }
    auto image = godot::Image::create_from_data(256, 256, false, raw__one_byte ? godot::Image::FORMAT_L8 : godot::Image::FORMAT_RGB8, pba);
    godot::Ref<godot::Image> ref = image;
    return ref;
}

DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ip_reached, getIPReached, setIPReached); // 1 if we're orbiting a planet
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ip_reaching, getIPReaching, setIPReaching); // 1 if we're approaching a local target
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, nsync, getNSync, setNSync); // drive tracking mode (i.e. how the stardrifter orbits around a planet)
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, stspeed, getSTSpeed, setSTSpeed); // 1 if we're in Vimana flight
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ap_reached, getAPReached, setAPReached); // 1 if we're in a solar system
DEFINE_NOCTIS_VARIABLE_ACCESSORS(float, float, charge, getLithiumCharge, setLithiumCharge);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, pwr, getPwr, setPwr);
DEFINE_NOCTIS_VARIABLE_ACCESSORS_WITH_SIGNAL(int8_t, int, ilightv, getIlightv, setIlightv); // 1 if internal light is on

DEFINE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, landing_pt_lat, getLandingPtLat, setLandingPtLat);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, landing_pt_lon, getLandingPtLon, setLandingPtLon);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, landing_point, getLandingPoint, setLandingPoint);

DEFINE_NOCTIS_VARIABLE_ACCESSORS(float, float, planet_grav, getPlanetGravity, setPlanetGravity);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(float, float, pp_pressure, getPlanetPressure, setPlanetPressure);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(float, float, pp_temp, getPlanetTemperature, setPlanetTemperature);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, nightzone, getNightzone, setNightzone);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(uint8_t, int, sky_brightness, getSkyBrightness, setSkyBrightness);
DEFINE_NOCTIS_VARIABLE_ACCESSORS(float, float, rainy, getRainy, setRainy);

void Feltyrion::_bind_methods()
{
    // Methods.
    godot::ClassDB::bind_method( godot::D_METHOD( "prepare_star" ), &Feltyrion::prepareStar );
    godot::ClassDB::bind_method( godot::D_METHOD( "load_planet" ), &Feltyrion::loadPlanet );
    godot::ClassDB::bind_method( godot::D_METHOD( "load_planet_at_current_system" ), &Feltyrion::loadPlanetAtCurrentSystem );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_image" ), &Feltyrion::returnImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_palette_as_image" ), &Feltyrion::getPaletteAsImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_surface_palette_as_image" ), &Feltyrion::getSurfacePaletteAsImage );
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

    godot::ClassDB::bind_method( godot::D_METHOD( "get_pos_x" ), &Feltyrion::getPosX );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_pos_y" ), &Feltyrion::getPosY );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_pos_z" ), &Feltyrion::getPosZ );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_user_alfa" ), &Feltyrion::getUserAlfa );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_user_beta" ), &Feltyrion::getUserBeta );

    godot::ClassDB::bind_method( godot::D_METHOD( "lock" ), &Feltyrion::lock );
    godot::ClassDB::bind_method( godot::D_METHOD( "unlock" ), &Feltyrion::unlock );

    godot::ClassDB::bind_method( godot::D_METHOD( "update_star_particles", "parsis_x", "parsis_y", "parsis_z", "distanceMultiplier", "nodePath"), &Feltyrion::updateStarParticles );
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

    godot::ClassDB::bind_method( godot::D_METHOD( "get_starnop_estimate", "star_x", "star_y", "star_z" ), &Feltyrion::getStarnopEstimate );
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ap_starmass" ), &Feltyrion::getAPStarMass );

    godot::ClassDB::bind_method( godot::D_METHOD( "set_ap_targetted" ), &Feltyrion::setAPTargetted );
    godot::ClassDB::bind_method( godot::D_METHOD( "set_ap_targetted_without_extracting_target_infos" ), &Feltyrion::setAPTargettedWithoutExtractingTargetInfos );

    godot::ClassDB::bind_method( godot::D_METHOD( "loop_iter" ), &Feltyrion::loopOneIter );

    godot::ClassDB::bind_method( godot::D_METHOD( "get_fcs_status" ), &Feltyrion::getFCSStatus );
    godot::ClassDB::bind_method( godot::D_METHOD( "set_fcs_status" ), &Feltyrion::setFCSStatus );

    godot::ClassDB::bind_method( godot::D_METHOD( "prepare_planet_surface" ), &Feltyrion::preparePlanetSurface );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_sky_image" ), &Feltyrion::returnSkyImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_surfacemap_image" ), &Feltyrion::returnSurfacemapImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_ruinschart_image" ), &Feltyrion::returnRuinschartImage );
    godot::ClassDB::bind_method( godot::D_METHOD( "return_txtr_image" ), &Feltyrion::returnTxtrImage );

    godot::ClassDB::bind_method( godot::D_METHOD( "freeze" ), &Feltyrion::freeze );
    godot::ClassDB::bind_method( godot::D_METHOD( "unfreeze" ), &Feltyrion::unfreeze );
    godot::ClassDB::bind_method( godot::D_METHOD( "processCommBinFile" ), &Feltyrion::processCommBinFile );

    godot::ClassDB::bind_method( godot::D_METHOD( "generateSurfacePolygons" ), &Feltyrion::generateSurfacePolygons );

    godot::ClassDB::bind_method( godot::D_METHOD( "get_cwd" ), &Feltyrion::getCWD );

    godot::ClassDB::bind_method( godot::D_METHOD( "additional_consumes" ), &Feltyrion::additionalConsumes );

    godot::ClassDB::bind_method( godot::D_METHOD( "prepare_surface_scattering", "target", "scenePath", "singleMesh" ), &Feltyrion::prepareSurfaceScattering);
    godot::ClassDB::bind_method( godot::D_METHOD( "prepare_surface_mesh", "target", "scenePath" ), &Feltyrion::prepareSurfaceMesh);

    // Properties
    godot::ClassDB::bind_method( godot::D_METHOD( "get_ap_targetted"), &Feltyrion::getAPTargetted); // set_ap_targetted defined above as method
    ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "ap_targetted"), "set_ap_targetted", "get_ap_targetted");
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
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::FLOAT, charge, getLithiumCharge, setLithiumCharge);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, pwr, getPwr, setPwr);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, landing_pt_lat, getLandingPtLat, setLandingPtLat);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, landing_pt_lon, getLandingPtLon, setLandingPtLon);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, landing_point, getLandingPoint, setLandingPoint);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::FLOAT, planet_grav, getPlanetGravity, setPlanetGravity);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::FLOAT, pp_pressure, getPlanetPressure, setPlanetPressure);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::FLOAT, pp_temp, getPlanetTemperature, setPlanetTemperature);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, nightzone, getNightzone, setNightzone);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, sky_brightness, getSkyBrightness, setSkyBrightness);
    EXPOSE_NOCTIS_VARIABLE(godot::Variant::FLOAT, rainy, getRainy, setRainy);

    EXPOSE_NOCTIS_VARIABLE(godot::Variant::INT, ilightv, getIlightv, setIlightv);

    // Signals
    DEFINE_NOCTIS_VARIABLE_SIGNAL(godot::Variant::INT, ilightv);
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
    ADD_SIGNAL( godot::MethodInfo( "found_ring_particle", 
        godot::PropertyInfo( godot::Variant::FLOAT, "xlight" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "ylight" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "zlight" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "radii" ), 
        godot::PropertyInfo( godot::Variant::INT, "unconditioned_color" ) ) );

    #ifdef EMIT_POLYGON3_SIGNALS
    // deprecated signals that are no longer needed - only compiled in when EMIT_POLYGON3_SIGNALS is defined
    ADD_SIGNAL( godot::MethodInfo( "found_surface_polygon3", 
        godot::PropertyInfo( godot::Variant::FLOAT, "x0" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "x1" ), 
        godot::PropertyInfo(godot::Variant::FLOAT, "x2"),
        godot::PropertyInfo(godot::Variant::FLOAT, "y0"),
        godot::PropertyInfo(godot::Variant::FLOAT, "y1"),
        godot::PropertyInfo(godot::Variant::FLOAT, "y2"),
        godot::PropertyInfo(godot::Variant::FLOAT, "z0"),
        godot::PropertyInfo(godot::Variant::FLOAT, "z1"),
        godot::PropertyInfo(godot::Variant::FLOAT, "z2"),
        godot::PropertyInfo(godot::Variant::INT, "color")
    ));
    ADD_SIGNAL( godot::MethodInfo( "found_scattering_polygon3", 
        godot::PropertyInfo( godot::Variant::FLOAT, "x0" ), 
        godot::PropertyInfo( godot::Variant::FLOAT, "x1" ), 
        godot::PropertyInfo(godot::Variant::FLOAT, "x2"),
        godot::PropertyInfo(godot::Variant::FLOAT, "y0"),
        godot::PropertyInfo(godot::Variant::FLOAT, "y1"),
        godot::PropertyInfo(godot::Variant::FLOAT, "y2"),
        godot::PropertyInfo(godot::Variant::FLOAT, "z0"),
        godot::PropertyInfo(godot::Variant::FLOAT, "z1"),
        godot::PropertyInfo(godot::Variant::FLOAT, "z2"),
        godot::PropertyInfo(godot::Variant::INT, "color")
    ));
    #endif

}
