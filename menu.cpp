//==============================================================================

#include <menu.hpp>
#include <menu_item.hpp>
#include <engine.hpp>
#include <viewport.hpp>
#include <config.hpp>

#include <hgeresource.h>
#include <hgefont.h>

//------------------------------------------------------------------------------
Menu::Menu()
    :
    Context(),
    m_font( 0 ),
    m_gui( 0 )
{
}

//------------------------------------------------------------------------------
Menu::~Menu()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Menu::init()
{
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );
    Config & config( Engine::instance()->getConfig() );

    char * label = config.leaderboard ? "Leaderboard" : "High Scores";

    m_font = rm->GetFont( "menu" );
    m_gui = new hgeGUI();
    float cx( 0.5f * vp->screen().x );
    float cy( 0.5f * vp->screen().y - 70 );
    m_gui->AddCtrl( new MenuItem( CTRL_START, cx, cy + 30, "Start", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_SCORE, cx, cy + 80, label, m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_HOME, cx, cy + 130, "Home Page", m_font ) );
    m_gui->AddCtrl( new MenuItem( CTRL_EXIT, cx, cy + 200, "Exit", m_font ) );
    m_gui->SetNavMode( HGEGUI_UPDOWN );
    m_gui->SetFocus( Engine::instance()->getConfig().menu );
    m_gui->Enter();
}

//------------------------------------------------------------------------------
void
Menu::fini()
{
    m_gui->DelCtrl( CTRL_START );
    m_gui->DelCtrl( CTRL_SCORE );
    m_gui->DelCtrl( CTRL_HOME );
    m_gui->DelCtrl( CTRL_EXIT );
    delete m_gui;
    m_gui = 0;
    m_font = 0;
}

//------------------------------------------------------------------------------
bool
Menu::update( float dt )
{
    HGE * hge( Engine::hge() );
    Engine * engine( Engine::instance() );

    if ( hge->Input_KeyDown( HGEK_ESCAPE ) )
    {
        return true;
    }

    switch ( static_cast< Control >( engine->updateGUI( dt, m_gui,
                                     engine->getConfig().menu, 4 ) ) )
    {
        case CTRL_START:
        {
            Engine::instance()->switchContext( STATE_GAME );
            break;
        }
        case CTRL_SCORE:
        {
            Engine::instance()->switchContext( STATE_SCORE );
            break;
        }
        case CTRL_HOME:
        {
            hge->System_Launch( "http://rockethands.com/99sheep" );
        }
        case CTRL_EXIT:
        {
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
void
Menu::render()
{
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );

    m_gui->Render();
    float cx( 0.5f * vp->screen().x );
    rm->GetSprite( "title" )->Render( cx, 100.0f );

    hgeFont * font( rm->GetFont( "menu" ) );
    font->SetColor( 0xFFFFFFFF );
    font->printf( cx, 150.0f, HGETEXT_CENTER, "A RocketHands Experiment by Lloyd Kranzky" );
    font->printf( cx, vp->screen().y - 70.0f, HGETEXT_CENTER, "Copyright (c) 2009 RocketHands Pty. Ltd.  All rights reserved." );
    font->printf( cx, vp->screen().y - 40.0f, HGETEXT_CENTER, "http://rockethands.com/99sheep" );
}

//==============================================================================
