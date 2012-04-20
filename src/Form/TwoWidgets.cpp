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

#include "Form/TwoWidgets.hpp"

#include <algorithm>

TwoWidgets::~TwoWidgets()
{
  delete second;
  delete first;
}

void
TwoWidgets::UpdateLayout()
{
  const auto layout = CalculateLayout(rc);
  first->Move(layout.first);
  second->Move(layout.second);
}

PixelScalar
TwoWidgets::CalculateSplit(const PixelRect &rc) const
{
  const PixelSize min_a = first->GetMinimumSize();
  const PixelSize min_b = second->GetMinimumSize();
  const PixelSize max_b = second->GetMaximumSize();

  assert(min_b.cy <= max_b.cy);

  if (min_a.cy <= 0 || min_b.cy <= 0)
    /* at least one Widet doesn't know its minimums size; there may be
       better solutions for this, but this workaround is good enough
       for fixing the assertion failure in DevicesConfigPanel */
    return (rc.top + rc.bottom) / 2;
  else if (rc.bottom - rc.top >= min_a.cy + max_b.cy)
    /* more than enough space: align the second Widget at the bottom
       and give the rest to the first Widget */
    return rc.bottom - max_b.cy;
  else if (rc.bottom - rc.top >= min_a.cy + min_b.cy)
    /* still somewhat enough space */
    return rc.bottom - min_b.cy;
  else {
    /* give the best for the rest */
    const PixelScalar first_height =
      std::min(min_a.cy, PixelScalar((rc.bottom - rc.top) / 2));
    return rc.top + first_height;
  }
}

std::pair<PixelRect,PixelRect>
TwoWidgets::CalculateLayout(const PixelRect &rc) const
{
  PixelRect a = rc, b = rc;
  a.bottom = b.top = CalculateSplit(rc);
  return std::make_pair(a, b);
}

PixelSize
TwoWidgets::GetMinimumSize() const
{
  const PixelSize a = first->GetMinimumSize();
  const PixelSize b = second->GetMinimumSize();

  return PixelSize{ std::max(a.cx, b.cx), PixelScalar(a.cy + b.cy) };
}

PixelSize
TwoWidgets::GetMaximumSize() const
{
  const PixelSize a = first->GetMaximumSize();
  const PixelSize b = second->GetMaximumSize();

  return PixelSize{ std::max(a.cx, b.cx), PixelScalar(a.cy + b.cy) };
}

/**
 * Calculates a "dummy" layout that is splitted in the middle.  In
 * TwoWidgets::Initialise() and TwoWidgets::Prepare(), we are not
 * allowed to call Widget::GetMinimumSize() yet.
 */
gcc_const
static std::pair<PixelRect,PixelRect>
DummyLayout(const PixelRect rc)
{
  PixelRect a = rc, b = rc;
  a.bottom = b.top = (rc.top + rc.bottom) / 2;
  return std::make_pair(a, b);
}

void
TwoWidgets::Initialise(ContainerWindow &parent, const PixelRect &rc)
{
  this->rc = rc;
  const auto layout = DummyLayout(rc);
  first->Initialise(parent, layout.first);
  second->Initialise(parent, layout.second);
}

void
TwoWidgets::Prepare(ContainerWindow &parent, const PixelRect &rc)
{
  this->rc = rc;
  const auto layout = DummyLayout(rc);
  first->Prepare(parent, layout.first);
  second->Prepare(parent, layout.second);
}

void
TwoWidgets::Unprepare()
{
  first->Unprepare();
  second->Unprepare();
}

bool
TwoWidgets::Save(bool &changed, bool &require_restart)
{
  return first->Save(changed, require_restart) &&
    second->Save(changed, require_restart);
}

bool
TwoWidgets::Click()
{
  return first->Click() || second->Click();
}

void
TwoWidgets::ReClick()
{
  first->ReClick();
  second->ReClick();
}

void
TwoWidgets::Show(const PixelRect &rc)
{
  this->rc = rc;
  const auto layout = CalculateLayout(rc);
  first->Show(layout.first);
  second->Show(layout.second);
}

bool
TwoWidgets::Leave()
{
  return first->Leave() && second->Leave();
}

void
TwoWidgets::Hide()
{
  first->Hide();
  second->Hide();
}

void
TwoWidgets::Move(const PixelRect &rc)
{
  this->rc = rc;
  const auto layout = CalculateLayout(rc);
  first->Move(layout.first);
  second->Move(layout.second);
}

bool
TwoWidgets::SetFocus()
{
  return first->SetFocus() || second->SetFocus();
}
