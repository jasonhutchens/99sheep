//==============================================================================

#include <fujin.hpp>
#include <engine.hpp>
#include <entity_manager.hpp>
#include <cloud.hpp>
#include <bullet.hpp>

#include <hgeSprite.h>
#include <Box2D.h>
#include <sqlite3.h>
#include <Database.h>
#include <Query.h>
#include <hgeparticle.h>
#include <hgeresource.h>

namespace
{
    const float BLOW[5] = { 0.6f, 0.5f, 0.4f, 0.3f, 0.2f };
}

//==============================================================================
Fujin::Fujin( float max_strength, float scale )
    :
    Entity( scale ),
    Damageable( max_strength ),
    m_channel( 0 ),
    m_target_scale( 0.0f ),
    m_bullet_timer( 0.0f )
{
}

//------------------------------------------------------------------------------
Fujin::~Fujin()
{
    if ( m_channel != 0 )
    {
        Engine::instance()->hge()->Channel_Stop( m_channel );
        m_channel = 0;
    }
}

//------------------------------------------------------------------------------
void
Fujin::collide( Entity * entity, b2ContactPoint * point )
{
}

//------------------------------------------------------------------------------
void
Fujin::persistToDatabase()
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
Fujin::setTargetScale( float scale )
{
    m_target_scale = scale;
}

//------------------------------------------------------------------------------
// static:
//------------------------------------------------------------------------------
void
Fujin::registerEntity()
{
    Engine::em()->registerEntity( Fujin::TYPE, Fujin::factory, "fujins",
                                  "id, x, y, angle, scale, sprite_id" );
}

//------------------------------------------------------------------------------
// protected:
//------------------------------------------------------------------------------
void
Fujin::onSetScale()
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
    shapeDef.radius = 0.5f * 0.7f * m_sprite->GetWidth() * m_scale;
    shapeDef.density = 1.0f;
    shapeDef.friction =0.0f;
    shapeDef.restitution = 0.3f;
    m_body->CreateShape( & shapeDef );
    m_body->SetMassFromShapes();
}

//------------------------------------------------------------------------------
void
Fujin::doInit()
{
    b2BodyDef bodyDef;
    bodyDef.allowSleep = false;
    bodyDef.userData = static_cast< void * >( this );
    m_body = Engine::b2d()->CreateDynamicBody( & bodyDef );
	m_body->m_linearDamping = 0.8f;
    onSetScale();

    m_channel = 0;
    m_target_scale = 0.0f;
    m_zoom = 0;

	const Controller & pad( Engine::instance()->getController() );
	Engine::instance()->setMouse("cursor");
}

//------------------------------------------------------------------------------
void
Fujin::doUpdate( float dt )
{
    const Controller & pad( Engine::instance()->getController() );
	const Mouse &mouse(Engine::instance()->getMouse());
	
	const Mouse::MouseButton & leftMouseBtn(mouse.getLeft());

    b2Vec2 acceleration( 0.0f, 0.0f );
    b2Vec2 shoot( 0.0f, 0.0f );

    if ( Engine::instance()->isPaused() )
    {
		Engine::instance()->hideMouse();
    }
    else if ( pad.isConnected() )
    {
		Engine::instance()->hideMouse();

        acceleration = pad.getStick( XPAD_THUMBSTICK_LEFT );
        shoot = pad.getStick( XPAD_THUMBSTICK_RIGHT );
    }
	else
	{
		Engine::instance()->showMouse();

		if(Engine::hge()->Input_GetKeyState(HGEK_W))
		{
            acceleration.y += 1.0f;
		}
		if (Engine::hge()->Input_GetKeyState(HGEK_S))
		{
            acceleration.y -= 1.0f;
		}
		if (Engine::hge()->Input_GetKeyState(HGEK_A))
		{
            acceleration.x -= 1.0f;
		}
		if (Engine::hge()->Input_GetKeyState(HGEK_D))
		{
            acceleration.x += 1.0f;
		}

		b2Vec2 position (m_body->GetPosition());
		b2Vec2 mousePosition(mouse.getMousePos());
		b2Vec2 newPos = mousePosition - position;

        if ( Engine::hge()->Input_GetKeyState( HGEK_LBUTTON ) )
        {
            shoot = newPos;
            shoot.y *= -1.0f;
            shoot.Normalize();
        }

	}

    if ( Engine::instance()->getConfig().vibrate )
    {
        //Engine::instance()->getController().rumble( force, force, 0.1f );
    }

    m_bullet_timer -= dt;
    if ( m_bullet_timer < 0.0f )
    {
        m_bullet_timer = 0.0f;
    }
    if ( shoot.LengthSquared() > 0.2f && m_bullet_timer <= 0.0f )
    {
        m_bullet_timer = 0.015f;
        Bullet * bullet( static_cast< Bullet * >( Engine::em()->factory( Bullet::TYPE ) ) );
        if ( m_black )
        {
            bullet->setSprite( "black_bullet" );
        }
        else
        {
            bullet->setSprite( "white_bullet" );
        }
        bullet->setScale( m_scale );
        bullet->setBlack( m_black );
        bullet->init();
        b2Vec2 velocity( 0.0f, 0.0f );
        velocity = 80.0f * shoot;
        velocity.y *= -1.0f;
//      velocity += m_body->GetLinearVelocity();
        bullet->getBody()->SetLinearVelocity( velocity );
        b2Vec2 position( m_body->GetPosition() );
        bullet->getBody()->SetXForm( position, 0.0f );
    }

	bool dead( acceleration.LengthSquared() < 0.1f );
    acceleration.y *= -1.0f;
    b2Vec2 velocity( m_body->GetLinearVelocity() );
    float angle = lookAt( velocity );
    velocity = 40.0f * acceleration;
	if ( dead )
	{
	    velocity *= 0.0f;
	}
	m_body->SetAngularVelocity( 0.0f );
    m_body->SetLinearVelocity( velocity );

	b2Vec2 position( m_body->GetPosition() );
	b2Vec2 direction( 0.3f, 1.0f );
	direction = b2Mul( m_body->GetXForm().R, -direction );
	position = position + 64.0f * m_scale * direction;
	position = m_body->GetPosition() - 64.0f * m_scale * direction;
    angle = m_body->GetAngle();
		
    if ( getBlack() )
    {
        m_sprite = Engine::rm()->GetSprite( "black_ship" );
    }
    else
    {
        m_sprite = Engine::rm()->GetSprite( "white_ship" );
    }
}

//------------------------------------------------------------------------------
void
Fujin::doRender( float scale )
{
    b2Vec2 position( m_body->GetPosition() );
    float angle( m_body->GetAngle() );
    m_sprite->RenderEx( position.x, position.y, angle, m_scale );
    renderDamageable( position, m_scale );
}

//------------------------------------------------------------------------------
void
Fujin::initFromQuery( Query & query )
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
float Fujin::lookAt( const b2Vec2& targetPoint )
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
