//==============================================================================

#ifndef ArseMenu
#define ArseMenu

#pragma once

#include <hge.h>

#include <context.hpp>

class hgeFont;
class hgeGUI;

enum Control
{
    CTRL_NONE = 0,
    CTRL_START = 1,
    CTRL_SCORE = 2,
    CTRL_EXIT = 3
};

//------------------------------------------------------------------------------
class Menu : public Context
{
  public:
    Menu();
    virtual ~Menu();

  private:
    Menu( const Menu & );
    Menu & operator=( const Menu & );

  public:
    virtual void init();
    virtual void fini();
    virtual bool update( float dt );
    virtual void render();

  private:
    hgeFont * m_font;
    hgeGUI * m_gui;
};

#endif

//==============================================================================
