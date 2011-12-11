#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector3.h"
#include <list>
#include "GLWidget.h"

using std::list;

class Particle
{
public:
        Particle(int id);

        void Update(long time);

private:
        void Rotate(float angle, float &x, float &y);

public:
        int id;
        Vector3 color;
        Vector3 position;
        Vector3 velocity;
        Vector3 acceleration;
        Vector3 rotation;

        long lastTime;

        float totalLife;
        float life;

        float alpha;
        float size;

        float bounciness;

        bool active;

};

#endif // PARTICLE_H
