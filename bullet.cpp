//==============================================================================

#include <bullet.hpp>
#include <engine.hpp>
#include <entity_manager.hpp>
#include <cloud.hpp>

#include <hgeSprite.h>
#include <Box2D.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>
#include <hgeparticle.h>
#include <hgeresource.h>

//==============================================================================
Bullet::Bullet( float max_strength, float scale )
    :
    Entity( scale ),
    Damageable( max_strength ),
    m_AABB(),
    m_channel( 0 ),
    m_target_scale( 0.0f ),
    m_lifetime( 0.0f )
{
}

//------------------------------------------------------------------------------
Bullet::~Bullet()
{
    if ( m_channel != 0 )
    {
        Engine::instance()->hge()->Channel_Stop( m_channel );
        m_channel = 0;
    }
}

//------------------------------------------------------------------------------
void
Bullet::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
Bullet::persistToDatabase()
{
    char * rows[] = { "x", "%f", "y", "%f", "angle", "%f", "scale", "%f",
                      "sprite_id", "%d" };
    m_id = Engine::em()->persistToDatabase( this, rows, m_body->GetPosition().x,
                                                        m_body->GetPosition().y,
                                                        m_body->GetAngle(),
                                                        m_scale, m_sprite_id );
}

//------------------------------------------------------------------------------
void
Bullet::setTargetScale( float scale )
{
    m_target_scale = scale;
}

//------------------------------------------------------------------------------
// static:
//------------------------------------------------------------------------------
void
Bullet::registerEntity()
{
    Engine::em()->registerEntity( Bullet::TYPE, Bullet::factory, "fujins",
                                  "id, x, y, angle, scale, sprite_id" );
}

//------------------------------------------------------------------------------
// protected:
//------------------------------------------------------------------------------
void
Bullet::onSetScale()
{
    if ( m_body == 0 )
    {
        return;
    }
    b2Shape * shape;
    while ( shape = m_body->GetShapeList() )
    {
        m_body->DestroyShape( shape );
    }
    b2CircleDef shapeDef;
    shapeDef.radius = 0.25f * 0.5f * 0.7f * m_sprite->GetWidth() * m_scale;
    shapeDef.density = 100.0f;
    shapeDef.friction =0.0f;
    shapeDef.restitution = 0.9f;
    m_body->CreateShape( & shapeDef );
    m_body->SetMassFromShapes();
}

//------------------------------------------------------------------------------
void
Bullet::doInit()
{
    b2BodyDef bodyDef;
    bodyDef.allowSleep = false;
    bodyDef.userData = static_cast< void * >( this );
    m_body = Engine::b2d()->CreateDynamicBody( & bodyDef );
	m_body->m_linearDamping = 0.2f;
	m_AABB.lowerBound= b2Vec2(-2.0f,-2.0f);
	m_AABB.upperBound= b2Vec2(2.0f,2.0f);
    onSetScale();

    m_channel = 0;
    m_target_scale = 0.0f;
    m_zoom = 0;

	const Controller & pad( Engine::instance()->getController() );
	Engine::instance()->setMouse("cursor");

    m_lifetime = 0.0f;
}

//------------------------------------------------------------------------------
void
Bullet::doUpdate( float dt )
{
    m_lifetime += dt;

    if ( m_lifetime > 2.0f )
    {
        destroy();
        return;
    }

    b2Vec2 velocity( m_body->GetLinearVelocity() );
    float angle = lookAt( velocity );

    if ( getBlack() )
    {
        m_sprite = Engine::rm()->GetSprite( "black_bullet" );
    }
    else
    {
        m_sprite = Engine::rm()->GetSprite( "white_bullet" );
    }
}

//------------------------------------------------------------------------------
void
Bullet::doRender( float scale )
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    m_sprite->RenderEx( position.x, position.y, angle, m_scale );
}

//------------------------------------------------------------------------------
void
Bullet::initFromQuery( Query & query )
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

//------------------------------------------------------------------------------
float Bullet::lookAt( const b2Vec2& targetPoint )
{
	b2Vec2 offset(targetPoint );
	float length( offset.Normalize() );
	float angle =0;
	if ( length > 0.2f )
	{
		b2Vec2 vertical( 0.0f, -1.0f );
		angle=( acosf( b2Dot( offset, vertical ) ) );
		if ( b2Cross( vertical, offset ) < 0.0f )
		{
			angle = -angle;
		}
		m_body->SetXForm( m_body->GetPosition(), angle );
	}
	return angle;
}

//==============================================================================
