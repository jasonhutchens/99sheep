//==============================================================================

#ifndef ArseCloud
#define ArseCloud

#pragma once

#include <entity.hpp>
#include <Box2D.h>

class hgeParticleSystem;
class Clump;

//------------------------------------------------------------------------------
class Cloud : public Entity
{
  public:
    static const unsigned int TYPE = 2;
    static Entity * factory() { return new Cloud(); }

    Cloud( float scale = 1.0f );
    virtual ~Cloud();

    virtual void collide( Entity * entity, b2ContactPoint * point );

    virtual void persistToDatabase();

    static void registerEntity();

	Clump* getClump() const { return m_clump; }
	void setClump(Clump* clump) { m_clump = clump; }
	void removeFromClump(bool notifyClumpManager);

	void removeFromWorld();
	void addToWorld(b2Vec2 position, float angle, float scale);
	bool isInWorld() { return m_inWorld; }

  protected:
    Cloud( const Cloud & );
    Cloud & operator=( const Cloud & );

    virtual void doInit();
    virtual void doUpdate( float dt );
    virtual void doRender( float scale );
    virtual void initFromQuery( Query & query );

  private:

    hgeParticleSystem* m_particles;
	Clump* m_clump;
	bool m_inWorld;
};

#endif

//==============================================================================
