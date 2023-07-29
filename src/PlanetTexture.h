#pragma once

#include "godot_cpp/classes/control.hpp"
#include "godot_cpp/core/binder_common.hpp"
#include "godot_cpp/classes/mutex.hpp"

class Feltyrion : public godot::Control
{
    GDCLASS( Feltyrion, godot::Control )

public:
    // Functions.
    void prepareStar() const;
    void loadPlanet(int logical_id, int type, double seedval, bool lighting, bool include_atmosphere) const;
    godot::Ref<godot::Image> getPaletteAsImage() const;
    godot::Ref<godot::Image> returnAtmosphereImage() const;
    godot::Ref<godot::Image> returnImage(bool raw__one_bit) const;
    void lock();
    void unlock();

    Feltyrion();
protected:
    static void _bind_methods();
private:
    godot::Mutex mutex;
};
