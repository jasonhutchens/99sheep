//==============================================================================

#include <tutorial.hpp>
#include <engine.hpp>
#include <entity.hpp>
#include <entity_manager.hpp>
#include <viewport.hpp>
#include <fujin.hpp>
#include <bullet.hpp>
#include <girder.hpp>

#include <hgeresource.h>

#include <algorithm>
#include <set>

namespace
{
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
}

//==============================================================================
Tutorial::Tutorial()
    :
    Context(),
    m_fujin( 0 )
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

    notifyOnCollision( true );

    Fujin::registerEntity();
    Bullet::registerEntity();
    Girder::registerEntity();

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
    m_fujin->setTargetScale( 0.8f );
    m_fujin->setZoom( 0 );

    m_fujin->setBlack( true );
    setColour( 0xFFFFFFFF );
    setBorder( 0xFF000000 );

    _initArena();
}

//------------------------------------------------------------------------------
void
Tutorial::fini()
{
    notifyOnCollision( false );
    Engine::instance()->getConfig().menu = 2;
    Engine::em()->fini();
}

//------------------------------------------------------------------------------
bool
Tutorial::update( float dt )
{
    const Controller & pad( Engine::instance()->getController() );
    HGE * hge( Engine::hge() );
    ViewPort * vp( Engine::vp() );

    Engine::em()->update( dt );

    if ( Engine::instance()->isPaused() )
    {
        return false;
    }

    vp->centre() = m_fujin->getBody()->GetPosition();

	// TODO: Cap'n speech bubble, click once to complete, twice to hide
	// TODO: Show sequence of instructions, change text depending on controls
	//		 "Welcome! It seems we're LOST. And I hear BLEATING!"
	//		 "Move with the LEFT STICK, or unplug for keyboard control"
	//		 "Use W A S D to move, or plug in a controller"
	//		 (when they move a certain distance, show the next one)
	//		 "Wow... look at the SIZE of that thing!"
	//		 "Shoot with the RIGHT STICK. Try killing that sheep!"
	//		 "Shoot with the LMB. Try killing that sheep!"
	//		 (when the sheep grows, show the next one)
	//	     "Hmmm... it EATS bullets of the same colour!"
	//		 "Perhaps you can kill it now that it's turned BLACK?"
	//		 (when it splits into 4, show the next one)
	//	     "Aha, I see! You can feed 'em or split 'em!"
	//		 "You can FLIP colours with the shoulder buttons."
	//       "You can FLIP colours with the RMB."
	//		 "Destroy them all!"
	//		 (when the player dies)
	//		 "Hmmm... it seems it's bad to COLLIDE with a sheep of OPPOSITE colour!"
	//		 (when a tiny sheep appears)
	//		 "Hmmm... perhaps try COLLECTING that tiny one, instead of KILLING it."
	//		 (when one is collected)
	//		 "Now I finally understand!"
	//	     "You must form a TAIL of 99 Sheep... in 99 Seconds!"
	//		 "Play the game by pressing the ESCAPE key, then selecting QUIT."
	//		 "Play the game by pressing START, then selecting QUIT."
	//		 (wait for the player)
	// TODO: Don't allow START until the tutorial is completed
	// TODO: Online high scores
	// TODO: Replace HUD with timer in full screen digits, and slider that moves left to right
	// TODO: Finish sounds
	// TODO: Particle effects (bullet hit, player die, expand, break)

    return false;
}

//------------------------------------------------------------------------------
void
Tutorial::render()
{
    hgeResourceManager * rm( Engine::rm() );
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
    float scale( 1.0f );
    for ( i = entities.begin(); i != entities.end(); ++i )
    {
        Entity * entity( * i );
        entity->render( scale );
    }
    Engine::vp()->reset();
    vp->setTransform();
}

//------------------------------------------------------------------------------
bool
Tutorial::shouldCollide( Entity * left, Entity * right )
{
    return ( left->getType() == Girder::TYPE ||
             right->getType() == Girder::TYPE );
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
Tutorial::_initArena()
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

//==============================================================================
