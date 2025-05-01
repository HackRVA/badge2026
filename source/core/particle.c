#define PARTICLE_H_DEFINE_GLOBALS
#include "particle.h"
#undef PARTICLE_H_DEFINE_GLOBALS
#include "framebuffer.h"
#include <stddef.h>

/* get this via get_common_particle_pool)() */
static struct particle_pool common_particle = {
	.config = default_particle_pool_config,
	.p = { { 0, }, },
	.nparticles = 0,
};

struct particle_pool *get_common_particle_pool(void)
{
	return &common_particle;
}

void add_particle_default(struct particle_pool *pool, int x, int y, int vx, int vy, int life, int color)
{
	if (pool->nparticles >= pool->config.maxparticles)
		return;

	pool->p[pool->nparticles].x = x;
	pool->p[pool->nparticles].y = y;
	pool->p[pool->nparticles].z = 0;
	pool->p[pool->nparticles].vx = vx;
	pool->p[pool->nparticles].vy = vy;
	pool->p[pool->nparticles].vz = 0;
	pool->p[pool->nparticles].life = life;
	pool->p[pool->nparticles].color = color;
	pool->nparticles++;
}

void add_3d_particle_default(struct particle_pool *pool, int x, int y, int z,
				int vx, int vy, int vz, int life, int color)
{
	if (pool->nparticles >= pool->config.maxparticles)
		return;

	pool->p[pool->nparticles].x = x;
	pool->p[pool->nparticles].y = y;
	pool->p[pool->nparticles].z = z;
	pool->p[pool->nparticles].vx = vx;
	pool->p[pool->nparticles].vy = vy;
	pool->p[pool->nparticles].vz = vz;
	pool->p[pool->nparticles].life = life;
	pool->p[pool->nparticles].color = color;
	pool->nparticles++;
}

void remove_particle_default(struct particle_pool *pool, struct particle *p)
{
	ptrdiff_t index = p - &pool->p[0];
	if (index > pool->nparticles || index < 0)
		return;
	if (index < pool->nparticles - 1)
		pool->p[index] = pool->p[pool->nparticles - 1]; /* overwrite deleted particle with last particle */
	pool->nparticles--;
}

void move_particle_default(struct particle_pool *pool, struct particle *p)
{
	p->x += p->vx;
	p->y += p->vy;
	p->z += p->vz;
	p->vx += pool->config.gravityx;
	p->vy += pool->config.gravityy;
	p->vz += pool->config.gravityz;
	if (p->life > 0)
		p->life--;
}

void move_particles_default(struct particle_pool *pool)
{
	int i = 0;
	while (i < pool->nparticles) {
		pool->config.move_particle(pool, &pool->p[i]);
		if (pool->p[i].life == 0)
			pool->config.remove_particle(pool, &pool->p[i]);
		else
			i++;
	}
}

void draw_particle_default(struct particle *p)
{
	int x, y;

	x = p->x / 256;
	y = p->y / 256;
	if (x >= 0 && x < LCD_XSIZE && y >= 0 && y < LCD_YSIZE) {
		FbColor(p->color);
		FbPoint((unsigned char) x, (unsigned char) y);
	}
}

void draw_particle_color_default(struct particle *p, int color)
{
	int x, y;

	x = p->x / 256;
	y = p->y / 256;
	if (x >= 0 && x < LCD_XSIZE && y >= 0 && y < LCD_YSIZE) {
		FbColor(color);
		FbPoint((unsigned char) x, (unsigned char) y);
	}
}

void draw_particles_default(struct particle_pool *pool)
{
	for (int i = 0; i < pool->nparticles; i++)
		pool->config.draw_particle(&pool->p[i]);
}

void draw_particles_color_default(struct particle_pool *pool, int color)
{
	for (int i = 0; i < pool->nparticles; i++) {
		pool->config.draw_particle_color(&pool->p[i], color);
	}
}

