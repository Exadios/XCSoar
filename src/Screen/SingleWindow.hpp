/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_SCREEN_SINGLE_WINDOW_HXX
#define XCSOAR_SCREEN_SINGLE_WINDOW_HXX

#include "Screen/TopWindow.hpp"

#include <stack>
#include <assert.h>

class WndForm;

/**
 * The single top-level window of an application.  When it is closed,
 * the process quits.
 */
class SingleWindow : public TopWindow {
protected:
  std::stack<WndForm *> dialogs;

public:
  void AddDialog(WndForm *dialog);
  void RemoveDialog(WndForm *dialog);

  /**
   * Forcefully cancel the top-most dialog.
   */
  void CancelDialog();

  bool HasDialog() {
    return !dialogs.empty();
  }

  /**
   * Check whether the specified dialog is the top-most one.
   */
  bool IsTopDialog(WndForm &dialog) {
    assert(HasDialog());

    return &dialog == dialogs.top();
  }

  WndForm &GetTopDialog() {
    assert(HasDialog());

    return *dialogs.top();
  }

#ifndef USE_GDI
protected:
  gcc_pure
  bool FilterMouseEvent(PixelScalar x, PixelScalar y, Window *allowed) const;
#endif

public:
  /**
   * Check if the specified event should be allowed.  An event may be
   * rejected when a modal dialog is active, and the event should go
   * to a window outside of the dialog.
   */
#ifdef ANDROID
  gcc_pure
  bool FilterEvent(const Event &event, Window *allowed) const;
#elif defined(ENABLE_SDL)
  gcc_pure
  bool FilterEvent(const SDL_Event &event, Window *allowed) const;
#else
  gcc_pure
  bool FilterEvent(const MSG &message, Window *allowed) const;
#endif

protected:
  virtual bool OnClose();
  virtual void OnDestroy();
};

#endif
