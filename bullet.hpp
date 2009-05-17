//==============================================================================

#ifndef ArseBullet
#define ArseBullet

#pragma once

#include <entity.hpp>
#include <damageable.hpp>
#include <Box2D.h>
#include <hge.h>

#include <vector>

//------------------------------------------------------------------------------
class Bullet : public Entity, public Damageable
{
  public:
    static const unsigned int TYPE = 4;
    static Entity * factory() { return new Bullet(); }

    Bullet( float max_strength = 1.0f, float scale = 1.0f );
    virtual ~Bullet();

    virtual void collide( Entity * entity, b2ContactPoint * point );

    virtual void persistToDatabase();

    static void registerEntity();

    void setTargetScale( float scale );

  protected:
    Bullet( const Bullet & );
    Bullet & operator=( const Bullet & );

    virtual void onSetScale();
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender( float scale );
    virtual void initFromQuery( Query & query );
    float lookAt(const b2Vec2& targetPoint);

	b2AABB m_AABB;
    HCHANNEL m_channel;
    float m_target_scale;
    float m_lifetime;
};

#endif

//==============================================================================
