#include "SensorBody.h"

namespace CGameEngine
{
    SensorBody::SensorBody(b2World* world, CGameEngine::Body* hostBody, const float& radius)
    {
        if(!world) { return; }

        b2FixtureDef fixtureDef;
        b2CircleShape circleShape;
        circleShape.m_radius = radius;
        fixtureDef.shape = &circleShape;
        fixtureDef.isSensor = true;
        m_fixture = hostBody->getb2Body()->CreateFixture(&fixtureDef);
    }
}
