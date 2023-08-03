#pragma once

#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/classes/node3d.hpp"
#include "godot_cpp/core/binder_common.hpp"
#include "godot_cpp/classes/mutex.hpp"

class Feltyrion : public godot::Control
{
    GDCLASS( Feltyrion, godot::Control )

public:
    // Functions.
    void prepareStar();
    void loadPlanet(int logical_id, int type, double seedval, bool lighting, bool include_atmosphere) const;
    godot::Ref<godot::Image> getPaletteAsImage() const;
    godot::Ref<godot::Image> returnAtmosphereImage(bool accurate_height) const;
    godot::Ref<godot::Image> returnImage(bool accurate_height, bool raw__one_bit) const;
    godot::String getStarName(double x, double y, double z) const;
    godot::String getPlanetName(double star_x, double star_y, double star_z, int index) const;
    godot::String getPlanetNameById(double planet_id) const;
    godot::Dictionary getCurrentStarInfo();
    godot::Dictionary getAPTargetInfo();
    godot::Dictionary getPlanetInfo(int n);
    godot::Vector3 getDzat();
    void setDzat(double parsis_x, double parsis_y, double parsis_z);
    void setNearstar(double parsis_x, double parsis_y, double parsis_z);
    void saveModels() const;
    void Feltyrion::setAPTarget(godot::Vector3 ap_target);
    godot::Vector3 Feltyrion::getAPTarget();
    void updateStarParticles(double parsis_x, double parsis_y, double parsis_z, godot::NodePath nodePath);
    void updateCurrentStarPlanets(godot::NodePath nodePath);
    void lock();
    void unlock();
    void scanStars();
    void onStarFound(double x, double y, double z, double id_code);
    void onPlanetFound(int8_t index, double planet_id, double seedval, double x, double y, double z, int8_t type, int16_t owner, int8_t moonid, double ring, double tilt, double ray, double orb_ray, double orb_tilt, double orb_orient, double orb_ecc, int16_t rtperiod, int16_t rotation, int16_t viewpoint, int16_t term_start, int16_t term_end, int16_t qsortindex, float qsortdist);
    void updateTime();
    void setSecs(double s);
    double getSecs();

    Feltyrion();
protected:
    static void _bind_methods();
private:
    godot::Mutex mutex;
};
