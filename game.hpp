//==============================================================================

#ifndef ArseGame
#define ArseGame

#pragma once

#include <context.hpp>
#include <list>

class Fujin;
class Entity;

//------------------------------------------------------------------------------
class Game : public Context
{
  public:
    Game();
    virtual ~Game();

  private:
    Game( const Game & );
    Game & operator=( const Game & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();
    virtual bool shouldCollide( Entity * left, Entity * right );

	int getZoom() { return m_zoom; }

  private:
    void _initArena();

	std::list<int> m_progress;
    float m_last_zoom;
    int m_zoom;
    Fujin * m_fujin;
	float m_timeRemaining;
	int m_score;
	float m_gameOutTimer;
    bool m_black;
};

#endif

//==============================================================================
