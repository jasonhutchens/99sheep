//==============================================================================

#include <splash.hpp>
#include <engine.hpp>

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
    Engine::hge()->Effect_PlayEx( rm->GetEffect( "music" ), 20, 0, 1, true );
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
    if ( m_timer > 4.0f )
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

    int width( Engine::hge()->System_GetState( HGE_SCREENWIDTH ) );
    int height( Engine::hge()->System_GetState( HGE_SCREENHEIGHT ) );

    setColour( 0xFFFFFFFF );
    if ( m_timer > 2.0f )
    {
        sprite = rm->GetSprite( "publisher" );
    }
    else
    {
        sprite = rm->GetSprite( "developer" );
    }

    sprite->RenderEx( 0.5f * static_cast<float>( width ),
                      0.5f * static_cast<float>( height ), 0.0f, 0.5f );
}

//==============================================================================
