#pragma once

#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/classes/surface_tool.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/image_texture.hpp"
#include "godot_cpp/classes/material.hpp"
#include "godot_cpp/classes/shader_material.hpp"
#include "godot_cpp/core/binder_common.hpp"
#include "godot_cpp/classes/mutex.hpp"

#define FAR_STAR_PARSIS_SCALING_FACTOR 0.001
#define PARSIS_X_MULTIPLIER 1
#define PARSIS_Y_MULTIPLIER 1
#define PARSIS_Z_MULTIPLIER 1

#define SURFACETOOL_USECASE_SURFACEMESH 1
#define SURFACETOOL_USECASE_SCATTERING 2

#define _Q(x) #x
#define _QUOTE(x) _Q(x)
#define _SET_QUOTE(x) _Q(set_ ## x)
#define _GET_QUOTE(x) _Q(get_ ## x)
#define _SIGNAL_NAME_QUOTE(x) _Q(on_ ## x ## _changed)

#define DECLARE_NOCTIS_VARIABLE_ACCESSORS(type, setterType, property, getterName, setterName) type getterName(); void setterName(setterType value)
#define EXPOSE_NOCTIS_VARIABLE(variantType, property, getterName, setterName) \
    godot::ClassDB::bind_method( godot::D_METHOD( _GET_QUOTE(property) ), &Feltyrion::getterName );\
    godot::ClassDB::bind_method( godot::D_METHOD( _SET_QUOTE(property) ), &Feltyrion::setterName );\
    ADD_PROPERTY(godot::PropertyInfo(variantType, _QUOTE(property)), _SET_QUOTE(property), _GET_QUOTE(property));
#define DEFINE_NOCTIS_VARIABLE_ACCESSORS(type, setterType, property, getterName, setterName) \
void Feltyrion::setterName(setterType value)\
{\
    property = value;\
}\
type Feltyrion::getterName()\
{\
    return property;\
}

#define DEFINE_NOCTIS_VARIABLE_ACCESSORS_WITH_SIGNAL(type, setterType, property, getterName, setterName) \
void Feltyrion::setterName(setterType value)\
{\
    property = value;\
    emit_signal(_SIGNAL_NAME_QUOTE(property), value);\
}\
type Feltyrion::getterName()\
{\
    return property;\
}

#define DEFINE_NOCTIS_VARIABLE_SIGNAL(variantType, property) \
ADD_SIGNAL(godot::MethodInfo(_SIGNAL_NAME_QUOTE(property), godot::PropertyInfo(variantType, "newValue")));

class Feltyrion : public godot::Control
{
    GDCLASS( Feltyrion, godot::Control )

public:
    // Functions.
    void prepareStar();
    void loadPlanet(int logical_id, int type, double seedval, bool lighting, bool include_atmosphere) const;
    void loadPlanetAtCurrentSystem(int logical_id);
    godot::Ref<godot::Image> getPaletteAsImage() const;
    godot::Ref<godot::Image> getSurfacePaletteAsImage() const;
    godot::Ref<godot::Image> returnAtmosphereImage(bool accurate_height) const;
    godot::Ref<godot::Image> returnImage(bool accurate_height, bool raw__one_bit, bool is_moon) const;
    godot::String getStarName(double x, double y, double z) const;
    godot::String getPlanetName(double star_x, double star_y, double star_z, int index) const;
    godot::String getPlanetNameById(double planet_id) const;
    godot::Dictionary getCurrentStarInfo();
    godot::Dictionary getAPTargetInfo();
    godot::Dictionary getPlanetInfo(int n);
    void setDzat(double parsis_x, double parsis_y, double parsis_z);
    void setNearstar(double parsis_x, double parsis_y, double parsis_z);
    void saveModels() const;

    void preparePlanetSurface();
    godot::Ref<godot::Image> returnSkyImage();
    godot::Ref<godot::Image> returnSurfacemapImage();
    godot::Ref<godot::Image> returnRuinschartImage();
    godot::Ref<godot::Image> returnTxtrImage(bool raw__one_byte);

    void freeze();
    void unfreeze();
    void processCommBinFile();
    
    void generateSurfacePolygons();

    godot::String getCWD() const;

    void additionalConsumes();

    godot::Array getAnimals();

    int getAPTargetted();
    void setAPTargetted(int i);
    void setAPTargettedWithoutExtractingTargetInfos(int i);
    void setAPTargetX(double x);
    double getAPTargetX();
    void setAPTargetY(double y);
    double getAPTargetY();
    void setAPTargetZ(double z);
    double getAPTargetZ();

    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ip_reached, getIPReached, setIPReached); // 1 if we're orbiting a planet
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ip_reaching, getIPReaching, setIPReaching); // 1 if we're approaching a local target
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, nsync, getNSync, setNSync); // drive tracking mode (i.e. how the stardrifter orbits around a planet)
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, stspeed, getSTSpeed, setSTSpeed); // 1 if we're in Vimana flight
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ap_reached, getAPReached, setAPReached); // 1 if we're in a solar system
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(float, float, charge, getLithiumCharge, setLithiumCharge);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, pwr, getPwr, setPwr);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, ilightv, getIlightv, setIlightv);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, autoscreenoff, getAutoscreenoff, setAutoscreenoff);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, revcontrols, getRevcontrols, setRevcontrols); // 1 if pitch (well, yaw, actually) controls are inverted ('reversed'), 0 otherwise

    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, landing_pt_lat, getLandingPtLat, setLandingPtLat);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, landing_pt_lon, getLandingPtLon, setLandingPtLon);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int8_t, int, landing_point, getLandingPoint, setLandingPoint); // 0 normally, 1 if we're selecting our landing point in the Stardrifter (9 - deploy surface capsule from FCD)

    DECLARE_NOCTIS_VARIABLE_ACCESSORS(float, float, planet_grav, getPlanetGravity, setPlanetGravity);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(float, float, pp_pressure, getPlanetPressure, setPlanetPressure);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(float, float, pp_temp, getPlanetTemperature, setPlanetTemperature);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(int16_t, int, nightzone, getNightzone, setNightzone);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(uint8_t, int, sky_brightness, getSkyBrightness, setSkyBrightness);
    DECLARE_NOCTIS_VARIABLE_ACCESSORS(float, float, rainy, getRainy, setRainy);


    void setDzatX(double x);
    double getDzatX();
    void setDzatY(double y);
    double getDzatY();
    void setDzatZ(double z);
    double getDzatZ();

    double getIPTargettedX();
    double getIPTargettedY();
    double getIPTargettedZ();

    double getNearstarX();
    double getNearstarY();
    double getNearstarZ();
    
    int getStarnopEstimate(double x, double y, double z);

    float getAPStarMass();

    // Player position and camera orientation
    double getPosX();
    double getPosY();
    double getPosZ();
    double getUserAlfa();
    double getUserBeta();

    void setIPTargetted(int new_target);
    int8_t getIPTargetted();
    void updateStarParticles(double parsis_x, double parsis_y, double parsis_z, double distanceMultiplier, godot::NodePath nodePath);
    void updateCurrentStarPlanets(godot::NodePath nodePath);
    void lock();
    void unlock();
    void scanStars();
    void onRingParticleFound(double xlight, double ylight, double zlight, double radii, int unconditioned_color, int body_index);
    void prepareSurfaceScattering(godot::Node3D* target, godot::String scenePath, bool singleMesh);
    void prepareSurfaceMesh(godot::Node3D* target, godot::String scenePath);
    void onScatteringBegin();
    void onScatteringEnd();
    void onScatteringItemBegin();
    void onScatteringItemEnd();
    void onSurfaceBegin();
    void onSurfaceEnd();
    void onSurfacePolygon3Found(double x0, double x1, double x2, double y0, double y1, double y2, double z0, double z1, double z2, int colore);
    void onScatteringPolygon3Found(double x0, double x1, double x2, double y0, double y1, double y2, double z0, double z1, double z2, int colore);
    void onStarFound(double x, double y, double z, double id_code);
    void onPlanetFound(int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t viewpoint, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist);
    void updateTime();
    void setSecs(double s);
    double getSecs();
    void loopOneIter();

    godot::String getFCSStatus();
    void setFCSStatus(godot::String status);

    Feltyrion();
protected:
    static void _bind_methods();
private:
    godot::Mutex mutex;

    godot::Node3D *surfaceScatteringNode;
    godot::Node3D *surfaceMeshNode;
    bool scatteringSingleMesh;
    godot::Ref<godot::SurfaceTool> surfaceTool;
    godot::Ref<godot::PackedScene> scatteringObjectScene;
    godot::Ref<godot::PackedScene> surfaceMeshScene;
    godot::Ref<godot::ImageTexture> surfacePaletteTxtr;
    godot::Ref<godot::ImageTexture> surfaceAlbedoL8Txtr;

    void instantiateSurfaceTool();
    void commitSurfaceTool(int8_t whichUsecase);
    void addSurfaceToolPolygon3(double x0, double x1, double x2, double y0, double y1, double y2, double z0, double z1, double z2, int colore);
};
