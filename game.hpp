//==============================================================================

#ifndef ArseGame
#define ArseGame

#pragma once

#include <context.hpp>
#include <list>

class Fujin;
class Entity;
class Girder;
class hgeSprite;

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
    bool getBlack() { return m_black; }

  private:
    void _initArena();

	std::list<int> m_progress;
    float m_last_zoom;
    int m_zoom;
    Fujin * m_fujin;
	float m_timeRemaining;
	int m_score;
	float m_gameOutTimer;
	float m_gameInTimer;
    bool m_black;
    Girder * m_shield;
    hgeSprite * m_overlay;
    int m_message;
    HCHANNEL m_channel;
    float m_timer;
};

#endif

//==============================================================================
