/*  GSnull
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GS.h"
#include "GSWx.h"

#include "wx/wx.h"
#include "wx/glcanvas.h"

wxFrame *frame = NULL;
wxGLContext *context = NULL;
wxGLCanvas *glPane = NULL;

int GSOpenWindow(void *pDsp, char *Title)
{
    //printf("GSOPEN1 %p %s\n", pDsp, Title);
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    frame = new wxFrame((wxFrame*)NULL, -1, Title, wxPoint(50,50), wxSize(640,480));

    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

    glPane = new wxGLCanvas( frame, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE);
    sizer->Add(glPane, 1, wxEXPAND);

    context = new wxGLContext(glPane);

    frame->SetSizer(sizer);
    frame->SetAutoLayout(true);

    frame->Show();

	return 0;
}

int GSOpenWindow2(void *pDsp, u32 flags)
{
    //printf("GSOPEN2 %p %i\n", pDsp, flags);
	if (pDsp != NULL)
        frame = *(wxFrame**)pDsp;
	else
		return -1;

	return 0;
}

void GSCloseWindow()
{
    //printf("GSCLOSE3 %p %p %p\n", context, glPane, frame);
    if (context != NULL)
        delete context; context = NULL;
    if (glPane != NULL)
        delete glPane; glPane = NULL;
    if (frame != NULL)
        delete frame; frame = NULL;
}

void GSProcessMessages()
{

}


void HandleKeyEvent(keyEvent *ev)
{

}
