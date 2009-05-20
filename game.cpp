//==============================================================================

#include <game.hpp>
#include <engine.hpp>
#include <entity.hpp>
#include <entity_manager.hpp>
#include <viewport.hpp>
#include <fujin.hpp>
#include <bullet.hpp>
#include <cloud.hpp>
#include <girder.hpp>
#include <score.hpp>

#include <hgeresource.h>

#include <algorithm>
#include <set>

namespace
{
    const float ZOOM[5] = { 1.0f, 1.8f, 3.2f, 5.8f, 10.5f };
    const float FUJIN( 0.8f );

    bool
    lessThan( const Entity * left, const Entity * right )
    {
        if ( left->getZoom() > right->getZoom() )
        {
            return true;
        }
        if ( left->getZoom() < right->getZoom() )
        {
            return false;
        }
        if ( left->getType() > right->getType() )
        {
            return true;
        }
        return false;
    }

    bool
    equal( const Entity * left, const Entity * right )
    {
        if ( left->getZoom() == right->getZoom() )
        {
            return true;
        }
        return false;
    }
}

//==============================================================================
Game::Game()
    :
    Context(),
    m_last_zoom( 1.0f ),
    m_zoom( 0 ),
    m_fujin( 0 ),
    m_gameOutTimer(0),
    m_black( true )
{
}

//------------------------------------------------------------------------------
Game::~Game()
{
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
void
Game::init()
{
    HGE * hge( Engine::hge() );
    b2World * b2d( Engine::b2d() );
    hgeResourceManager * rm( Engine::rm() );
    ViewPort * vp( Engine::vp() );

    notifyOnCollision( true );

    Fujin::registerEntity();
    Bullet::registerEntity();
    Cloud::registerEntity();
    Girder::registerEntity();

    m_last_zoom = 1.0f;
    m_gameOutTimer = 0;
    m_zoom = 0;

    m_timeRemaining = 300;
    m_score = 0;
    Engine::em()->init();

    vp->offset().x = 0.0f;
    vp->offset().y = 0.0f;
    vp->centre().x = 0.0f;
    vp->centre().y = 0.0f;
    vp->bounds().x = 1280.0f;
    vp->bounds().y = 720.0f;
    vp->setAngle( 0.0f );
    vp->setScale( 10.0f );

    m_fujin = static_cast< Fujin * >( Engine::em()->factory( Fujin::TYPE ) );
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );
    m_fujin->setSprite( "white_ship" );
    m_fujin->setScale( 0.1f );
    m_fujin->init();
    m_fujin->getBody()->SetXForm( position, angle );
    m_fujin->setTargetScale( FUJIN / ZOOM[m_zoom] );
    m_fujin->setZoom( m_zoom );

    setColour( 0xFFFFFFFF );
    m_fujin->setBlack( true );

    for (int i = 0; i < 8; ++i)
    {
        Entity* entity = Engine::em()->factory( Cloud::TYPE );
        b2Vec2 position( Engine::hge()->Random_Float( -60.0f, 60.0f),
                         Engine::hge()->Random_Float( -35.0f, 35.0f) );
        float angle( Engine::hge()->Random_Float( -7.0f, 7.0f) );
        static_cast< Cloud * >( entity )->setSize( Engine::hge()->Random_Int( 0, 3 ) );
        entity->setBlack( Engine::hge()->Random_Int( 0, 1 ) == 0 );
        entity->setScale( 0.1f );
        entity->init();
        entity->getBody()->SetXForm( position, angle );
    }

    _initArena();
}

//------------------------------------------------------------------------------
void
Game::fini()
{
    notifyOnCollision( false );

    Engine::em()->fini();
}

//------------------------------------------------------------------------------
bool
Game::update( float dt )
{
    const Controller & pad( Engine::instance()->getController() );
    HGE * hge( Engine::hge() );
    ViewPort * vp( Engine::vp() );

    if ( m_gameOutTimer <= 0 && m_timeRemaining <=0)
    {
        return false;
    }

    Engine::em()->update( dt );

    if ( Engine::instance()->isPaused() )
    {
        return false;
    }

    if ( pad.isConnected() )
    {
        if ( pad.buttonDown( XPAD_BUTTON_LEFT_SHOULDER ) )
        {
            m_black = true;
            setColour( 0xFFFFFFFF );
            m_fujin->setBlack( true );
        }
        else if ( pad.buttonDown( XPAD_BUTTON_RIGHT_SHOULDER ) )
        {
            m_black = false;
            setColour( 0xFF000000 );
            m_fujin->setBlack( false );
        }
    }
    else
    {
        if ( ( Engine::hge()->Input_KeyDown( HGEK_Q ) ||
               hge->Input_GetMouseWheel() < 0 ) )
        {
            m_black = true;
            setColour( 0xFFFFFFFF );
            m_fujin->setBlack( true );
        }
        else if ( ( Engine::hge()->Input_KeyDown( HGEK_E ) ||
                    hge->Input_GetMouseWheel() > 0 ) )
        {
            m_black = false;
            setColour( 0xFF000000 );
            m_fujin->setBlack( false );
        }
    }

    if ( ZOOM[m_zoom] > m_last_zoom )
    {
        m_last_zoom += ( ZOOM[m_zoom] - m_last_zoom ) * dt * 10.0f;
        vp->setScale( m_last_zoom );
        m_fujin->setScale( FUJIN / m_last_zoom );
    }
    else if ( ZOOM[m_zoom] < m_last_zoom )
    {
        m_last_zoom += ( ZOOM[m_zoom] - m_last_zoom ) * dt * 10.0f;
        vp->setScale( m_last_zoom );
        m_fujin->setScale( FUJIN / m_last_zoom );
    }

    vp->centre() = m_fujin->getBody()->GetPosition();

    return false;
}

//------------------------------------------------------------------------------
void
Game::render()
{
    hgeResourceManager * rm( Engine::rm() );
    hgeFont* font = Engine::rm()->GetFont("menu");
    b2Vec2 timeTextLocation (700,10);
    b2Vec2 scoreTextLocation(0,10);
    char timeRemainingText[10];
    sprintf_s(timeRemainingText,"%d:%02d",(int)m_timeRemaining/60,(int)(m_timeRemaining)%60);

    ViewPort * vp( Engine::vp() );
    
    vp->setTransform();

    std::vector< Entity * > entities;
    for ( b2Body * body( Engine::b2d()->GetBodyList() ); body != NULL;
          body = body->GetNext() )
    {
        Entity * entity( static_cast< Entity * >( body->GetUserData() ) );
        if ( entity )
        {
            entities.push_back( entity );
        }
    }

    std::sort( entities.begin(), entities.end(), lessThan );

    std::vector< Entity * >::iterator i;
    float scale( 1.0f / static_cast< float >( ZOOM[m_zoom] ) );
    for ( i = entities.begin(); i != entities.end(); ++i )
    {
        Entity * entity( * i );
        entity->render( scale );
    }
    // render time remaining
    Engine::hge()->Gfx_SetTransform();

    std::string progressText;
    std::list<int>::iterator iter;
    for ( iter = m_progress.begin(); iter != m_progress.end(); ++iter )
    {
        for (int i = (*iter); i > 0; --i)
        {
            progressText.append("@");
        }
        progressText.append("  ");
    }

    if ( m_black )
    {
        font->SetColor( 0xFF000000 );
    }
    else
    {
        font->SetColor( 0xFFFFFFFF );
    }

    vp->setTransform();
}

//------------------------------------------------------------------------------
bool
Game::shouldCollide( Entity * left, Entity * right )
{
    if ( left->getType() == Girder::TYPE || right->getType() == Girder::TYPE )
    {
        return true;
    }

    if ( equal( left, right ) )
    {
        return left->getType() == right->getType();
    }

    return false;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Game::_initArena()
{
    b2Vec2 position( 0.0f, 0.0f );
    b2Vec2 dimensions( 0.0f, 0.0f );
    Entity * entity( 0 );

    for ( int i = 0; i < 4; ++i )
    {
        switch( i )
        {
            case 0:
            {
                dimensions.x = 128.0f;
                dimensions.y = 0.1f;
                position.x = 0.0f;
                position.y = -36.1f;
                break;
            }
            case 1:
            {
                dimensions.x = 0.1f;
                dimensions.y = 72.0f;
                position.x = 64.1f;
                position.y = 0.0f;
                break;
            }
            case 2:
            {
                dimensions.x = 128.0f;
                dimensions.y = 0.1f;
                position.x = 0.0f;
                position.y = 36.1f;
                break;
            }
            case 3:
            {
                dimensions.x = 0.1f;
                dimensions.y = 72.0f;
                position.x = -64.1f;
                position.y = 0.0f;
                break;
            }
        }
        Girder * girder( static_cast< Girder * >(
            Engine::em()->factory( Girder::TYPE ) ) );
        girder->setScale( 1.0f );
        girder->setDimensions( dimensions );
        girder->init();
        girder->getBody()->SetXForm( position, 0.0f );
    }
}
