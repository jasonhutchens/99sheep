//==============================================================================

#ifndef ArseFujin
#define ArseFujin

#pragma once

#include <entity.hpp>
#include <damageable.hpp>
#include <Box2D.h>
#include <hge.h>

#include <vector>

class Bullet;
class Cloud;

//------------------------------------------------------------------------------
class Fujin : public Entity, public Damageable
{
  public:
    static const unsigned int TYPE = 1;
    static Entity * factory() { return new Fujin(); }

    Fujin( float max_strength = 1.0f, float scale = 1.0f );
    virtual ~Fujin();

    virtual void collide( Entity * entity, b2ContactPoint * point );

    virtual void persistToDatabase();

    static void registerEntity();

    void setTargetScale( float scale );

    int getScore() const;

  protected:
    Fujin( const Fujin & );
    Fujin & operator=( const Fujin & );

    virtual void onSetScale();
    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender( float scale );
    virtual void initFromQuery( Query & query );
    float lookAt(const b2Vec2& targetPoint);

    HCHANNEL m_channel;
    float m_target_scale;
    float m_bullet_timer;
    std::vector< Cloud * > m_join;
    std::vector< Cloud * > m_friends;
};

#endif

//==============================================================================
