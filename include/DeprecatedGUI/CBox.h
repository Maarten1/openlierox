// OpenLieroX


// Image
// Created 29/10/06
// Dark Charlie

// code under LGPL


#ifndef __CBOX_H__DEPRECATED_GUI__
#define __CBOX_H__DEPRECATED_GUI__

#include "InputEvents.h"

namespace DeprecatedGUI {

// Event types
enum {
	BOX_NOEVENT=-2,
	BOX_NONE=-1
};


class CBox : public CWidget {
public:
	// Constructor
	CBox(int round,int border,Uint32 lightcolour,Uint32 darkcolour, Uint32 bgcolour) {
		iType = wid_Frame;
		iRound = round;
		iBorder = border;
		iLightColour = lightcolour;
		iDarkColour = darkcolour;
		iBgColour = bgcolour;
		bmpBuffer = NULL;
	}

private:
    // Attributes
	int		iRound;
	int		iBorder;
	Uint32	iLightColour;
	Uint32	iDarkColour;
	Uint32	iBgColour;

	SmartPointer<SDL_Surface> bmpBuffer;


public:
    // Methods

    void			Create()		{ iType = wid_Frame; PreDraw(); }
	void			Destroy();

	void			setRound(int round)			{ iRound = round; }
	int				getRound()				{ return iRound; }
	void			setBorder(int border)		{ iBorder = border; }
	int				getBorder()				{ return iBorder; }
	void			setLightColour(Uint32 col)	{ iLightColour = col; }
	Uint32			getLightColour()		{ return iLightColour; }
	void			setDarkColour(Uint32 col)	{ iDarkColour = col;  }
	Uint32			getDarkColour()			{ return iDarkColour; }
	void			setBgColour(Uint32 col)		{ iBgColour = col; }
	int				getBgColour()			{ return iBgColour; }

	//These events return an event id, otherwise they return -1
	int		MouseOver(mouse_t *tMouse)				{ return CheckEvent(); }
	int		MouseUp(mouse_t *tMouse, int nDown)		{ return CheckEvent(); }
	int		MouseDown(mouse_t *tMouse, int nDown)	{ return CheckEvent(); }
	int		MouseWheelDown(mouse_t *tMouse)			{ return CheckEvent(); }
	int		MouseWheelUp(mouse_t *tMouse)			{ return CheckEvent(); }
	int		KeyDown(UnicodeChar c, int keysym, const ModifiersState& modstate)		{ return BOX_NONE; }
	int		KeyUp(UnicodeChar c, int keysym, const ModifiersState& modstate)		{ return BOX_NONE; }

	DWORD SendMessage(int iMsg, DWORD Param1, DWORD Param2)	{ return 0; }
	DWORD SendMessage(int iMsg, const std::string& sStr, DWORD Param) { return 0; }
	DWORD SendMessage(int iMsg, std::string *sStr, DWORD Param)  { return 0; }

	int		CheckEvent();

	void	PreDraw();
	void	Draw(SDL_Surface * bmpDest);

	void	LoadStyle() {}

	static CWidget * WidgetCreator( const std::vector< ScriptVar_t > & p, CGuiLayoutBase * layout, int id, int x, int y, int dx, int dy )
	{
		CWidget * w = new CBox( p[0].i, p[1].i, p[2].c, p[3].c, p[4].c );
		layout->Add( w, id, x, y, dx, dy );
		return w;
	};
	
	void	ProcessGuiSkinEvent(int iEvent) {};
};

}; // namespace DeprecatedGUI

#endif  //  __CBOX_H__DEPRECATED_GUI__
