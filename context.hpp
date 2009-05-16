//==============================================================================

#ifndef ArseContext
#define ArseContext

class Entity;

#pragma once

class Context
{
  public:
    Context();
    virtual ~Context();

    void notifyOnCollision( bool handle );
    bool handlesCollisions();
    virtual bool shouldCollide( Entity * left, Entity * right );

  private:
    Context( const Context & );
    Context & operator=( const Context & );

  public:
    virtual void init() = 0;
    virtual void fini() = 0;
    virtual bool update( float dt ) = 0;
    virtual void render() = 0;

  private:
    bool m_handles_collisions;
};

#endif

//==============================================================================
