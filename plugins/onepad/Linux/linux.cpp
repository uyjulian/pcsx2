/*  OnePAD - author: arcum42(@gmail.com)
 *  Copyright (C) 2009
 *
 *  Based on ZeroPAD, author zerofrog@gmail.com
 *  Copyright (C) 2006-2007
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "joystick.h"
#include "onepad.h"
#include "keyboard.h"

#include <string.h>
#include "PS2Eext.h"
#include "linux.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

Display *GSdsp;
Window	GSwin;
extern bool mutex_WasInit;

s32  _PADopen(void *pDsp)
{
    SPININIT(mutex_KeyEvent);
    mutex_WasInit = true;

	GSdsp = *(Display**)pDsp;
	GSwin = (Window)*(((u32*)pDsp)+1);

    SetAutoRepeat(false);
	return 0;
}

void _PADclose()
{
    mutex_WasInit = false;
    SPINDESTROY(mutex_KeyEvent);

	SetAutoRepeat(true);

	vector<JoystickInfo*>::iterator it = s_vjoysticks.begin();

	// Delete everything in the vector vjoysticks.
	while (it != s_vjoysticks.end())
	{
		delete *it;
		it ++;
	}

	s_vjoysticks.clear();
}

EXPORT_C_(void) PADWriteEvent(keyEvent &evt)
{
    // This function call be called before PADopen. Therefore we cann't
    // guarantee that the spin lock was initialized
    if (mutex_WasInit) {
        pthread_spin_lock(&mutex_KeyEvent);
        ev_fifo.push(evt);
        pthread_spin_unlock(&mutex_KeyEvent);
    }
}

EXPORT_C_(void) PADupdate(int pad)
{
    // Actually PADupdate is always call with pad == 0. So you need to update both
    // pads -- Gregory
    for (int cpad = 0; cpad < 2; cpad++) {
        // Poll keyboard/mouse event
        key_status->keyboard_state_acces(cpad);
        PollForKeyboardInput(cpad);

        // Get joystick state
        key_status->joystick_state_acces(cpad);
        PollForJoystickInput(cpad);

        key_status->commit_status(cpad);
    }
}

EXPORT_C_(void) PADabout()
{
    SysMessage("OnePad is a rewrite of Zerofrog's ZeroPad, done by arcum42.");
}

void PollForKeyboardInput(int pad)
{
    keyEvent evt;
    XEvent E;

    // Keyboard input send by PCSX2
    while (!ev_fifo.empty()) {
        SPINLOCK(&mutex_KeyEvent);

        evt = ev_fifo.front();
        // First platform specific functionality. End if handled.
        if (PlatformAnalyzeKeyEvent(evt))
            AnalyzeKeyEvent(pad, evt);
        ev_fifo.pop();

        SPINUNLOCK(&mutex_KeyEvent);
    }

    // keyboard input
    while (XPending(GSdsp) > 0)
    {
        XNextEvent(GSdsp, &E);

        // Change the format of the structure to be compatible with GSOpen2
        // mode (event come from pcsx2 not X)
        evt.evt = E.type;
        switch (E.type) {
            case MotionNotify:
                evt.key = (E.xbutton.x & 0xFFFF) | (E.xbutton.y << 16);
                break;
            case ButtonRelease:
            case ButtonPress:
                evt.key = E.xbutton.button;
                break;
            default:
                evt.key = (int)XLookupKeysym(&E.xkey, 0);
        }

        AnalyzeKeyEvent(pad, evt);
    }
}

bool s_grab_input = false;
bool s_Shift = false;

// Returns false if completely handled within this function and no further analyze is necessary.
bool PlatformAnalyzeKeyEvent(keyEvent &evt)
{
    KeySym key = (KeySym)evt.key;
    switch (evt.evt)
    {
        case KeyPress:
            // Shift F12 is not yet use by pcsx2. So keep it to grab/ungrab input
            // I found it very handy vs the automatic fullscreen detection
            // 1/ Does not need to detect full-screen
            // 2/ Can use a debugger in full-screen
            // 3/ Can grab input in window without the need of a pixelated full-screen
            if (key == XK_Shift_R || key == XK_Shift_L) s_Shift = true;
            if (key == XK_F12 && s_Shift) {
                if(!s_grab_input) {
                    s_grab_input = true;
                    XGrabPointer(GSdsp, GSwin, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, GSwin, None, CurrentTime);
                    XGrabKeyboard(GSdsp, GSwin, True, GrabModeAsync, GrabModeAsync, CurrentTime);
                } else {
                    s_grab_input = false;
                    XUngrabPointer(GSdsp, CurrentTime);
                    XUngrabKeyboard(GSdsp, CurrentTime);
                }
            }
            break;

        case KeyRelease:
            if (key == XK_Shift_R || key == XK_Shift_L) s_Shift = false;
            break;

        case FocusIn:
            XAutoRepeatOff(GSdsp);
            break;

        case FocusOut:
            XAutoRepeatOn(GSdsp);
            break;
    }

    return true;
}

bool PollKeyboardMouseEvent(u32 &pkey)
{
    GdkEvent *ev = gdk_event_get();

    if (ev != NULL)
    {
        if (ev->type == GDK_KEY_PRESS) {
            pkey = ev->key.keyval != GDK_KEY_Escape ? ev->key.keyval : 0;
            return true;
        } else if(ev->type == GDK_BUTTON_PRESS) {
            pkey = ev->button.button;
            return true;
        }
    }

    return false;
}

void SetAutoRepeat(bool autorep)
{
    if (toggleAutoRepeat)
    {
        if (autorep)
            XAutoRepeatOn(GSdsp);
        else
            XAutoRepeatOff(GSdsp);
    }
}

void DefaultKeyboardValues()
{
    set_keyboad_key(0, XK_a, PAD_L2);
    set_keyboad_key(0, XK_semicolon, PAD_R2);
    set_keyboad_key(0, XK_w, PAD_L1);
    set_keyboad_key(0, XK_p, PAD_R1);
    set_keyboad_key(0, XK_i, PAD_TRIANGLE);
    set_keyboad_key(0, XK_l, PAD_CIRCLE);
    set_keyboad_key(0, XK_k, PAD_CROSS);
    set_keyboad_key(0, XK_j, PAD_SQUARE);
    set_keyboad_key(0, XK_v, PAD_SELECT);
    set_keyboad_key(0, XK_n, PAD_START);
    set_keyboad_key(0, XK_e, PAD_UP);
    set_keyboad_key(0, XK_f, PAD_RIGHT);
    set_keyboad_key(0, XK_d, PAD_DOWN);
    set_keyboad_key(0, XK_s, PAD_LEFT);
}

const char* PlatformKeysymToString(int keysym)
{
    return XKeysymToString(keysym);
}
