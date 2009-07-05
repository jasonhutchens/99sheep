//==============================================================================

#include <menu_item.hpp>

#include <hgeresource.h>
#include <hgefont.h>

//------------------------------------------------------------------------------
MenuItem::MenuItem( int control, float x, float y, const char * title,
                    hgeFont * font )
    :
    hgeGUIObject(),
    m_title( title ),
    m_font( font ),
    m_focused( false )
{
    id = control;

    bStatic = false;
    bVisible = true;
    bEnabled = true;

    float width( m_font->GetStringWidth( title ) );
    rect.Set( x - width / 2, y, x + width / 2, y + m_font->GetHeight() );
}
 
//------------------------------------------------------------------------------
void
MenuItem::Render()
{
    if ( m_focused )
    {
        m_font->SetColor( 0xFFFFFFFF );
    }
    else
    {
        m_font->SetColor( 0x55888888 );
    }
    m_font->Render( rect.x1, rect.y1, HGETEXT_LEFT, m_title );
}

//------------------------------------------------------------------------------
void
MenuItem::Update( float dt )
{
}

//------------------------------------------------------------------------------
void
MenuItem::Enter()
{
}

//------------------------------------------------------------------------------
void
MenuItem::Leave()
{
}

//------------------------------------------------------------------------------
bool
MenuItem::IsDone()
{
    return true;
}

//------------------------------------------------------------------------------
void
MenuItem::Focus( bool focused )
{
    m_focused = focused;
}

//------------------------------------------------------------------------------
void
MenuItem::MouseOver( bool over )
{
}

//------------------------------------------------------------------------------
bool
MenuItem::MouseLButton( bool down )
{
    return false;
}

//------------------------------------------------------------------------------
bool
MenuItem::KeyClick( int key, int chr )
{
    return ( key == HGEK_ENTER || key == HGEK_SPACE );
}

//==============================================================================
