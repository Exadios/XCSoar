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

#include "WindChartRenderer.hpp"
#include "ChartRenderer.hpp"
#include "Wind/WindStore.hpp"
#include "FlightStatistics.hpp"
#include "NMEA/Info.hpp"
#include "Math/LeastSquares.hpp"
#include "Units/Units.hpp"

void
RenderWindChart(Canvas &canvas, const PixelRect rc,
                const ChartLook &chart_look,
                const FlightStatistics &fs,
                const NMEAInfo &nmea_info,
                const WindStore &wind_store)
{
  int numsteps = 10;
  int i;
  fixed h;
  Vector wind;
  bool found = true;
  fixed mag;

  LeastSquares windstats_mag;
  ChartRenderer chart(chart_look, canvas, rc);

  if (fs.altitude_ceiling.y_max - fs.altitude_ceiling.y_min <= fixed_ten) {
    chart.DrawNoData();
    return;
  }

  for (i = 0; i < numsteps; i++) {
    h = fixed(fs.altitude_ceiling.y_max - fs.altitude_base.y_min) * i /
        (numsteps - 1) + fixed(fs.altitude_base.y_min);

    wind = wind_store.GetWind(nmea_info.time, h, found);
    mag = hypot(wind.x, wind.y);

    windstats_mag.LeastSquaresUpdate(mag, h);
  }

  chart.ScaleXFromData(windstats_mag);
  chart.ScaleXFromValue(fixed_zero);
  chart.ScaleXFromValue(fixed_ten);

  chart.ScaleYFromData(windstats_mag);

  chart.DrawXGrid(Units::ToSysSpeed(fixed(5)), fixed_zero,
                  ChartLook::STYLE_THINDASHPAPER, fixed(5), true);
  chart.DrawYGrid(Units::ToSysAltitude(fixed(1000)),
                  fixed_zero,
                  ChartLook::STYLE_THINDASHPAPER, fixed(1000), true);
  chart.DrawLineGraph(windstats_mag, ChartLook::STYLE_MEDIUMBLACK);

#define WINDVECTORMAG 25

  numsteps = (int)((rc.bottom - rc.top) / WINDVECTORMAG) - 1;

  // draw direction vectors
  fixed hfact;
  for (i = 0; i < numsteps; i++) {
    hfact = fixed(i + 1) / (numsteps + 1);
    h = fixed(fs.altitude_ceiling.y_max - fs.altitude_base.y_min) * hfact +
        fixed(fs.altitude_base.y_min);

    wind = wind_store.GetWind(nmea_info.time, h, found);
    if (windstats_mag.x_max == fixed_zero)
      windstats_mag.x_max = fixed_one; // prevent /0 problems
    wind.x /= fixed(windstats_mag.x_max);
    wind.y /= fixed(windstats_mag.x_max);
    mag = hypot(wind.x, wind.y);
    if (negative(mag))
      continue;

    Angle angle = Angle::Radians(atan2(-wind.x, wind.y));

    chart.DrawArrow((chart.GetXMin() + chart.GetXMax()) / 2, h,
                    mag * WINDVECTORMAG, angle, ChartLook::STYLE_MEDIUMBLACK);
  }

  chart.DrawXLabel(_T("w"), Units::GetSpeedName());
  chart.DrawYLabel(_T("h"), Units::GetAltitudeName());
}
