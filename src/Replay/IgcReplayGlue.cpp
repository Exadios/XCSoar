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

#include "Replay/IgcReplayGlue.hpp"
#include "Logger/Logger.hpp"
#include "Dialogs/Message.hpp"
#include "Language/Language.hpp"
#include "Blackboard/DeviceBlackboard.hpp"
#include "Components.hpp"

#include <algorithm>

IgcReplayGlue::IgcReplayGlue(Logger *_logger)
  :logger(_logger)
{
}

bool
IgcReplayGlue::UpdateTime()
{
  // Allow for poor time slicing, we never get called more
  // than 4 times per second, so this will yield 1 second updates
  if (!clock.Check(760))
    return false;

  t_simulation += time_scale * max(clock.Elapsed(), 0) / 1000;
  clock.Update();

  return true;
}

void
IgcReplayGlue::ResetTime()
{
  clock.Reset();
  t_simulation = fixed_zero;
}

void
IgcReplayGlue::OnAdvance(const GeoPoint &loc, const fixed speed,
                          const Angle bearing, const fixed alt,
                          const fixed baroalt, const fixed t)
{
  device_blackboard->SetLocation(loc, speed, bearing, alt, baroalt, t);
}

void
IgcReplayGlue::OnStop()
{
  device_blackboard->StopReplay();

  if (logger != NULL)
    logger->ClearBuffer();
}

void
IgcReplayGlue::OnBadFile()
{
  ShowMessageBox(_("Could not open IGC file!"),
              _("Flight replay"), MB_OK | MB_ICONINFORMATION);
}

void
IgcReplayGlue::OnReset()
{
  if (logger != NULL)
    logger->ClearBuffer();
}
