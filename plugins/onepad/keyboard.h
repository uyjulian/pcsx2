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

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "onepad.h"

// TODO OSX these are taken from X11/X.h. It would be nice to have these as PCSX2 specific defs...
#ifndef KeyPress
#define KeyPress                2
#define KeyRelease              3
#define ButtonPress             4
#define ButtonRelease           5
#define MotionNotify            6
#define EnterNotify             7
#define LeaveNotify             8
//#define FocusIn                 9
//#define FocusOut                10
//#define KeymapNotify            11
#endif

#ifdef __POSIX__
bool PollKeyboardMouseEvent(u32 &pkey);
void PollForKeyboardInput(int pad);
bool PlatformAnalyzeKeyEvent(keyEvent &evt);
void AnalyzeKeyEvent(int pad, keyEvent &evt);
void SetAutoRepeat(bool autorep);
const char* PlatformKeysymToString(int keysym);
void DefaultKeyboardValues();
string KeyName(int pad, int key, int keysym = 0);
#else
extern char* KeysymToChar(int keysym);
extern WNDPROC GSwndProc;
extern HWND GShwnd;
#endif

#endif //__KEYBOARD_H__
