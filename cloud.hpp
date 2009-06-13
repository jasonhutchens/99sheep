//==============================================================================

#ifndef ArseCloud
#define ArseCloud

#pragma once

#include <entity.hpp>
#include <damageable.hpp>
#include <Box2D.h>

//------------------------------------------------------------------------------
class Cloud : public Entity, public Damageable
{
  public:
    static const unsigned int TYPE = 2;
    static Entity * factory() { return new Cloud(); }

    Cloud( float scale = 1.0f );
    virtual ~Cloud();

    virtual void collide( Entity * entity, b2ContactPoint * point );

    virtual void persistToDatabase();

    int getSize() const;
    void setSize( int size );

    bool isHarmful() const;
    bool getFriend() const;
    void setFriend( bool b_friend );

    static void registerEntity();

  protected:
    Cloud( const Cloud & );
    Cloud & operator=( const Cloud & );

    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender( float scale );
    virtual void initFromQuery( Query & query );

  private:
    int m_size;
    bool m_friend;
    float m_life;
};

#endif

//==============================================================================
