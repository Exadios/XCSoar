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
#include "NMEA/Info.hpp"

#include <stdio.h>
#include <string.h>

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
  last_f_record[0] = 0;
  detect_f_record_change=true;
  clock.Reset(); // reset clock / timer
  clock.SetDT(fixed_one); // 1 sec so it appears at top of each file
}

const char *
LoggerFRecord::Update(const GPSState &gps, const BrokenTime &broken_time,
                      fixed time, bool nav_warning)
{ 
  char f_record[sizeof(last_f_record)];
  
  sprintf(f_record,"F%02u%02u%02u",
          broken_time.hour, broken_time.minute, broken_time.second);
  unsigned length = 7;

  if (gps.satellite_ids_available) {
    for (unsigned i = 0; i < GPSState::MAXSATELLITES; ++i) {
      if (gps.satellite_ids[i] > 0){
        sprintf(f_record + length, "%02d", gps.satellite_ids[i]);
        length += 2;
      }
    }
  }

  detect_f_record_change = detect_f_record_change ||
    strcmp(f_record + 7, last_f_record + 7);
  
  if (!gps.satellites_used_available || gps.satellites_used < 3 || nav_warning)
    clock.SetDT(fixed(30)); // accelerate to 30 seconds if bad signal
   
  if (!clock.CheckAdvance(fixed(time)) || !detect_f_record_change)
    return NULL;

  strcpy(last_f_record, f_record);
  detect_f_record_change=false;
  clock.SetDT(fixed_270); //4.5 minutes
  return last_f_record;
}
