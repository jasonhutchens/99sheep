//==============================================================================

#include <game.hpp>
#include <engine.hpp>
#include <entity.hpp>
#include <entity_manager.hpp>
#include <clump_manager.hpp>
#include <viewport.hpp>
#include <fujin.hpp>
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
        if ( left->getType() < right->getType() )
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
    Cloud::registerEntity();
    Girder::registerEntity();

    m_last_zoom = 1.0f;
    m_gameOutTimer = 0;
    m_zoom = 0;

	m_timeRemaining = 300;
	m_score = 0;
    Engine::em()->init();
	Engine::cm()->init();

    vp->offset().x = 0.0f;
    vp->offset().y = 0.0f;
    vp->centre().x = 0.0f;
    vp->centre().y = 0.0f;
    vp->bounds().x = 1280.0f;
    vp->bounds().y = 720.0f;
    vp->setAngle( 0.0f );
    vp->setScale( ZOOM[m_zoom] );

    m_fujin = static_cast< Fujin * >( Engine::em()->factory( Fujin::TYPE ) );
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );
    m_fujin->setSprite( "white_ship" );
    m_fujin->setScale( FUJIN / ZOOM[m_zoom] );
    m_fujin->init();
    m_fujin->getBody()->SetXForm( position, angle );
    m_fujin->setTargetScale( FUJIN / ZOOM[m_zoom] );
    m_fujin->setZoom( m_zoom );

    setColour( 0xFFFFFFFF );
    m_fujin->setBlack( true );

    /*
    for ( int zoom = 0; zoom < 5; ++ zoom )
    {
	for (int i = 0; i < 8; ++i)
	{
		Entity* entity = Engine::em()->factory( Cloud::TYPE );
		b2Vec2 position( Engine::hge()->Random_Float( -400.0f, 400.0f),
                         Engine::hge()->Random_Float( -300.0f, 300.0f) );
		float angle( Engine::hge()->Random_Float( -7.0f, 7.0f) );
		entity->setSprite( "silver" );
		entity->setScale( 1.0f / ZOOM[zoom] );
		entity->init();
		entity->getBody()->SetXForm( position, angle );
        static_cast< Cloud * >( entity )->setZoom( zoom );
	}
    }
    */

    _initArena();
}

//------------------------------------------------------------------------------
void
Game::fini()
{
    notifyOnCollision( false );

    Engine::cm()->fini();
	Engine::em()->fini();
}

//------------------------------------------------------------------------------
bool
Game::update( float dt )
{
	m_timeRemaining -= dt;
    const Controller & pad( Engine::instance()->getController() );
    HGE * hge( Engine::hge() );
    ViewPort * vp( Engine::vp() );

    if ( m_gameOutTimer <= 0 && m_timeRemaining <=0)
    {
        Engine::instance()->switchContext( STATE_SCORE );
        Context * context( Engine::instance()->getContext() );
		static_cast< Score * >( context )->setValue( static_cast<int>(m_score * Engine::cm()->getClumpMultiplier()) );
        return false;
    }
	else if(m_timeRemaining <=0 &&  !m_fujin->isAsleep()  && m_gameOutTimer <=0)
	{
		m_fujin->setAsleep(true);
	}
	if(Engine::cm()->isTopClusterFull() && !m_fujin->isAsleep())
	{
		m_score += (int)(m_timeRemaining *10);
		if( m_timeRemaining > 20)
			m_timeRemaining = 20;
		m_fujin->setAsleep(true);
		m_gameOutTimer = 0;
		Engine::cm()->startClearingClump(m_timeRemaining - 1);
	}

	Engine::cm()->update( dt );
    Engine::em()->update( dt );

	updateProgressData();
	m_score += Engine::cm()->getClumpPoints();

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
               hge->Input_GetMouseWheel() < 0 ) && m_zoom > 0 )
        {
            --m_zoom;
            m_fujin->setTargetScale( FUJIN / ZOOM[m_zoom] );
            m_fujin->setZoom( m_zoom );
            hge->Effect_PlayEx( Engine::rm()->GetEffect( "up" ), 100 );
        }
        else if ( ( Engine::hge()->Input_KeyDown( HGEK_E ) ||
                    hge->Input_GetMouseWheel() > 0 ) && m_zoom < 4 )
        {
            ++m_zoom;
            m_fujin->setTargetScale( FUJIN / ZOOM[m_zoom] );
            m_fujin->setZoom( m_zoom );
            hge->Effect_PlayEx( Engine::rm()->GetEffect( "down" ), 100 );
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

	char scoreText[25];
	sprintf_s(scoreText,"Score: %5d",m_score);

	char multiplierText[25];
	sprintf_s(multiplierText,"Multiplier: x%2.02f",Engine::cm()->getClumpMultiplier());

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

	font->printf( vp->screen().x * 0.5f, vp->screen().y - 30.0f, HGETEXT_CENTER, timeRemainingText );

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
                dimensions.x = 1280.0f;
                dimensions.y = 1.0f;
                position.x = 0.0f;
                position.y = -361.0f;
                break;
            }
            case 1:
            {
                dimensions.x = 1.0f;
                dimensions.y = 720.0f;
                position.x = 641.0f;
                position.y = 0.0f;
                break;
            }
            case 2:
            {
                dimensions.x = 1280.0f;
                dimensions.y = 1.0f;
                position.x = 0.0f;
                position.y = 361.0f;
                break;
            }
            case 3:
            {
                dimensions.x = 1.0f;
                dimensions.y = 720.0f;
                position.x = -641.0f;
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

//------------------------------------------------------------------------------
void Game::updateProgressData()
{
	//This whole function is fairly horrible. Why not gloss over this and have
	//a look at something a bit nicer? :o)

	//get all the entities that are at the same level as the player
	std::vector< Cloud * > entities;
    for ( b2Body * body( Engine::b2d()->GetBodyList() ); body != NULL;
          body = body->GetNext() )
    {
        Entity * entity( static_cast< Entity * >( body->GetUserData() ) );
        if ( entity && entity->getType() == Cloud::TYPE && equal(entity, m_fujin))
        {
			Cloud* cloud =  static_cast<Cloud*>(entity);
			if (cloud->isInWorld())
				entities.push_back(cloud);
        }
	}

	int numClouds = entities.size();

	int looseEntities = 0;
	std::vector< Cloud * >::iterator i;
	std::set< Clump* > uniqueClumps;

	//find out all the entities that are not in a clump, and get a reference to
	//each clump that has some of these level entities in it
	for ( i = entities.begin(); i != entities.end(); ++i )
	{
		Clump* clump = (*i)->getClump();
		if (clump == NULL)
			++looseEntities;
		else
		{
			uniqueClumps.insert(clump);
		}
	}

	m_progress.clear();

	std::set< Clump * >::iterator iter;
	for (iter = uniqueClumps.begin(); iter != uniqueClumps.end(); ++iter)
	{
		Clump* clump = (*iter);
		m_progress.push_back(clump->getClouds()->size());
	}

	for (int i = 0; i < looseEntities; ++i)
	{
		m_progress.push_back(1);
	}

	//m_progress now will have a bunch of numbers in it, with each number being either 
	//the size of a clump, or a 1 for cloud not in a clump

	using namespace std;
	m_progress.sort( greater<int>( ) );		//sort by clump size
}
