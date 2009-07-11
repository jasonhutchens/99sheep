//==============================================================================

#ifndef ArseTute
#define ArseTute

#pragma once

#include <context.hpp>

class Fujin;

//------------------------------------------------------------------------------
class Tutorial : public Context
{
  public:
    Tutorial();
    virtual ~Tutorial();

  private:
    Tutorial( const Tutorial & );
    Tutorial & operator=( const Tutorial & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    void _initArena();
    Fujin * m_fujin;
};

#endif

//==============================================================================
