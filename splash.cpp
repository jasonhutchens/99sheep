//==============================================================================

#include <splash.hpp>
#include <engine.hpp>
#include <viewport.hpp>

#include <hgeresource.h>

//------------------------------------------------------------------------------
Splash::Splash()
    :
    Context(),
    m_timer( 0.0f )
{
}

//------------------------------------------------------------------------------
Splash::~Splash()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Splash::init()
{
    hgeResourceManager * rm( Engine::rm() );
    m_timer = 0.0f;
}

//------------------------------------------------------------------------------
void
Splash::fini()
{
}

//------------------------------------------------------------------------------
bool
Splash::update( float dt )
{
    if ( m_timer > 5.0f )
    {
        Engine::instance()->switchContext( STATE_MENU );
    }

    m_timer += dt;

    return false;
}

//------------------------------------------------------------------------------
void
Splash::render()
{
    hgeResourceManager * rm( Engine::rm() );
    hgeSprite * sprite( 0 );

    float width( Engine::vp()->screen().x );
    float height( Engine::vp()->screen().y );

    setColour( 0xFFFFFFFF );
    setBorder( 0xFF000000 );
    sprite = rm->GetSprite( "developer" );

    sprite->RenderEx( 0.5f * width, 0.5f * height, 0.0f, 1.0f );
}

//==============================================================================
