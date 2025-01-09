// animal-related externally defined variables

#define LFS 100 // maximum number of animals.
extern int16_t animals; // currently visible animals.
extern int8_t ani_type[LFS]; // type
extern int32_t ani_seed[LFS]; // pseudo seed for shape changes.
extern float ani_scale[LFS]; // scale.
extern float ani_x[LFS]; // position (X)
extern float ani_quote[LFS]; // altitude above ground.
extern float ani_z[LFS]; // position (Z)
extern float ani_pitch[LFS]; // direction they are moving.
extern float ani_speed[LFS]; // current speed.
extern float tgt_quote[LFS]; // altitude they want to reach.
extern float tgt_speed[LFS]; // speed they want to reach.
extern float tgt_pitch[LFS]; // direction they want to acquire.
extern int8_t ani_lcount[LFS]; // proximity time counter.
extern uint16_t ani_sqc[LFS]; // sub-quadrant coordinates (current).
extern int8_t ani_mtype[LFS]; // type of movement.
#define FELINE_LIKE 0
#define RABBIT_LIKE 1
#define KANGAROO_LIKE 2

#define BIRD 1
#define REPTIL 4
#define MAMMAL 5