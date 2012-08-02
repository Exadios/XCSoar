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

#include "AirspaceFormatter.hpp"

#include "Engine/Airspace/AbstractAirspace.hpp"
#include "Engine/Airspace/AirspaceAltitude.hpp"
#include "Util/StaticString.hpp"
#include "Units/Units.hpp"

tstring
AirspaceFormatter::GetAltitudeShort(const AirspaceAltitude &altitude)
{
  StaticString<64> buffer;

  switch (altitude.type) {
  case AirspaceAltitude::Type::AGL:
    if (!positive(altitude.altitude_above_terrain))
      buffer = _T("GND");
    else
      buffer.Format(_T("%d %s AGL"),
                    iround(Units::ToUserAltitude(altitude.altitude_above_terrain)),
                    Units::GetAltitudeName());
    break;
  case AirspaceAltitude::Type::FL:
    buffer.Format(_T("FL%d"), (int)altitude.flight_level);
    break;
  case AirspaceAltitude::Type::MSL:
    buffer.Format(_T("%d %s"), iround(Units::ToUserAltitude(altitude.altitude)),
                  Units::GetAltitudeName());
    break;
  case AirspaceAltitude::Type::UNDEFINED:
    buffer.clear();
    break;
  }

  return tstring(buffer);
}

tstring
AirspaceFormatter::GetAltitude(const AirspaceAltitude &altitude)
{
  StaticString<64> buffer;
  if (altitude.type != AirspaceAltitude::Type::MSL && positive(altitude.altitude))
    buffer.Format(_T(" %d %s"), iround(Units::ToUserAltitude(altitude.altitude)),
                        Units::GetAltitudeName());
  else
    buffer.clear();

  return GetAltitudeShort(altitude) + buffer.c_str();
}

tstring
AirspaceFormatter::GetBase(const AbstractAirspace &airspace)
{
  return GetAltitude(airspace.GetBase());
}

tstring
AirspaceFormatter::GetBaseShort(const AbstractAirspace &airspace)
{
  return GetAltitudeShort(airspace.GetBase());
}

tstring
AirspaceFormatter::GetTop(const AbstractAirspace &airspace)
{
  return GetAltitude(airspace.GetTop());
}

tstring
AirspaceFormatter::GetTopShort(const AbstractAirspace &airspace)
{
  return GetAltitudeShort(airspace.GetTop());
}
