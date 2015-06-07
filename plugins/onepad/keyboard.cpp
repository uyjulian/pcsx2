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

 /*
  * Theoretically, this header is for anything to do with keyboard input.
  * Pragmatically, event handing's going in here too.
  */

#include "keyboard.h"

static unsigned int  s_previous_mouse_x = 0;
static unsigned int  s_previous_mouse_y = 0;
void AnalyzeKeyEvent(int pad, keyEvent &evt)
{
	u32 key = evt.key;
	int index = get_keyboard_key(pad, key);

	switch (evt.evt)
	{
		case KeyPress:
			// Analog controls.
			if (IsAnalogKey(index))
			{
				switch (index)
				{
					case PAD_R_LEFT:
					case PAD_R_UP:
					case PAD_L_LEFT:
					case PAD_L_UP:
						key_status->press(pad, index, -MAX_ANALOG_VALUE);
						break;
					case PAD_R_RIGHT:
					case PAD_R_DOWN:
					case PAD_L_RIGHT:
					case PAD_L_DOWN:
						key_status->press(pad, index, MAX_ANALOG_VALUE);
						break;
				}
			} else if (index != -1)
				key_status->press(pad, index);

			//PAD_LOG("Key pressed:%d\n", index);

			event.evt = KEYPRESS;
			event.key = key;
			break;

		case KeyRelease:
			if (index != -1)
				key_status->release(pad, index);

			event.evt = KEYRELEASE;
			event.key = key;
			break;

		case ButtonPress:
			if (index != -1)
				key_status->press(pad, index);
			break;

		case ButtonRelease:
			if (index != -1)
				key_status->release(pad, index);
			break;

		case MotionNotify:
			// FIXME: How to handle when the mouse does not move, no event generated!!!
			// 1/ small move == no move. Cons : can not do small movement
			// 2/ use a watchdog timer thread
			// 3/ ??? idea welcome ;)
			if (conf->options & ((PADOPTION_MOUSE_L|PADOPTION_MOUSE_R) << 16 * pad )) {
				unsigned int pad_x;
				unsigned int pad_y;
				// Note when both PADOPTION_MOUSE_R and PADOPTION_MOUSE_L are set, take only the right one
				if (conf->options & (PADOPTION_MOUSE_R << 16 * pad)) {
					pad_x = PAD_R_RIGHT;
					pad_y = PAD_R_UP;
				} else {
					pad_x = PAD_L_RIGHT;
					pad_y = PAD_L_UP;
				}

				unsigned x = evt.key & 0xFFFF;
				unsigned int value = (s_previous_mouse_x > x) ? s_previous_mouse_x - x : x - s_previous_mouse_x;
				value *= conf->sensibility;

				if (x == 0)
					key_status->press(pad, pad_x, -MAX_ANALOG_VALUE);
				else if (x == 0xFFFF)
					key_status->press(pad, pad_x, MAX_ANALOG_VALUE);
				else if (x < (s_previous_mouse_x -2))
					key_status->press(pad, pad_x, -value);
				else if (x > (s_previous_mouse_x +2))
					key_status->press(pad, pad_x, value);
				else
					key_status->release(pad, pad_x);


				unsigned y = evt.key >> 16;
				value = (s_previous_mouse_y > y) ? s_previous_mouse_y - y : y - s_previous_mouse_y;
				value *= conf->sensibility;

				if (y == 0)
					key_status->press(pad, pad_y, -MAX_ANALOG_VALUE);
				else if (y == 0xFFFF)
					key_status->press(pad, pad_y, MAX_ANALOG_VALUE);
				else if (y < (s_previous_mouse_y -2))
					key_status->press(pad, pad_y, -value);
				else if (y > (s_previous_mouse_y +2))
					key_status->press(pad, pad_y, value);
				else
					key_status->release(pad, pad_y);

				s_previous_mouse_x = x;
				s_previous_mouse_y = y;
			}

			break;
	}
}

// TODO OSX move Windows specific implementations to Windows/platform.cpp
#ifndef __POSIX__
LRESULT WINAPI PADwndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool lbutton = false, rbutton = false;
	key_status->keyboard_state_acces(cpad);

	switch (msg)
	{
		case WM_KEYDOWN:
			if (lParam & 0x40000000) return TRUE;

			for (int pad = 0; pad < 2; ++pad)
			{
				for (int i = 0; i < MAX_KEYS; i++)
				{
					if (wParam ==  get_key(pad, i))
					{
						key_status->press(pad, i);
						break;
					}
				}
			}

			event.evt = KEYPRESS;
			event.key = wParam;
			break;

		case WM_KEYUP:
			for (int pad = 0; pad < 2; ++pad)
			{
				for (int i = 0; i < MAX_KEYS; i++)
				{
					if (wParam == get_key(pad,i))
					{
						key_status->release(pad, i);
						break;
					}
				}
			}


			event.evt = KEYRELEASE;
			event.key = wParam;
			break;

		case WM_DESTROY:
		case WM_QUIT:
			event.evt = KEYPRESS;
			event.key = VK_ESCAPE;
			return GSwndProc(hWnd, msg, wParam, lParam);

		default:
			return GSwndProc(hWnd, msg, wParam, lParam);
	}

	for (int pad = 0; pad < 2; ++pad)
		key_status->commit_status(pad);

	return TRUE;
}

char* KeysymToChar(int keysym)
{
	LPWORD temp;

	ToAscii((UINT) keysym, NULL, NULL, temp, NULL);
	return (char*)temp;
}
#endif
