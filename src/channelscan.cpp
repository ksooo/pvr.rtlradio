//---------------------------------------------------------------------------
// Copyright (c) 2020 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------

#include "stdafx.h"
#include "channelscan.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// channelscan Constructor
//
// Arguments:
//
//	NONE

channelscan::channelscan() : kodi::gui::CWindow("channelscan.xml", "skin.estuary", true)
{
}

//---------------------------------------------------------------------------
// channelscan Destructor

channelscan::~channelscan()
{
}

//---------------------------------------------------------------------------
// channelscan::GetContextButtons (private)
//
// Get context menu buttons for list entry
//
// Arguments:
//
//	itemNumber	- Selected list item entry
//	buttons		- List of context menus to be added

void channelscan::GetContextButtons(int itemNumber, std::vector<std::pair<unsigned int, std::string>>& buttons)
{
	return kodi::gui::CWindow::GetContextButtons(itemNumber, buttons);
}

//---------------------------------------------------------------------------
// channelscan::OnAction (private)
//
// Receives action codes that are sent to this window
//
// Arguments:
//
//	actionId	- The action id to perform
//	buttoncode	- Button code associated with the action
//	unicode		- Unicode character associated with the action

bool channelscan::OnAction(int actionId, uint32_t buttoncode, wchar_t unicode)
{
	return kodi::gui::CWindow::OnAction(actionId, buttoncode, unicode);
}

//---------------------------------------------------------------------------
// channelscan::OnClick (private)
//
// Receives click event notifications for a control
//
//	controlId	- GUI control identifier

bool channelscan::OnClick(int controlId)
{
	return kodi::gui::CWindow::OnClick(controlId);
}

//---------------------------------------------------------------------------
// channelscan::OnContextButton (private)
//
// Called after selection in context menu
//
// Arguments:
//
//	itemNumber	- Selected list item entry
//	button		- The pressed button id

bool channelscan::OnContextButton(int itemNumber, unsigned int button)
{
	return kodi::gui::CWindow::OnContextButton(itemNumber, button);
}

//---------------------------------------------------------------------------
// channelscan::OnFocus (private)
//
// Receives focus event notifications for a control
//
// Arguments:
//
//	controlId		- GUI control identifier

bool channelscan::OnFocus(int controlId)
{
	return kodi::gui::CWindow::OnFocus(controlId);
}

//---------------------------------------------------------------------------
// channelscan::OnInit (private)
//
// Called to initialize the window object
//
// Arguments:
//
//	NONE

bool channelscan::OnInit(void)
{
	return kodi::gui::CWindow::OnInit();
}

//---------------------------------------------------------------------------

#pragma warning(pop)
