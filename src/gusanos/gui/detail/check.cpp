#include "check.h"

namespace OmfgGUI
{
	
LuaReference Check::metaTable;

bool Check::render()
{
	//Draw a flat, grey box with the text by default
	Renderer* renderer = context()->renderer();
	
	int h = getRect().getHeight();
	Rect r(getRect().x1, getRect().y1, getRect().x1 + h, getRect().y2); 
	
	if(m_formatting.background.skin)
	{
		renderer->drawSkinnedBox(*m_formatting.background.skin, r, m_formatting.background.color);
	}
	else
	{
		renderer->drawBox(
			r, m_formatting.background.color,
			m_formatting.borders[0].color,
			m_formatting.borders[1].color,
			m_formatting.borders[2].color,
			m_formatting.borders[3].color);
	}
		
	if(m_formatting.background.spriteSet)
	{
		renderer->drawSprite(*m_formatting.background.spriteSet, 0, r.centerX(), r.centerY());
	}
	
	if(m_font)
		renderer->drawText(*m_font, m_text, BaseFont::CenterV, getRect().x1 + h + 4, getRect().centerY(), m_formatting.fontColor);
	
	return true;
}

void Check::process()
{
}

void Check::toggleState()
{
	m_checked = !m_checked;
	Wnd::applyGSS();
}

void Check::applyGSS(Context::GSSselectors const& style)
{
	if(m_checked)
	{
		if(m_active)
			m_state = "active-checked";
		else if(isFocused())
			m_state = "focused-checked";
		else
			m_state = "checked";
		Wnd::applyGSSreally(style);
	}
	else
		Wnd::applyGSS(style);
}

}

