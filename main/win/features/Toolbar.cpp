﻿#include "Toolbar.hpp"

#include <Windows.h>
#include <commctrl.h>

#include "plugin.hpp"
#include "rom.h"
#include "RomBrowser.hpp"
#include "../Config.hpp"
#include "../main_win.h"
#include "../../winproject/resource.h"
#include "helpers/io_helpers.h"

HWND toolbar_hwnd;

void toolbar_create()
{
	TBBUTTON tbButtons[] =
	{
		{
			0, IDLOAD, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,
			{0, 0}, 0, 0
		},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0, 0},
		{
			1, EMU_PLAY, TBSTATE_ENABLED, TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,
			{0, 0}, 0, 0
		},
		{
			2, EMU_PAUSE, TBSTATE_ENABLED, TBSTYLE_CHECK | TBSTYLE_AUTOSIZE,
			{0, 0}, 0, 0
		},
		{
			3, EMU_STOP, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,
			{0, 0}, 0, 0
		},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0, 0},
		{
			4, FULL_SCREEN, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,
			{0, 0}, 0, 0
		},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, {0, 0}, 0, 0},
		{
			5, IDGFXCONFIG, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,
			{0, 0}, 0, 0
		},
		{
			6, IDSOUNDCONFIG, TBSTATE_ENABLED,
			TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0, 0}, 0, 0
		},
		{
			7, IDINPUTCONFIG, TBSTATE_ENABLED,
			TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0, 0}, 0, 0
		},
		{
			8, IDRSPCONFIG, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE,
			{0, 0}, 0, 0
		},
		{
			9, ID_LOAD_CONFIG, TBSTATE_ENABLED,
			TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE, {0, 0}, 0, 0
		},
	};

	const auto tb_buttons_count = sizeof(tbButtons) / sizeof(TBBUTTON);
	toolbar_hwnd = CreateToolbarEx(mainHWND,
	                               WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS,
	                               IDC_TOOLBAR, 10, app_hInstance, IDB_TOOLBAR,
	                               tbButtons,
	                               static_cast<int>(tb_buttons_count), 16, 16,
	                               static_cast<int>(tb_buttons_count) * 16, 16,
	                               sizeof(TBBUTTON));

	if (toolbar_hwnd == nullptr)
		MessageBox(mainHWND, "Could not create tool bar.", "Error",
		           MB_OK | MB_ICONERROR);

	if (emu_launched)
	{
		if (emu_paused)
		{
			SendMessage(toolbar_hwnd, TB_CHECKBUTTON, EMU_PAUSE, 1);
		} else
		{
			SendMessage(toolbar_hwnd, TB_CHECKBUTTON, EMU_PLAY, 1);
		}
	} else
	{
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, EMU_STOP, FALSE);
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, EMU_PAUSE, FALSE);
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, FULL_SCREEN, FALSE);
	}
}

void toolbar_set_visibility(int32_t is_visible)
{
	if (toolbar_hwnd)
	{
		DestroyWindow(toolbar_hwnd);
		toolbar_hwnd = nullptr;
	}

	if (is_visible)
	{
		toolbar_create();
	}

	rombrowser_update_size();
}

void toolbar_on_emu_state_changed(int32_t is_running, int32_t is_resumed)
{
	if (!toolbar_hwnd) return;

	if (is_running)
	{
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, EMU_PLAY, TRUE);
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, EMU_STOP, TRUE);
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, EMU_PAUSE, TRUE);
		if (recording && !externalReadScreen)
			SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, FULL_SCREEN, FALSE);
		else
			SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, FULL_SCREEN, TRUE);

		SendMessage(toolbar_hwnd, TB_CHECKBUTTON, EMU_PAUSE, !is_resumed);
		SendMessage(toolbar_hwnd, TB_CHECKBUTTON, EMU_PLAY, is_resumed);
	} else
	{
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, EMU_STOP, FALSE);
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, EMU_PAUSE, FALSE);
		SendMessage(toolbar_hwnd, TB_ENABLEBUTTON, FULL_SCREEN, FALSE);
	}



}
