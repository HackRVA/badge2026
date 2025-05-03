#ifndef PARTICLE_H__
#define PARTICLE_H__

#include <stdint.h>

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
	 *
	 * x, y, z = particle possition.
	 * vx, vy, vz = particle velocity
	 * life = particle lifetime, decrements with every movement until zero
	 * color = particle color
	 */
	int x, y, z;
	int16_t vx, vy, vz, life, color;
};

struct particle_pool;

/* particle_pool_config is used for customizing particle behavior and appearance */
struct particle_pool_config {
	int gravityx, gravityy, gravityz; /* 24.8 fixed point numbers, added to vx,vy,vz with movement */
	int maxparticles; /* be default, MAX_PARTICLES, but apps may lower it */

	/* For adding a new particle */
	void (*add_particle)(struct particle_pool *pool, int x, int y, int vx, int vy, int life, int color);

	/* For adding a new particle in 3D space, same as add particle, but with z and vz */
	void (*add_3d_particle)(struct particle_pool *pool, int x, int y, int z,
			int vx, int vy, int vz, int life, int color);

	void (*remove_particle)(struct particle_pool *pool, struct particle *p);
	void (*move_particle)(struct particle_pool *pool, struct particle *p);
	void (*draw_particle)(struct particle *p);
	/* draw_particle_color draws a particle with a specific color rather than the particle's own
	 * color.  Can be useful for erasing particles with custom drawing functions */
	void (*draw_particle_color)(struct particle *p, int color);
	void (*move_particles)(struct particle_pool *pool);
	void (*draw_particles)(struct particle_pool *pool);

	/* Same as draw_particles but uses the specified color instead of particle's own */
	void (*draw_particles_color)(struct particle_pool *pool, int color);

	void *cookie; /* For use by badge apps, (e.g. battlezone.c stores camera data in here) */
};

extern const struct particle_pool_config default_particle_pool_config;

struct particle_pool {
	int current_badge_app; /* to allow apps to detect if the particle pool was tampered with by another app */
	struct particle_pool_config config; /* any app specific customizations are in here */
	struct particle p[MAX_PARTICLES];
	int nparticles;
};

struct particle_pool *get_common_particle_pool(void);

/* Claim pool for a particular badge app.
 * If pool->current_badge_app == app_signature, returns 0.
 * Otherwise sets pool->current_badge_app = app_signature, and sets
 * pool->config = default_particle_pool_config, and set
 * pool->nparticles = 0, and finally returns 1.
 *
 * The idea is if it returns 1, then the app knows that it must (re)set up any
 * customizations that it might require for pool->config.
 */
int claim_particle_pool(struct particle_pool *pool, int app_signature);

/* Set and get particle_pool_config in one go */
void particle_pool_set_config(struct particle_pool *pool, struct particle_pool_config *config);
struct particle_pool_config *particle_pool_get_config(struct particle_pool *pool);

/* Default functions for default_particle_pool_config */
void add_particle_default(struct particle_pool *pool, int x, int y, int vx, int vy, int life, int color);
void add_3d_particle_default(struct particle_pool *pool, int x, int y, int z,
						int vx, int vy, int vz, int life, int color);
void remove_particle_default(struct particle_pool *pool, struct particle *p);
void move_particle_default(struct particle_pool *pool, struct particle *p);
void draw_particle_default(struct particle *p);
void draw_particle_color_default(struct particle *p, int color);

/* calls pool->config.move_particle() for each live particle in the pool */
void move_particles_default(struct particle_pool *pool);

/* calls pool->config.draw_particle() for each live particle in the pool */
void draw_particles_default(struct particle_pool *pool);

/* calls pool->config.draw_particle_color() for each live particle in the pool */
void draw_particles_color_default(struct particle_pool *pool, int color);

#endif

