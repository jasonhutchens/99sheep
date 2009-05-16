//==============================================================================

#include <controller.hpp>

#pragma warning( disable : 4351 )

//------------------------------------------------------------------------------
Controller::Controller()
    :
    m_pad(),
    m_buttons(),
    m_stick(),
    m_duration( 0.0f )
{
}

//------------------------------------------------------------------------------
Controller::~Controller()
{
}

//------------------------------------------------------------------------------
void
Controller::clear()
{
    XenonPad()->UpdateState();
    m_pad = XenonPad()->GetState();
    for ( int i = 0; i < XPAD_NUM_BUTTONS; ++i )
    {
        m_buttons[i] = m_pad.button[i].isPressed;
    }
}

//------------------------------------------------------------------------------
void
Controller::init()
{
    XenonPad()->SetController( XPAD_CONTROLLER1 );
}

//------------------------------------------------------------------------------
void
Controller::fini()
{
    rumble( 0.0f, 0.0f, 0.0f );
    XenonPad()->Release();
}

//------------------------------------------------------------------------------
void
Controller::update( float dt )
{
    for ( int i = 0; i < static_cast<int>( XPAD_NUM_BUTTONS ); ++i )
    {
        m_buttons[i] = m_pad.button[i].isPressed;
    }
    XenonPad()->UpdateState();
    m_pad = XenonPad()->GetState();
    if ( m_duration > 0.0f )
    {
        m_duration -= dt;
        if ( m_duration <= 0.0f )
        {
            rumble( 0.0f, 0.0f, 0.0f );
        }
    }
}

//------------------------------------------------------------------------------
bool
Controller::isConnected() const
{
    return m_pad.bIsConnected;
}

//------------------------------------------------------------------------------
bool
Controller::buttonDown( XPad_GAMEPADLAYOUT button ) const
{
    return ! m_buttons[button] && m_pad.button[button].isPressed;
}

//------------------------------------------------------------------------------
bool
Controller::buttonUp( XPad_GAMEPADLAYOUT button ) const
{
    return m_buttons[button] && ! m_pad.button[button].isPressed;
}

//------------------------------------------------------------------------------
bool
Controller::getButtonState( XPad_GAMEPADLAYOUT button ) const
{
    return m_pad.button[button].isPressed;
}

//------------------------------------------------------------------------------
const b2Vec2 &
Controller::getStick( XPad_ANALOGUESTICK num ) const
{
    m_stick[num].x = m_pad.Stick[num].xAxis;
    m_stick[num].y = m_pad.Stick[num].yAxis;
    return m_stick[num];
}

//------------------------------------------------------------------------------
float
Controller::getTrigger( XPad_ANALOGUETRIGGER num ) const
{
    return m_pad.Trigger[num].Value;
}

//------------------------------------------------------------------------------
void
Controller::rumble( float leftSpeed, float rightSpeed, float duration ) const
{
    m_duration = duration;
    XenonPad()->SetForceFeedback( leftSpeed, rightSpeed );
}

//==============================================================================
