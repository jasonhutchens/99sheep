//==============================================================================

#include <cloud.hpp>
#include <bullet.hpp>
#include <engine.hpp>
#include <entity_manager.hpp>
#include <game.hpp>

#include <hgesprite.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>
#include <hgeparticle.h>
#include <hgeresource.h>

//------------------------------------------------------------------------------

namespace
{
    const char * WHITE [] = {
        "white_sheep_32",
        "white_sheep_64",
        "white_sheep_128",
        "white_sheep_256",
        "white_sheep_512",
    };
    const char * BLACK [] = {
        "black_sheep_32",
        "black_sheep_64",
        "black_sheep_128",
        "black_sheep_256",
        "black_sheep_512",
    };
    const float DAMAGE[] = {
        1.0f,
        20.0f,
        10.0f,
        5.0f,
        2.0f
    };
    const float SPEED[] = {
        20.0f,
        30.0f,
        20.0f,
        10.0f,
        5.0f
    };
};

//==============================================================================
Cloud::Cloud( float scale )
    :
    Entity( scale ),
    Damageable( 99.0f ),
    m_size( 0 ),
    m_friend( false ),
    m_life( 4.0f )
{
}

//------------------------------------------------------------------------------
Cloud::~Cloud()
{
}

//------------------------------------------------------------------------------
void
Cloud::collide( Entity * entity, b2ContactPoint * point )
{
    if ( getFriend() || entity->getType() != Bullet::TYPE )
    {
        return;
    }
    if ( entity->getBlack() != getBlack() || m_size == 0 )
    {
        takeDamage( DAMAGE[ m_size ] );
    }
    else if ( m_size > 0 && m_size < 4 )
    {
        addStrength( DAMAGE[ m_size + 1 ] );
    }
    entity->destroy();
}

//------------------------------------------------------------------------------
void
Cloud::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "angle", "%f", "scale", "%f",
                      "sprite_id", "%d" };
    m_id = Engine::em()->persistToDatabase( this, rows, m_body->GetPosition().x,
                                                        m_body->GetPosition().y,
                                                        m_body->GetAngle(),
                                                        m_scale, m_sprite_id );
}

//------------------------------------------------------------------------------
int
Cloud::getSize() const
{
    return m_size;
}

//------------------------------------------------------------------------------
void
Cloud::setSize( int size )
{
    m_size = size;
}

//------------------------------------------------------------------------------
bool
Cloud::isHarmful() const
{
    return m_life > 3.0f;
}

//------------------------------------------------------------------------------
bool
Cloud::getFriend() const
{
    return m_friend;
}

//------------------------------------------------------------------------------
void
Cloud::setFriend( bool b_friend )
{
    m_friend = b_friend;
}

//------------------------------------------------------------------------------
// static:
//------------------------------------------------------------------------------
void
Cloud::registerEntity()
{
    Engine::em()->registerEntity( Cloud::TYPE, Cloud::factory, "clouds",
                                  "id, x, y, angle, scale, sprite_id" );
}

//------------------------------------------------------------------------------
// protected:
//------------------------------------------------------------------------------
void
Cloud::doInit()
{
    if ( m_black )
    {
        m_sprite = Engine::rm()->GetSprite( BLACK[m_size] );
    }
    else
    {
        m_sprite = Engine::rm()->GetSprite( WHITE[m_size] );
    }

    m_zoom = 0;
    m_life = 4.0f;

	b2BodyDef bodyDef;
	bodyDef.userData = static_cast<void*> (this);
	m_body = Engine::b2d()->CreateDynamicBody(&bodyDef);

	b2CircleDef shapeDef;
	shapeDef.radius = 0.5f * 0.95f * m_sprite->GetWidth() * m_scale;
	shapeDef.density = 10.0f;
	shapeDef.friction = 0.0f;
	shapeDef.restitution = 1.0f;
    if ( m_size == 0 )
    {
	    shapeDef.density = 0.001f;
    }

	m_body->CreateShape(&shapeDef);
	m_body->SetMassFromShapes();
    m_body->m_linearDamping = 0.0f;
    if ( m_size == 0 )
    {
        m_body->m_linearDamping = 0.99f;
        m_body->m_angularDamping = 0.8f;
    }
    m_body->m_angularDamping = 0.0f;
    float spin( 5.0f / static_cast<float>(m_size + 1.0f) );
    m_body->SetAngularVelocity( Engine::hge()->Random_Float( -spin, spin ) );
    float speed( SPEED[m_size] );
    b2Vec2 velocity( Engine::hge()->Random_Float( -speed, speed ),
                     Engine::hge()->Random_Float( -speed, speed ) );
    m_body->SetLinearVelocity( velocity );
}

//------------------------------------------------------------------------------
void
Cloud::doUpdate( float dt )
{
    m_life += dt;
    if ( m_size == 0 )
    {
        setBlack( static_cast<Game *>( Engine::instance()->getContext() )->getBlack() );
    }
    updateDamageable( dt );
    if ( m_black )
    {
        m_sprite = Engine::rm()->GetSprite( BLACK[m_size] );
    }
    else
    {
        m_sprite = Engine::rm()->GetSprite( WHITE[m_size] );
    }
    if ( isDestroyed() )
    {
        if ( m_size > 0 )
        {
            for ( int i = 0; i < 3; ++i )
            {
                Entity* entity = Engine::em()->factory( Cloud::TYPE );
                if ( getBlack() )
                {
                    entity->setBlack( 0 );
                }
                else
                {
                    entity->setBlack( 1 );
                }
                entity->setScale( 0.1f );
                static_cast< Cloud * >( entity )->setSize( m_size - 1 );
                entity->init();
                entity->getBody()->SetXForm( m_body->GetPosition(),
                                             m_body->GetAngle() );
            }
        }
        destroy();
    }
    else if ( isHealthy() && m_size < 4 )
    {
        Entity* entity = Engine::em()->factory( Cloud::TYPE );
        if ( getBlack() )
        {
            entity->setBlack( 0 );
        }
        else
        {
            entity->setBlack( 1 );
        }
        entity->setScale( 0.1f );
        static_cast< Cloud * >( entity )->setSize( m_size + 1 );
        entity->init();
        entity->getBody()->SetXForm( m_body->GetPosition(),
                                     m_body->GetAngle() );
        destroy(); 
    }
    if ( getFriend() )
    {
        b2Vec2 velocity( 0.0f, 0.0f );
        m_body->SetAngularVelocity( 0.0f );
        m_body->SetLinearVelocity( velocity );
    }
}

//------------------------------------------------------------------------------
void
Cloud::doRender( float scale )
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    if ( m_life < 3.0f )
    {
        if ( static_cast<int>( m_life * 10 ) % 2 == 0 )
        {
            m_sprite->RenderEx( position.x, position.y, angle, m_scale );
        }
    }
    else
    {
        m_sprite->SetColor( 0xFFFFFFFF );
        m_sprite->RenderEx( position.x, position.y, angle, m_scale );
    }
}

//------------------------------------------------------------------------------
void
Cloud::initFromQuery( Query & query )
{
    b2Vec2 position( 0.0f, 0.0f );
    float angle( 0.0f );

    m_id = static_cast< sqlite_int64 >( query.getnum() );
    position.x = static_cast< float >( query.getnum() );
    position.y = static_cast< float >( query.getnum() );
    angle = static_cast< float >( query.getnum() );
    m_scale = static_cast< float >( query.getnum() );

    setSpriteID( static_cast< sqlite_int64 >( query.getnum() ) );

    init();

    m_body->SetXForm( position, angle );
}

//==============================================================================

