/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// GUI Layout class
// Created 5/6/02
// Jason Boettcher

#include <assert.h>

#include "LieroX.h"
#include "AuxLib.h"
#include "Menu.h"
#include "StringUtils.h"




// XML parsing library
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


/*
// Useful XML functions
int		xmlGetInt(xmlNodePtr Node, const std::string& Name);
float	xmlGetFloat(xmlNodePtr Node, const std::string& Name);
Uint32	xmlGetColour(xmlNodePtr Node, const std::string& Name);
*/


// ==============================
//
// Useful XML functions
//
// ==============================

#define		CMP(str1,str2)  !xmlStrcmp((const xmlChar *)str1,(const xmlChar *)str2)


///////////////////
// Get an integer from the specified property
int xmlGetInt(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue;
	sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());
	if(!sValue)
		return 0;
	int result = atoi((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a float from the specified property
float xmlGetFloat(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());
	if (!sValue)
		return 0;
	float result = (float)atof((const char *)sValue);
	xmlFree(sValue);
	return result;
}

///////////////////
// Get a colour from the specified property
Uint32 xmlGetColour(xmlNodePtr Node, const std::string& Name)
{
	xmlChar *sValue;

	// Get the value
	sValue = xmlGetProp(Node,(const xmlChar *)Name.c_str());

	Uint32 result = StrToCol((char*)sValue);

	xmlFree(sValue);
	return result;
}







///////////////////
// Initialize the layout
void CGuiLayout::Initialize(int LayoutID)
{
	Shutdown();

	iID = LayoutID;

	cWidgets = NULL;
	cFocused = NULL;
	cMouseOverWidget = NULL;

	// Reset mouse repeats
	nMouseButtons = 0;
	for(int i=0; i<3; i++)
		fMouseNext[i] = -9999;

}


///////////////////
// Add a widget to the gui layout
void CGuiLayout::Add(CWidget *widget, int id, int x, int y, int w, int h)
{
	widget->Setup(id, x, y, w, h);
	widget->Create();
	widget->setParent(this);

	// Link the widget in
	widget->setPrev(NULL);
	widget->setNext(cWidgets);

	if(cWidgets)
		cWidgets->setPrev(widget);

	cWidgets = widget;
}


///////////////////
// Remove a widget
void CGuiLayout::removeWidget(int id)
{
    CWidget *w = getWidget(id);
    if( !w )
        return;

    // If this is the focused widget, set focused to null
    if(cFocused) {
        if(w->getID() == cFocused->getID())
            cFocused = NULL;
    }

    // Unlink the widget
    if( w->getPrev() )
        w->getPrev()->setNext( w->getNext() );
    else
        cWidgets = w->getNext();

    if( w->getNext() )
        w->getNext()->setPrev( w->getPrev() );

    // Free it
    w->Destroy();
	assert(w);
    delete w;
}


///////////////////
// Shutdown the gui layout
void CGuiLayout::Shutdown(void)
{
	CWidget *w,*wid;

	for(w=cWidgets ; w ; w=wid) {
		wid = w->getNext();

		w->Destroy();

		if(w)
			delete w;
	}
	cWidgets = NULL;

	// tEvent is freed in destructor

	cFocused = NULL;
	cMouseOverWidget = NULL;
}


///////////////////
// Draw the widgets
void CGuiLayout::Draw(SDL_Surface *bmpDest)
{
	CWidget *w, *end;

	// Draw the widgets in reverse order
	end = NULL;
	for(w=cWidgets ; w ; w=w->getNext()) {
		if(w->getNext() == NULL) {
			end = w;
			break;
		}
	}


	for(w=end ; w ; w=w->getPrev()) {
		if(w->getEnabled() && w)
			w->Draw(bmpDest);
	}
}

//////////////////
// Reads common events, that are available for almost every widget
void CGuiLayout_ReadEvents(CGuiLayout* gui, xmlNodePtr Node, generic_events_t *Events)
{
	// TODO: this function was a member function before; this is now gui
	
	// Load the values
	xmlChar *evs[NumEvents];
	evs[OnMouseOver] = xmlGetProp(Node,(const xmlChar *)"onmouseover");
	evs[OnMouseOut]  = xmlGetProp(Node,(const xmlChar *)"onmouseout");
	evs[OnMouseDown] = xmlGetProp(Node,(const xmlChar *)"onmousedown");
	evs[OnClick]	 = xmlGetProp(Node,(const xmlChar *)"onclick");

	// Copy the values into the events
	int i;
	for (i=0;i<NumEvents;i++)  {
		if (evs[i]) {
			fix_strncpy(Events->Events[i], (char *)evs[i]);
		} else
			Events->Events[i][0] = '\0';
	}

	// Free the data
	xmlFree(evs[OnMouseOver]);
	xmlFree(evs[OnMouseOut]);
	xmlFree(evs[OnMouseDown]);
	xmlFree(evs[OnClick]);
}

//////////////////
// Build the layout according to code specified in skin file
bool CGuiLayout::Build(void)
{

	//
	//	1. Get the file to parse
	//

	std::string sFilename = "";

	// Default skin extension
	std::string sExtension = "skn";

	// Get the skin path
	std::string path = tLXOptions->sSkinPath+tLXOptions->sResolution;

	// Temp
	static std::string file = "";

	// Get the file name of the skin file
	switch (iID)  {
		case L_MAINMENU: file = "mainmenu"; break;
		case L_LOCALPLAY: file = "localplay"; break;
		case L_GAMESETTINGS: file = "gamesettings"; break;
		case L_WEAPONOPTIONS: file = "weaponoptions"; break;
		case L_LOADWEAPONS: file = "loadweapons"; break;
		case L_SAVEWEAPONS: file = "saveweapons"; break;
		case L_NET: file = "net"; break;
		case L_NETINTERNET: file = "netinternet"; break;
		case L_INTERNETDETAILS: file = "internetdetails"; break;
		case L_ADDSERVER: file = "addserver"; break;
		case L_NETLAN: file = "netlan"; break;
		case L_LANDETAILS: file = "landetails"; break;
		case L_NETHOST: file = "nethost"; break;
		case L_NETFAVOURITES: file = "netfavourites"; break;
		case L_FAVOURITESDETAILS: file = "favouritesdetails"; break;
		case L_RENAMESERVER: file = "renameserver"; break;
		case L_ADDFAVOURITE: file = "addfavourite"; break;
		case L_CONNECTING: file = "connecting"; break;
		case L_NETJOINLOBBY: file = "netjoinlobby"; break;
		case L_NETHOSTLOBBY: file = "nethostlobby"; break;
		case L_SERVERSETTINGS: file = "serversettings"; break;
		case L_BANLIST: file = "banlist"; break;
		case L_PLAYERPROFILES: file = "playerprofiles"; break;
		case L_CREATEPLAYER: file = "createplayer"; break;
		case L_VIEWPLAYERS: file = "viewplayers"; break;
		case L_LEVELEDITOR: file = "leveleditor"; break;
		case L_NEWDIALOG: file = "newdialog"; break;
		case L_SAVELOADLEVEL: file = "saveloadlevel"; break;
		case L_OPTIONS: file = "options"; break;
		case L_OPTIONSCONTROLS: file = "optionscontrols"; break;
		case L_OPTIONSGAME: file = "optionsgame"; break;
		case L_OPTIONSSYSTEM: file = "optionssystem"; break;
		case L_MESSAGEBOXOK: file = "messageboxok"; break;
		case L_MESSAGEBOXYESNO: file = "messageboxyesno"; break;
		default: file =  "fuckingshit";
	}

	// Get the Filename + Path
	sFilename = path+"/"+file+"."+sExtension;

	//
	//	2. Parse the file
	//

	xmlDocPtr	tDocument;
	xmlNodePtr	tCurrentNode;

	// Parse the document
	tDocument = xmlParseFile(sFilename.c_str());
	if (tDocument == NULL)  {
		Error(ERR_COULDNOTPARSE,"Could not parse the document %s",sFilename.c_str());
		return false;
	}

	// Get the root node
	tCurrentNode = xmlDocGetRootElement(tDocument);
	if (tCurrentNode == NULL)  {
		Error(ERR_EMPTYDOC,"The '%s' document is empty",sFilename.c_str());
		return false;
	}

	// Validate the root node
	if (CMP(tCurrentNode->name,"skin"))  {
		Error(ERR_INVALIDROOT,"The document '%s' contains invalid parent node: %s",sFilename.c_str(),tCurrentNode->name);
		return false;
	}

	// Get the first child
	tCurrentNode = tCurrentNode->xmlChildrenNode;

	// Get all the nodes in document and create widgets in layout
	while (tCurrentNode != NULL)  {

		// Box
		if (CMP(tCurrentNode->name,"box"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			int round  = xmlGetInt(tCurrentNode,"round");
			int border = xmlGetInt(tCurrentNode,"border");
			Uint32 light_colour = xmlGetColour(tCurrentNode,"lightcolor");
			Uint32 dark_colour = xmlGetColour(tCurrentNode,"darkcolor");
			Uint32 bgcolour = xmlGetColour(tCurrentNode,"bgcolor");

			Add(new CBox(round,border,light_colour,dark_colour,bgcolour),-1,left,top,width,height);
		}

		// Image
		if (CMP(tCurrentNode->name,"img"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *src  = xmlGetProp(tCurrentNode,(const xmlChar *)"src");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the image
			CImage *Image = new CImage((char *) src);
			Add(Image,GetIdByName((char*)name),left,top,width,height);
			Image->SetupEvents(&Events);

			// Free resources
			xmlFree(name);
			xmlFree(src);

		}

		// Button
		if (CMP(tCurrentNode->name,"button"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *src	 = xmlGetProp(tCurrentNode,(const xmlChar *)"src");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the button
			CButton *Button = new CButton((char *)src);
			Add(Button,GetIdByName((char*)name),left,top,width,height);
			Button->SetupEvents(&Events);

			xmlFree(name);
			xmlFree(src);
		}


		// Checkbox
		if (CMP(tCurrentNode->name,"checkbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the checkbox
			CCheckbox *Checkbox = new CCheckbox(false);
			Add(Checkbox,GetIdByName((char*)name),left,top,width,height);
			Checkbox->SetupEvents(&Events);

			xmlFree(name);
		}


		// Combobox
		if (CMP(tCurrentNode->name,"combobox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the combobox
			CCombobox *Combobox = new CCombobox();
			Add(Combobox,GetIdByName((char*)name),left,top,width,height);
			Combobox->SetupEvents(&Events);

			xmlFree(name);
		}

		// Inputbox
		if (CMP(tCurrentNode->name,"inputbox"))  {
/*			 // TODO: not used
            int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height"); */
			xmlChar *name	 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *image	 = xmlGetProp(tCurrentNode,(const xmlChar *)"image");
			xmlChar *title	 = xmlGetProp(tCurrentNode,(const xmlChar *)"title");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			//CInputbox *Inputbox = new CInputbox(0,"",(char *)image,(char *)title);
			//Add(Inputbox,GetIdByName(name),left,top,width,height);
			//Inputbox->SetupEvents(&Events);

			xmlFree(name);
			xmlFree(image);
			xmlFree(title);
		}

		// Label
		if (CMP(tCurrentNode->name,"label"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			xmlChar *text = xmlGetProp(tCurrentNode,(const xmlChar *)"text");
			Uint32 colour = xmlGetColour(tCurrentNode,"color");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the label
			CLabel *Label = new CLabel((char *)text,colour);
			Add(Label,GetIdByName((char*)name),left,top,width,height);
			Label->SetupEvents(&Events);

			xmlFree(name);
			xmlFree(text);
		}

		// Listview
		if (CMP(tCurrentNode->name,"listview"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the listview
			CListview *Listview = new CListview();
			Add(Listview,GetIdByName((char*)name),left,top,width,height);
			Listview->SetupEvents(&Events);

			xmlFree(name);
		}

		// Scrollbar
		if (CMP(tCurrentNode->name,"scrollbar"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	= xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the scrollbar
			CScrollbar *Scrollbar = new CScrollbar();
			Add(Scrollbar,GetIdByName((char*)name),left,top,width,height);
			Scrollbar->SetupEvents(&Events);

			xmlFree(name);
		}

		// Slider
		if (CMP(tCurrentNode->name,"slider"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name		 = xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the slider
			CSlider *Slider = new CSlider(1);
			Add(Slider,GetIdByName((char*)name),left,top,width,height);
			Slider->SetupEvents(&Events);

			xmlFree(name);
		}

		// Textbox
		if (CMP(tCurrentNode->name,"checkbox"))  {
			int left   = xmlGetInt(tCurrentNode,"left");
			int top    = xmlGetInt(tCurrentNode,"top");
			int width  = xmlGetInt(tCurrentNode,"width");
			int height = xmlGetInt(tCurrentNode,"height");
			xmlChar *name	= xmlGetProp(tCurrentNode,(const xmlChar *)"name");
			generic_events_t Events;
			CGuiLayout_ReadEvents(this,tCurrentNode,&Events);

			// Add the textbox
			CTextbox *Textbox = new CTextbox();
			Add(Textbox,GetIdByName((char*)name),left,top,width,height);
			Textbox->SetupEvents(&Events);

			xmlFree(name);
		}


		tCurrentNode = tCurrentNode->next;
	}

	return true;
}


///////////////////
// Process all the widgets
gui_event_t *CGuiLayout::Process(void)
{
	CWidget *w;
	mouse_t *tMouse = GetMouse();
	SDL_Event *Event = GetEvent();
	int ev=-1;
	int widget = false;

	SetGameCursor(CURSOR_ARROW); // Reset the cursor here

	if (!tEvent)  {
		return NULL;
	}

	// Switch between window and fullscreen mode
	keyboard_t *Keyboard = GetKeyboard();
	if( cSwitchMode.isDown() )  {
		// Set to fullscreen
		tLXOptions->iFullscreen = !tLXOptions->iFullscreen;

		// Set the new video mode
		SetVideoMode();

		// Update both menu and game screens
		tMenu->bmpScreen = SDL_GetVideoSurface();

		// Redraw the mouse
		Menu_RedrawMouse(true);
	}

	// Put it here, so the mouse will never display
	SDL_ShowCursor(SDL_DISABLE);

	// Parse keyboard events to the focused widget
	// Make sure a key event happened
	if(Event->type == SDL_KEYUP || Event->type == SDL_KEYDOWN) {

		// If we don't have any focused widget, get the first textbox
		if (!cFocused)  {
			CWidget *txt = cWidgets;
			for (;txt;txt=txt->getNext())  {
				if (txt->getType() == wid_Textbox && txt->getEnabled()) {
					cFocused = txt;
					txt->setFocused(true);
					break;
				}
			}
		}


		if (cFocused)  {
			// Check the characters
			if(Event->key.state == SDL_PRESSED || Event->key.state == SDL_RELEASED) {
				tEvent->cWidget = cFocused;
				tEvent->iControlID = cFocused->getID();

				UnicodeChar input = Event->key.keysym.unicode;
				if (input == 0)
					switch (Event->key.keysym.sym) {
					case SDLK_LEFT:
					case SDLK_RIGHT:
					case SDLK_DELETE:
					case SDLK_HOME:
					case SDLK_END:
						input = Event->key.keysym.sym;
						break;
					case SDLK_KP0:
					case SDLK_KP1:
					case SDLK_KP2:
					case SDLK_KP3:
					case SDLK_KP4:
					case SDLK_KP5:
					case SDLK_KP6:
					case SDLK_KP7:
					case SDLK_KP8:
					case SDLK_KP9:
					case SDLK_KP_MULTIPLY:
					case SDLK_KP_MINUS:
					case SDLK_KP_PLUS:
					case SDLK_KP_EQUALS:
						input = (uchar) (Event->key.keysym.sym - 208);
						break;
					case SDLK_KP_PERIOD:
					case SDLK_KP_DIVIDE:
						input = (uchar) (Event->key.keysym.sym - 220);
						break;
					case SDLK_KP_ENTER:
						input = '\r';
						break;
                    default:
                        // TODO: much not handled keys; is this correct?
                        break;

				}  // switch


				if(Event->type == SDL_KEYUP || Event->key.state == SDL_RELEASED)
					ev = cFocused->KeyUp(input);

				// Handle more keys at once keydown
				if (Keyboard->queueLength > 1) 
					for(int i=0; i<Keyboard->queueLength; i++)
						if(!Keyboard->keyQueue[i].down || Keyboard->keyQueue[i].ch != input)  {
							ev = cFocused->KeyDown(Keyboard->keyQueue[i].ch);
							if (ev != -1)  {
								tEvent->iEventMsg = ev;
								return tEvent;
							}
						}

				// Keydown
				if(Event->type == SDL_KEYDOWN)  {
					ev = cFocused->KeyDown(input);
				}

				// Tab switches between widgets
				/*if (Keyboard->KeyUp[SDLK_TAB])  {
					if (cFocused)  {
						// The current one is not focused anymore
						cFocused->setFocused(false);

						// Switch to next widget
						if (cFocused->getNext())  {
							cFocused = cFocused->getNext();
							cFocused->setFocused(true);
						// The current focused widget is the last one in the list
						} else {
							cFocused = cWidgets;
							cFocused->setFocused(true);
						}
					} else {
						cFocused = cWidgets;
						cFocused->setFocused(true);
					}

					// Repeat the same thing until we find first enabled widget
					while (!cFocused->getEnabled())  {
						// The current one is not focused anymore
						cFocused->setFocused(false);

						if (cFocused->getNext())  {
							cFocused = cFocused->getNext();
							cFocused->setFocused(true);
						// The current focused widget is the last one in the list
						} else {
							cFocused = cWidgets;
							cFocused->setFocused(true);
						}
					}
				}*/

				if(ev >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}
		}
	}


	// Go through all the widgets
	for(w=cWidgets ; w ; w=w->getNext()) {
		tEvent->cWidget = w;
		tEvent->iControlID = w->getID();

		// Don't process disabled widgets
		if(!w->getEnabled())
			continue;

		// Special mouse button event first (for focused widgets)
		if(tMouse->Down && cFocused == w && !iCanFocus && !w->InBox(tMouse->X,tMouse->Y)) {
			widget = true;

			// Process the skin-defined code
			w->ProcessEvent(OnMouseDown);

			if( (ev=w->MouseDown(tMouse, tMouse->Down)) >= 0) {
				tEvent->iEventMsg = ev;
				return tEvent;
			}
		}


		if(w->InBox(tMouse->X,tMouse->Y)) {

			// Mouse wheel up
			if(tMouse->WheelScrollUp)  {
				widget = true;
				if(cFocused)
					cFocused->setFocused(false);
				w->setFocused(true);
				cFocused = w;

				if( (ev=w->MouseWheelUp(tMouse)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}

			// Mouse wheel down
			if(tMouse->WheelScrollDown)  {
				widget = true;
				if(cFocused)
					cFocused->setFocused(false);
				w->setFocused(true);
				cFocused = w;

				if( (ev=w->MouseWheelDown(tMouse)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}

			// Mouse button event first
			if(tMouse->Down) {

				widget = true;
				if (iCanFocus)  {
					if(cFocused)  {
						if (cFocused->CanLoseFocus())  {
							cFocused->setFocused(false);
							w->setFocused(true);
							cFocused = w;
							iCanFocus = false;
						}
					}
					else  {
						w->setFocused(true);
						cFocused = w;
						iCanFocus = false;
					}

				}

				// Process the skin defined code
				w->ProcessEvent(OnMouseDown);

				if( (ev=w->MouseDown(tMouse, tMouse->Down)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}

			// Mouse up event
			if(tMouse->Up) {
				iCanFocus = true;
				widget = true;
				if(cFocused)  {
					if(cFocused->CanLoseFocus())  {
						cFocused->setFocused(false);
						w->setFocused(true);
						cFocused = w;
					}
				}
				else  {
					w->setFocused(true);
					cFocused = w;
				}

				// Process the skin defined code
				w->ProcessEvent(OnClick);

				if( (ev=w->MouseUp(tMouse, tMouse->Up)) >= 0) {
					tEvent->iEventMsg = ev;
					return tEvent;
				}
			}


			// Mouse over
			if (w != cMouseOverWidget)  {
				w->ProcessEvent(OnMouseOver);

				// For the current Mouse over widget this means a mouse out event
				if(cMouseOverWidget)
					cMouseOverWidget->ProcessEvent(OnMouseOut);

				cMouseOverWidget = w;
			}

			if( (ev=w->MouseOver(tMouse)) >= 0) {
				tEvent->iEventMsg = ev;
				return tEvent;
			}

			// -2 - the widget says, that no event happened (various reasons)
			if (ev != -2)
				return NULL;
		}
	}

	// If mouse is over empty space it means, it's not over any widget ;-)
	if (cMouseOverWidget)  {
		cMouseOverWidget->ProcessEvent(OnMouseOut);
		cMouseOverWidget = NULL;
	}

	// If the mouse is clicked on empty space, take the focus of off the widget (if we can)
	if(!widget && (tMouse->Up)) {
		iCanFocus = true;
		if(cFocused)  {
			// Can we take the focus off?
			if (cFocused->CanLoseFocus())  {
				cFocused->setFocused(false);
				cFocused = NULL;
			}
			else  {
				cFocused->MouseUp(tMouse, tMouse->Up);
				cFocused->setLoseFocus(true);
			}
		}
		else  {
			cFocused = NULL;
		}

	}

	// Non-widget wheel up
	if(tMouse->WheelScrollUp)  {
		tEvent->iControlID = -9999;
		tEvent->iEventMsg = SDL_BUTTON_WHEELUP;
		return tEvent;
	}

	// Non-widget wheel down
	if(tMouse->WheelScrollDown)  {
		tEvent->iControlID = -9999;
		tEvent->iEventMsg = SDL_BUTTON_WHEELDOWN;
		return tEvent;
	}


	return NULL;
}


///////////////////
// Return a widget based on id
CWidget *CGuiLayout::getWidget(int id)
{
	CWidget *w;

	for(w=cWidgets ; w ; w=w->getNext()) {
		if(w->getID() == id)
			return w;
	}

	return NULL;
}

////////////////////
// Get the widget ID
int	CGuiLayout::GetIdByName(char *Name)
{
	int ID = -1;
	// Find the standard or previously added widget
	ID = LayoutWidgets[iID].getID((char *)Name);

	// Non-standard widget, add it to the list
	if (ID == -1)
		ID = LayoutWidgets[iID].Add((char *)Name);
	return ID;
}

////////////////////
// Notifies about the error that occured
void CGuiLayout::Error(int ErrorCode, char *Format, ...)
{
	static char buf[512];
	va_list	va;

	va_start(va,Format);
	vsnprintf(buf,sizeof(buf),Format,va);
	fix_markend(buf);
	va_end(va);

	// TODO: this better
	printf("%i: %s",ErrorCode,buf);
}

///////////////
// Set a property for all widgets
void CGuiLayout::SetGlobalProperty(int property, int value)
{
	CWidget *w = cWidgets;
# define FOREACH for (; w; w = w->getNext())

	// Set the property
	switch (property)  {
	case PRP_REDRAWMENU:
		FOREACH w->setRedrawMenu(value != 0);
		break;
	case PRP_ENABLED:
		FOREACH w->setEnabled(value != 0);
		break;
	case PRP_ID:
		FOREACH w->setID(value);
		break;
	default:
		printf("CGuiLayout::SetGlobalProperty: unknown property\n");
	}

#undef FOREACH
		
}


///////////////////
// Send a message to a widget
DWORD CGuiLayout::SendMessage(int iControl, int iMsg, DWORD Param1, DWORD Param2)
{
	CWidget *w = getWidget(iControl);

	// Couldn't find widget
	if(w == NULL)
		return 0;

	// Check if it's a widget message
	if(iMsg < 0) {
		switch( iMsg ) {

			// Set the enabled state of the widget
			case WDM_SETENABLE:
				w->setEnabled(Param1 != 0);
				break;
		}
		return 0;
	}

	return w->SendMessage(iMsg, Param1, Param2);
}

DWORD CGuiLayout::SendMessage(int iControl, int iMsg, const std::string& sStr, DWORD Param)
{
	CWidget *w = getWidget(iControl);

	// Couldn't find widget
	if(w == NULL)
		return 0;

	return w->SendMessage(iMsg, sStr, Param);

}

DWORD CGuiLayout::SendMessage(int iControl, int iMsg, std::string *sStr, DWORD Param)
{
	// Check the string
	if (!sStr)
		return 0;

	CWidget *w = getWidget(iControl);

	// Couldn't find widget
	if(w == NULL)
		return 0;

	return w->SendMessage(iMsg, sStr, Param);
}
