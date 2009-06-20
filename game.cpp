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

    const char * MESSAGE[4] = {
        "Get Ready",
        "Game Over",
        "Time Gone",
        "Goal Get!"
    };
}

//==============================================================================
Game::Game()
    :
    Context(),
    m_progress(),
    m_last_zoom( 1.0f ),
    m_zoom( 0 ),
    m_fujin( 0 ),
    m_timeRemaining( 0.0f ),
    m_score( 0 ),
    m_gameOutTimer( 0.0f ),
    m_gameInTimer( 0.0f ),
    m_black( true ),
    m_shield( 0 ),
    m_overlay( 0 ),
    m_message( 0 ),
    m_channel( 0 ),
    m_timer( 0.0f )
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

    m_overlay = new hgeSprite( 0, 0, 0, 1, 1 );

    notifyOnCollision( true );

    Fujin::registerEntity();
    Bullet::registerEntity();
    Cloud::registerEntity();
    Girder::registerEntity();

    m_last_zoom = 1.0f;
    m_gameOutTimer = 0.0f;
    m_gameInTimer = 6.5f;
    m_zoom = 0;
    m_black = true;

    m_timeRemaining = 99;
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

    {
    Entity* entity = Engine::em()->factory( Cloud::TYPE );
    b2Vec2 position( -30.0f, -17.0f );
    float angle( Engine::hge()->Random_Float( -7.0f, 7.0f) );
    static_cast< Cloud * >( entity )->setSize( 2 );
    entity->setBlack( 1 );
    entity->setScale( 0.1f );
    entity->init();
    entity->getBody()->SetXForm( position, angle );
    }
    {
    Entity* entity = Engine::em()->factory( Cloud::TYPE );
    b2Vec2 position( 30.0f, -17.0f );
    float angle( Engine::hge()->Random_Float( -7.0f, 7.0f) );
    static_cast< Cloud * >( entity )->setSize( 2 );
    entity->setBlack( 0 );
    entity->setScale( 0.1f );
    entity->init();
    entity->getBody()->SetXForm( position, angle );
    }
    {
    Entity* entity = Engine::em()->factory( Cloud::TYPE );
    b2Vec2 position( 30.0f, 17.0f );
    float angle( Engine::hge()->Random_Float( -7.0f, 7.0f) );
    static_cast< Cloud * >( entity )->setSize( 2 );
    entity->setBlack( 1 );
    entity->setScale( 0.1f );
    entity->init();
    entity->getBody()->SetXForm( position, angle );
    }
    {
    Entity* entity = Engine::em()->factory( Cloud::TYPE );
    b2Vec2 position( -30.0f, 17.0f );
    float angle( Engine::hge()->Random_Float( -7.0f, 7.0f) );
    static_cast< Cloud * >( entity )->setSize( 2 );
    entity->setBlack( 0 );
    entity->setScale( 0.1f );
    entity->init();
    entity->getBody()->SetXForm( position, angle );
    }

    _initArena();

    Engine::hge()->Channel_StopAll();
    m_timer = 0.0f;
    HEFFECT music = rm->GetEffect( "game" );
    m_channel = Engine::hge()->Effect_PlayEx( music, 100, 0, 0, false );
}

//------------------------------------------------------------------------------
void
Game::fini()
{
    notifyOnCollision( false );

    delete m_overlay;
    m_overlay = 0;

    Engine::hge()->Channel_StopAll();
    Engine::em()->fini();
}

//------------------------------------------------------------------------------
bool
Game::update( float dt )
{
    const Controller & pad( Engine::instance()->getController() );
    HGE * hge( Engine::hge() );
    ViewPort * vp( Engine::vp() );

    bool paused( Engine::instance()->isPaused() );
    if ( paused && hge->Channel_IsPlaying( m_channel ) )
    {
        hge->Channel_Pause( m_channel );
    }
    else if ( ! paused && ! hge->Channel_IsPlaying( m_channel ) )
    {
        hge->Channel_Resume( m_channel );
    }

    m_timer = hge->Channel_GetPos( m_channel );

    if ( m_gameInTimer > 0.0f )
    {
        m_gameInTimer -= dt;
        m_message = 0;
        return false;
    }

    int score( m_fujin->getScore() );

    if ( m_gameOutTimer > 0.0f )
    {
        m_gameOutTimer -= dt;
        if ( m_gameOutTimer <= 0.0f )
        {
            if ( score == 0 )
            {
                Engine::instance()->switchContext( STATE_MENU );
            }
            else
            {
                if ( score >= 99 )
                {
                    score += static_cast< int >( m_timeRemaining );
                }
                Engine::instance()->switchContext( STATE_SCORE );
                Context * context( Engine::instance()->getContext() );
                static_cast< Score * >( context )->setValue( score );
            }
        }
        return false;
    }
    if ( m_shield != 0 )
    {
        m_shield->destroy();
        m_shield = 0;
    }

    m_timeRemaining -= dt;

    Engine::em()->update( dt );

    if ( Engine::instance()->isPaused() )
    {
        return false;
    }

    if ( m_fujin->isDestroyed() || score >= 99 || m_timeRemaining <= 0.0f )
    {
        if ( m_gameOutTimer <= 0.0f )
        {
            if (m_timer < 103.0f )
            {
                Engine::hge()->Channel_SetPos( m_channel, 105.0f );
            }
            m_gameOutTimer = 7.5f;
            if ( score >= 99 )
            {
                m_message = 3;
            }
            else if ( m_timeRemaining <= 0.0f )
            {
                m_message = 2;
            }
            else
            {
                m_message = 1;
            }
        }
        return false;
    }

    if ( pad.isConnected() )
    {
        if ( pad.buttonDown( XPAD_BUTTON_LEFT_SHOULDER ) ||
             pad.buttonDown( XPAD_BUTTON_RIGHT_SHOULDER ) ||
             pad.buttonDown( XPAD_BUTTON_LEFT_THUMB ) ||
             pad.buttonDown( XPAD_BUTTON_RIGHT_THUMB ) ||
             pad.buttonDown( XPAD_BUTTON_BUTTON_Y ) ||
             pad.buttonDown( XPAD_BUTTON_X ) ||
             pad.buttonDown( XPAD_BUTTON_B ) ||
             pad.buttonDown( XPAD_BUTTON_A ) )
        {
            m_black = ! m_black;
            m_fujin->setBlack( m_black );
            if ( m_black )
            {
                setColour( 0xFFFFFFFF );
            }
            else
            {
                setColour( 0xFF000000 );
            }
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
    char scoreText[64];
    sprintf_s(timeRemainingText,"%02.2f",m_timeRemaining);
    if ( m_fujin->getScore() == 0 )
    {
        sprintf_s( scoreText, "No Friends" ); 
    }
    else if ( m_fujin->getScore() == 1 )
    {
        sprintf_s( scoreText, "1 Friend" ); 
    }
    else
    {
        sprintf_s( scoreText, "%d Friends", m_fujin->getScore() ); 
    }

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

    /*
    font->printf( vp->screen().x * 0.5f, 10.0f, HGETEXT_CENTER, timeRemainingText );
    font->printf( vp->screen().x * 0.5f, vp->screen().y - 40.0f,
                  HGETEXT_CENTER, scoreText); 
                  */

    if ( ( m_gameInTimer > 0.0f || m_gameOutTimer > 0.0f ) &&
         ! Engine::instance()->isPaused() )
    {
        font->SetColor( 0xFFFFFFFF );
        m_overlay->SetColor( 0xFFFFFFFF );
        m_overlay->RenderStretch( 0.0f,
                                  vp->screen().y * 0.5f - 22.0f,
                                  vp->screen().x,
                                  vp->screen().y * 0.5f + 22.0f  );
        m_overlay->SetColor( 0xFF000000 );
        m_overlay->RenderStretch( 0.0f,
                                  vp->screen().y * 0.5f - 20.0f,
                                  vp->screen().x,
                                  vp->screen().y * 0.5f + 20.0f  );
        font->printf( vp->screen().x * 0.5f, vp->screen().y * 0.5f - 15.0f,
                      HGETEXT_CENTER, MESSAGE[m_message] );
    }

    vp->setTransform();
}

//------------------------------------------------------------------------------
bool
Game::shouldCollide( Entity * left, Entity * right )
{
    if ( m_gameOutTimer > 0.0f )
    {
        return false;
    }

    if ( left->getType() == Fujin::TYPE && right->getType() == Girder::TYPE )
    {
        return ! static_cast< Girder * >( right )->getShield();
    }
    if ( left->getType() == Girder::TYPE && right->getType() == Fujin::TYPE )
    {
        return ! static_cast< Girder * >( left )->getShield();
    }

    if ( left->getType() == Cloud::TYPE && right->getType() == Girder::TYPE )
    {
        return ! static_cast< Cloud * >( left )->getFriend();
    }
    if ( left->getType() == Girder::TYPE && right->getType() == Cloud::TYPE )
    {
        return ! static_cast< Cloud * >( right )->getFriend();
    }

    if ( left->getType() == Girder::TYPE || right->getType() == Girder::TYPE )
    {
        return true;
    }

    if ( left->getType() == Fujin::TYPE && right->getType() == Bullet::TYPE )
    {
        return false;
    }
    if ( left->getType() == Bullet::TYPE && right->getType() == Fujin::TYPE )
    {
        return false;
    }

    if ( left->getType() == Cloud::TYPE && right->getType() == Bullet::TYPE )
    {
        return ! static_cast< Cloud * >( left )->getFriend();
    }
    if ( left->getType() == Bullet::TYPE && right->getType() == Cloud::TYPE )
    {
        return ! static_cast< Cloud * >( right )->getFriend();
    }

    if ( left->getType() == Cloud::TYPE && right->getType() == Fujin::TYPE )
    {
        return ! static_cast< Cloud * >( left )->getFriend();
    }
    if ( left->getType() == Fujin::TYPE && right->getType() == Cloud::TYPE )
    {
        return ! static_cast< Cloud * >( right )->getFriend();
    }

    return true;
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
    dimensions.x = 5.0f;
    dimensions.y = 5.0f;
    m_shield = static_cast< Girder * >( Engine::em()->factory( Girder::TYPE ) );
    m_shield->setScale( 1.0f );
    m_shield->setDimensions( dimensions );
    m_shield->setShield( true );
    m_shield->init();
    position.x = 0.0f;
    position.y = 0.0f;
    m_shield->getBody()->SetXForm( position, 0.0f );
}
