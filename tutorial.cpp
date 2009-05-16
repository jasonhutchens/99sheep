//==============================================================================

#include <tutorial.hpp>
#include <engine.hpp>
#include <viewport.hpp>

#include <hgeresource.h>

//==============================================================================
Tutorial::Tutorial()
    :
    Context()
{
}

//------------------------------------------------------------------------------
Tutorial::~Tutorial()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Tutorial::init()
{
    HGE * hge( Engine::hge() );
    b2World * b2d( Engine::b2d() );
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );

    vp->offset().x = 0.0f;
    vp->offset().y = 0.0f;
    vp->centre().x = 0.0f;
    vp->centre().y = 0.0f;
    vp->bounds().x = 800.0f;
    vp->bounds().y = 600.0f;
    vp->setAngle( 0.0f );
}

//------------------------------------------------------------------------------
void
Tutorial::fini()
{
    Engine::instance()->getConfig().menu = 2;
}

//------------------------------------------------------------------------------
bool
Tutorial::update( float dt )
{
    const Controller & pad( Engine::instance()->getController() );
    HGE * hge( Engine::hge() );

    if ( hge->Input_GetKey() != 0 &&
         ! Engine::instance()->handledKey() )
    {
        Engine::instance()->switchContext( STATE_MENU );
        return false;
    }
    if ( pad.buttonDown( XPAD_BUTTON_A ) ||
         pad.buttonDown( XPAD_BUTTON_B ) ||
         pad.buttonDown( XPAD_BUTTON_START ) ||
         pad.buttonDown( XPAD_BUTTON_BACK ) )
    {
        Engine::instance()->switchContext( STATE_MENU );
        return false;
    }

    return false;
}

//------------------------------------------------------------------------------
void
Tutorial::render()
{
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );
    
    vp->setTransform();

    rm->GetSprite( "help" )->Render( 0.0f, 0.0f ); 
}

//==============================================================================
