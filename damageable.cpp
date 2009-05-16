//==============================================================================

#include <damageable.hpp>

#include <hgesprite.h>
#include <Box2D.h>

//------------------------------------------------------------------------------
Damageable::Damageable( float strength )
    :
    m_max_strength( strength ),
    m_strength( strength ),
    m_damage( 0.0f ),
    m_timer( 0.0f ),
    m_bar( 0 )
{
    m_bar = new hgeSprite( 0, 0, 0, 1, 1 );
}

//------------------------------------------------------------------------------
Damageable::~Damageable()
{
    delete m_bar;
}

//------------------------------------------------------------------------------
void
Damageable::updateDamageable( float dt )
{
    m_timer -= dt;
    if ( m_timer < 0.0f )
    {
        m_timer = 0.0f;
    }

    if ( dt > 0.0f && m_damage > 0.0f )
    {
        float dd( m_damage * dt );
        m_strength -= dd;
        m_damage -= dd;
        if ( m_damage < 0.1f )
        {
            m_damage = 0.0f;
        }
        m_timer = 0.1f;
    }

    addStrength( 0.1f * dt );
}

//------------------------------------------------------------------------------
void
Damageable::renderDamageable( const b2Vec2 & position, float scale )
{
    if ( m_timer <= 0.0f || isDestroyed() )
    {
        return;
    }
    m_bar->SetColor( 0xBB000000 );
    float width( 40.0f );
    float height( 4.0f );
    float x1( position.x - 0.5f * width * scale );
    float y1( position.y - 0.5f * height * scale - 20.0f * scale );
    float x2( position.x + 0.5f * width * scale );
    float y2( position.y + 0.5f * height * scale - 20.0f * scale );
    m_bar->RenderStretch( x1, y1, x2, y2 );
    float ratio( m_strength / m_max_strength );
    m_bar->SetColor( 0xBB000000 +
                     ( static_cast<DWORD>( ratio * 255.0f ) << 8 ) +
                     ( static_cast<DWORD>( (1.0f - ratio)*255.0f ) << 16 ) );
    x1 = position.x - 0.5f * width * scale;
    y1 = position.y - 0.5f * height * scale - 20.0f * scale;
    x2 = position.x - 0.5f * width * scale + 40.0f * ratio * scale;
    y2 = position.y + 0.5f * height * scale - 20.0f * scale;
    m_bar->RenderStretch( x1, y1, x2, y2 );
}

//------------------------------------------------------------------------------
void
Damageable::addStrength( float amount )
{
    m_strength += amount;
    if ( m_strength > m_max_strength )
    {
        m_strength = m_max_strength;
        m_damage = 0.0f;
    }
}

//------------------------------------------------------------------------------
void
Damageable::takeDamage( float amount )
{
    if ( amount >= 0.1f )
    {
        m_damage += amount;
    }
}

//------------------------------------------------------------------------------
bool
Damageable::isDestroyed()
{
    return m_strength <= 0.0f;
}

//==============================================================================
