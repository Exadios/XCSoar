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

#include "Logger/LoggerFRecord.hpp"
#include "DateTime.hpp"

/*
 * From FAI_Tech_Spec_Gnss.pdf 
 * 4.3 F RECORD - SATELLITE CONSTELLATION.
 * This is a mandatory record. However, there is no requirement to update the F-record at intervals of less than 5
 * minutes, so that transient changes of satellites received due to changing angles of bank, flying in valleys, etc do
 * not lead to frequent F-record lines. For the US GPS system, the satellite ID for each satellite is the PRN of the
 * satellite in question, for other satellite systems the ID will be assigned by GFAC as the need arises. Where
 * NMEA data is used within the FR, the ID should be taken from the GSA sentence that lists the IDs of those
 * satellites used in the fixes which are recorded in the B record. The F Record is not recorded continuously but at
 * the start of fixing and then only when a change in satellites used is detected. (AL4)
 */

/*
 * Interpretation:
 * Every logpoint, check if constellation has changed, and set flag if change is detected
 * every 4.5 minutes, if constellation has changed during the period
 * then log the new FRecord
 * Else, don't log it
 * Note: if a NAV Warning exists, we accelerate checking to every 30 seconds for valid constellation.
 * This is not required, but seems advantageous
 */

void
LoggerFRecord::Reset()
{
  update_needed = true;

  satellite_ids_available = false;
  std::fill_n(satellite_ids, GPSState::MAXSATELLITES, 0);

  clock.Reset(); // reset clock / timer
  clock.SetDT(fixed_one); // 1 sec so it appears at top of each file
}

bool
LoggerFRecord::Update(const GPSState &gps, fixed time, bool nav_warning)
{
  // Accelerate to 30 seconds if bad signal
  if (!gps.satellites_used_available || gps.satellites_used < 3 || nav_warning)
    clock.SetDT(fixed(ACCELERATED_UPDATE_TIME));
   
  // Check whether we still have satellite information
  bool available_changed =
      gps.satellite_ids_available != satellite_ids_available;

  // We need an update if
  // 1) the satellite information availability changed or
  // 2) satellite information is available and the IDs have changed
  update_needed = update_needed || available_changed ||
      (satellite_ids_available &&
       memcmp(gps.satellite_ids, satellite_ids, sizeof(satellite_ids)) != 0);

  // Check whether it's time for a new F record yet. Only if
  // 1) the last F record is a certain time ago and
  // 2) something has changed since then
  if (!clock.CheckAdvance(time) || !update_needed)
    return false;

  // Save the current satellite information for next time
  satellite_ids_available = gps.satellite_ids_available;
  if (satellite_ids_available)
    memcpy(satellite_ids, gps.satellite_ids, sizeof(satellite_ids));

  update_needed = false;

  clock.SetDT(fixed(DEFAULT_UPDATE_TIME));

  return true;
}
