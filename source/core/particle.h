#ifndef PARTICLE_H__
#define PARTICLE_H__

/* This is meant as a common pool of particles to be used by badge apps for cosmetic "sparks"
 * "explosions", etc.
 *
 * It has default functions for adding particles, drawing particles, moving particles, etc.
 * but is customiziable by allowing replacement of these functions by a badge app's own
 * functions in the particle_pool_config.
 *
 * Note: because the particle pool is shared between badge apps, if a badge app pauses,
 * and another badge app runs, then the original app resumes, the particle pool may have
 * been altered by the intermediate app.  For this reason care must be taken. The
 * current_badge_app field of struct particle_pool is meant to allow badge apps to
 * cooperatively communicate that they have used the pool, so if a badge app sees that
 * the particle pool current_badge_app is different than what it expects, it knows that
 * the contents of the particle pool are suspect.
 *
 */

#define MAX_PARTICLES 400

struct particle {
	/* Note: x, y, z, vx, vy, vz are 24.8 fixed point numbers.
	 * if you don't use the 3d functions, z, and vz are unused
	 * if you need to do, e.g. 3d perspective projection, you must
	 * override the particle drawing functions and write that code
	 * yourself.
	 */
	int x, y, z, vx, vy, vz, life, color;
};

struct particle_pool;

struct particle_pool_config {
	int gravityx, gravityy, gravityz; /* 24.8 fixed point numbers */
	int maxparticles;
	void (*add_particle)(struct particle_pool *pool, int x, int y, int vx, int vy, int life, int color);
	void (*add_3d_particle)(struct particle_pool *pool, int x, int y, int z,
			int vx, int vy, int vz, int life, int color);
	void (*remove_particle)(struct particle_pool *pool, struct particle *p);
	void (*move_particle)(struct particle_pool *pool, struct particle *p);
	void (*draw_particle)(struct particle *p);
	void (*draw_particle_color)(struct particle *p, int color);
	void (*move_particles)(struct particle_pool *pool);
	void (*draw_particles)(struct particle_pool *pool);
	void (*draw_particles_color)(struct particle_pool *pool, int color);
	void *cookie; /* For use by badge apps */
};

struct particle_pool {
	int current_badge_app; /* to allow apps to detect if the particle pool was tampered with by another app */
	struct particle_pool_config config;
	struct particle p[MAX_PARTICLES];
	int nparticles;
};

struct particle_pool *get_common_particle_pool(void);

void particle_pool_set_config(struct particle_pool *pool, struct particle_pool_config *config);
struct particle_pool_config *particle_pool_get_config(struct particle_pool *pool);

void add_particle_default(struct particle_pool *pool, int x, int y, int vx, int vy, int life, int color);
void add_3d_particle_default(struct particle_pool *pool, int x, int y, int z,
						int vx, int vy, int vz, int life, int color);
void remove_particle_default(struct particle_pool *pool, struct particle *p);
void move_particle_default(struct particle_pool *pool, struct particle *p);
void draw_particle_default(struct particle *p);
void draw_particle_color_default(struct particle *p, int color);
void move_particles_default(struct particle_pool *pool);
void draw_particles_default(struct particle_pool *pool);
void draw_particles_color_default(struct particle_pool *pool, int color);

const struct particle_pool_config default_particle_pool_config
#ifdef PARTICLE_H_DEFINE_GLOBALS
 = {
	.gravityx = 0,
	.gravityy = 0,
	.gravityz = 0,
	.maxparticles = MAX_PARTICLES,
	.add_particle = add_particle_default,
	.add_3d_particle = add_3d_particle_default,
	.remove_particle = remove_particle_default,
	.move_particle = move_particle_default,
	.draw_particle = draw_particle_default,
	.draw_particle_color = draw_particle_color_default,
	.move_particles = move_particles_default,
	.draw_particles = draw_particles_default,
	.draw_particles_color = draw_particles_color_default,
}
#endif
;

#endif

