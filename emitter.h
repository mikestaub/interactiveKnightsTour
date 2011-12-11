#ifndef EMITTER_H
#define EMITTER_H

#include "particle.h"
#include "GLWidget.h"
#include "texture.h"
#include <list>

using std::list;
class Particle;
class Texture;

class Emitter
{
public:
        Emitter();
        ~Emitter();

        void Update(long time);
        void setTexture(Texture* texture);

protected:
        void addParticle(void);

        float frand(float start = 0, float end = 1);
public:
        list<Particle*> particles;

        Texture* texture;

        long lastTime;

        // Attributes
        float emissionRate;
        float emissionRadius;

        float life;
        float lifeRange;

        float size;
        float sizeRange;

        float saturation;
        float alpha;

        float spread;

        float gravity;

        Vector3 position;
        Vector3 wind;
        Vector3 rotation;
};

#endif // EMITTER_H
