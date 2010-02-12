/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009

	M Roberts (original release)
	Robin Birch <robinb@ruffnready.co.uk>
	Samuel Gisiger <samuel.gisiger@triadis.ch>
	Jeff Goodenough <jeff@enborne.f2s.com>
	Alastair Harrison <aharrison@magic.force9.co.uk>
	Scott Penrose <scottp@dd.com.au>
	John Wharington <jwharington@gmail.com>
	Lars H <lars_hn@hotmail.com>
	Rob Dunning <rob@raspberryridgesheepfarm.com>
	Russell King <rmk@arm.linux.org.uk>
	Paolo Ventafridda <coolwind@email.it>
	Tobias Lohner <tobias@lohner-net.de>
	Mirek Jezek <mjezek@ipplc.cz>
	Max Kellermann <max@duempel.org>
	Tobias Bieniek <tobias.bieniek@gmx.de>

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

#include "WayPointParser.h"
#include "DeviceBlackboard.hpp"
#include "Dialogs.h"
#include "Language.hpp"
#include "Registry.hpp"
#include "Profile.hpp"
#include "LocalPath.hpp"
#include "UtilsText.hpp"
#include "StringUtil.hpp"
#include "RasterTerrain.h"
#include "RasterMap.h"
#include "LogFile.hpp"
#include "Waypoint/Waypoints.hpp"
#include "UtilsFile.hpp"
#include "Units.hpp"

#include <algorithm>
using std::min;
using std::max;

#include <tchar.h>
#include <stdio.h>

#ifdef HAVE_POSIX
#include <errno.h>
#endif

#include "wcecompat/ts_string.h"

int WayPointParser::WaypointsOutOfRangeSetting = 0;

/**
 * This functions checks if the home, alternate 1/2 and teamcode waypoint
 * indices exist and if necessary tries to find new ones in the waypoint list
 * @param way_points Waypoint list
 * @param terrain RasterTerrain (for placing the aircraft
 * in the middle of the terrain if no home was found)
 * @param settings SETTING_COMPUTER (for determining the
 * special waypoint indices)
 * @param reset This should be true if the waypoint file was changed,
 * it resets all special waypoints indices
 * @param set_location If true, the SetStartupLocation function will be called
 */
void
SetHome(const Waypoints &way_points, const RasterTerrain *terrain,
        SETTINGS_COMPUTER &settings,
        const bool reset, const bool set_location)
{
  StartupStore(TEXT("SetHome\n"));

  // check invalid home waypoint or forced reset due to file change
  if (reset || way_points.empty() ||
      !way_points.lookup_id(settings.HomeWaypoint)) {
    // search for home in waypoint list, if we don't have a home
    settings.HomeWaypoint = -1;

    const Waypoint* wp = way_points.find_home();
    if (wp)
      settings.HomeWaypoint = wp->id;
  }

  // reset Alternates
  if (reset || way_points.empty() ||
      !way_points.lookup_id(settings.Alternate1) ||
      !way_points.lookup_id(settings.Alternate2)) {
    settings.Alternate1 = -1;
    settings.Alternate2 = -1;
  }

  // check invalid task ref waypoint or forced reset due to file change
  if (reset || way_points.empty() ||
      !way_points.lookup_id(settings.TeamCodeRefWaypoint))
    // set team code reference waypoint if we don't have one
    settings.TeamCodeRefWaypoint = settings.HomeWaypoint;

  if (set_location) {
    if (const Waypoint *wp = way_points.lookup_id(settings.HomeWaypoint)) {
      // OK, passed all checks now
      StartupStore(TEXT("Start at home waypoint\n"));
      device_blackboard.SetStartupLocation(wp->Location, wp->Altitude);
    } else if (terrain != NULL) {
      // no home at all, so set it from center of terrain if available
      GEOPOINT loc;
      if (terrain->GetTerrainCenter(&loc)) {
        StartupStore(TEXT("Start at terrain center\n"));
        device_blackboard.SetStartupLocation(loc, 0);
      }
    }
  }

  //
  // Save the home waypoint number in the resgistry
  //
  // VENTA3> this is probably useless, since HomeWayPoint &c were currently
  //         just loaded from registry.
  SetToRegistry(szRegistryHomeWaypoint,settings.HomeWaypoint);
  SetToRegistry(szRegistryAlternate1,settings.Alternate1);
  SetToRegistry(szRegistryAlternate2,settings.Alternate2);
  SetToRegistry(szRegistryTeamcodeRefWaypoint,settings.TeamCodeRefWaypoint);
}

WayPointParser::WayPointParser()
{
  if (WaypointsOutOfRangeSetting == 1)
    WaypointOutOfTerrainRangeDialogResult = wpTerrainBoundsYesAll;
  if (WaypointsOutOfRangeSetting == 2)
    WaypointOutOfTerrainRangeDialogResult = wpTerrainBoundsNoAll;

  ClearFile();
}

bool
WayPointParser::ReadWaypoints(Waypoints &way_points,
                              const RasterTerrain *terrain)
{
  StartupStore(TEXT("ReadWaypoints\n"));

  TCHAR szFile[MAX_PATH];
  bool loaded = false;
  WayPointParser parser;

  // Delete old waypoints
  CloseWaypoints(way_points);

  // ### FIRST FILE ###

  // Get first waypoint filename
  GetRegistryString(szRegistryWayPointFile, szFile, MAX_PATH);
  // and clear registry setting (if loading goes totally wrong)
  SetRegistryString(szRegistryWayPointFile, TEXT("\0"));

  // If waypoint file exists
  if (parser.SetFile(szFile, true, 0)) {
    // parse the file
    if (parser.Parse(way_points, terrain)) {
      // reset the registry to the actual file name
      SetRegistryString(szRegistryWayPointFile, szFile);

      // Set waypoints writable flag
      way_points.set_file0_writable(parser.IsWritable());
      loaded = true;
    } else {
      StartupStore(TEXT("Parse error in waypoint file 1\n"));
    }
  } else {
    StartupStore(TEXT("No waypoint file 1\n"));
  }

  // ### SECOND FILE ###

  // Get second waypoint filename
  GetRegistryString(szRegistryAdditionalWayPointFile, szFile, MAX_PATH);
  // and clear registry setting (if loading goes totally wrong)
  SetRegistryString(szRegistryAdditionalWayPointFile, TEXT("\0"));

  // If waypoint file exists
  if (parser.SetFile(szFile, true, 1)) {
    // parse the file
    if (parser.Parse(way_points, terrain)) {
      // reset the registry to the actual file name
      SetRegistryString(szRegistryWayPointFile, szFile);

      // Set waypoints writable flag
      way_points.set_file0_writable(!loaded && parser.IsWritable());
      loaded = true;
    } else {
      StartupStore(TEXT("Parse error in waypoint file 2\n"));
    }
  } else {
    StartupStore(TEXT("No waypoint file 2\n"));
  }

  // ### MAP/THIRD FILE ###

  // If no waypoint file loaded yet
  if (!loaded) {
    // Get the map filename
    GetRegistryString(szRegistryMapFile, szFile, MAX_PATH);
    _tcscat(szFile, TEXT("/"));
    _tcscat(szFile, TEXT("waypoints.xcw"));

    // If waypoint file inside map file exists
    if (parser.SetFile(szFile, true, 2)) {
      // parse the file
      if (parser.Parse(way_points, terrain)) {
        // reset the registry to the actual file name
        SetRegistryString(szRegistryWayPointFile, szFile);

        // Set waypoints writable flag
        way_points.set_file0_writable(false);
        loaded = true;
      } else {
        StartupStore(TEXT("Parse error in map waypoint file\n"));
      }
    } else {
      StartupStore(TEXT("No waypoint file in the map file\n"));
    }
  }

  // Optimise the waypoint list after attaching new waypoints
  way_points.optimise();

  // Return whether waypoints have been loaded into the waypoint list
  return loaded;
}

void
WayPointParser::SaveWaypoints(Waypoints &way_points)
{
  StartupStore(TEXT("SaveWaypoints\n"));

  TCHAR szFile[MAX_PATH];
  WayPointParser parser;

  // ### FIRST FILE ###

  // Get first waypoint filename
  GetRegistryString(szRegistryWayPointFile, szFile, MAX_PATH);

  // If waypoint file seems okay
  if (parser.SetFile(szFile, false, 0)) {
    // Save the file
    if (!parser.Save(way_points))
      StartupStore(TEXT("Save error in waypoint file 1\n"));
  } else {
    StartupStore(TEXT("Waypoint file 1 can not be written\n"));
  }

  // ### SECOND FILE ###

  // Get second waypoint filename
  GetRegistryString(szRegistryAdditionalWayPointFile, szFile, MAX_PATH);

  // If waypoint file seems okay
  if (parser.SetFile(szFile, false, 1)) {
    // Save the file
    if (!parser.Save(way_points))
      StartupStore(TEXT("Save error in waypoint file 2\n"));
  } else {
    StartupStore(TEXT("Waypoint file 2 can not be written\n"));
  }
}

void
WayPointParser::CloseWaypoints(Waypoints &way_points)
{
  // Clear the waypoint list
  way_points.clear();
}

bool
WayPointParser::SetFile(TCHAR* filename, bool returnOnFileMissing, int filenum)
{
  // If filename is empty -> clear and return false
  if (string_is_empty(filename)) {
    ClearFile();
    return false;
  }

  // Save the filenumber
  this->filenum = filenum;

  // Copy the filename to the internal field
  _tcscpy(file, filename);
  // and convert it to filepath
  ExpandLocalPath(file);

  // Convert the filepath from unicode to ascii for zzip files
  char path_ascii[MAX_PATH];
  unicode2ascii(file, path_ascii, sizeof(path_ascii));

  // If file does not exist -> clear and return true
  if (returnOnFileMissing &&
      !FileExists(file) &&
      !FileExistsZipped(path_ascii)) {
    ClearFile();
    return false;
  }

  // If file does not exist but exists inside map file -> save compressed flag
  if (!FileExists(file) &&
      FileExistsZipped(path_ascii))
    compressed = true;

  // If WinPilot waypoint file -> save type and return true
  if (MatchesExtension(filename, _T(".dat")) ||
      MatchesExtension(filename, _T(".xcw"))) {
    filetype = ftWinPilot;
    return true;
  }

  // If SeeYou waypoint file -> save type and return true
  if (MatchesExtension(filename, _T(".cup"))) {
    filetype = ftSeeYou;
    return true;
  }

  // If Zander waypoint file -> save type and return true
  if (MatchesExtension(filename, _T(".wpz"))) {
    filetype = ftZander;
    return true;
  }

  // If unknown file -> clear and return false
  ClearFile();
  return false;
}

void
WayPointParser::ClearFile()
{
  file[0] = 0;
  compressed = false;
}

bool
WayPointParser::Parse(Waypoints &way_points, const RasterTerrain *terrain)
{
  // If no file loaded yet -> return false
  if (file[0] == 0)
    return false;

  TCHAR line[255];

  // If normal file
  if (!compressed) {
    // Try to open waypoint file
    FILE *fp;
    fp = _tfopen(file, _T("rt"));
    if (fp == NULL)
      return false;

    // Read through the lines of the file
    for (unsigned i = 0; ReadStringX(fp, 255, line); i++) {
      // and parse them
      parseLine(line, i, way_points, terrain);
    }

    fclose(fp);

  // If compressed file inside map file
  } else {
    // convert path to ascii
    char path_ascii[MAX_PATH];
    unicode2ascii(file, path_ascii, sizeof(path_ascii));

    // Try to open compressed waypoint file inside map file
    ZZIP_FILE *fp;
    fp = zzip_fopen(path_ascii, "rt");
    if (fp == NULL)
      return false;

    // Read through the lines of the file
    for (unsigned i = 0; ReadString(fp, 255, line); i++) {
      // and parse them
      parseLine(line, i, way_points, terrain);
    }

    zzip_fclose(fp);
  }

  return true;
}

bool
WayPointParser::Save(Waypoints &way_points)
{
  // No filename -> return
  if (file[0] == 0)
    return false;
  // Not writable -> return
  if (!IsWritable())
    return false;
  // Compressed file -> return
  if (compressed)
    return false;

  // Try to open waypoint file for writing
  FILE *fp;
  fp = _tfopen(file, _T("wt"));
  if (fp == NULL)
    return false;

  // Call the saveFile function depending on the file type
  switch (filetype) {
    case ftWinPilot:
      saveFileWinPilot(fp, way_points);
      break;
  }

  // Close the file
  fclose(fp);

  // and tell everyone we saved successfully
  return true;
}

bool
WayPointParser::parseLine(const TCHAR* line, unsigned linenum,
    Waypoints &way_points, const RasterTerrain *terrain)
{
  // Hand the parsing over to the right parser depending on the file type
  switch (filetype) {
    case ftWinPilot:
      return parseLineWinPilot(line, linenum, way_points, terrain);
    case ftSeeYou:
      return parseLineSeeYou(line, linenum, way_points, terrain);
    case ftZander:
      return parseLineZander(line, linenum, way_points, terrain);
  }

  // Return false if the file type is not known
  return false;
}

bool
WayPointParser::parseLineWinPilot(const TCHAR* line, const unsigned linenum,
    Waypoints &way_points, const RasterTerrain *terrain)
{
  TCHAR ctemp[255];
  const TCHAR *params[20];
  size_t n_params;

  // If (end-of-file or comment)
  if (line[0] == '\0' || line[0] == 0x1a ||
      _tcsstr(line, _T("**")) == line ||
      _tcsstr(line, _T("*")) == line)
    // -> return without error condition
    return true;

  // Create new waypoint (not appended to the waypoint list yet)
  Waypoint new_waypoint = way_points.create(GEOPOINT(fixed_zero, fixed_zero));
  // Set FileNumber
  new_waypoint.FileNum = filenum;
  // Set Zoom to zero as default
  new_waypoint.Zoom = 0;
  // Set flags to false as default
  setDefaultFlags(new_waypoint.Flags, false);

  // Get fields
  n_params = extractParameters(line, ctemp, params, 20);
  if (n_params < 5)
    return false;

  // Name (e.g. KAMPLI)
  if (!parseStringWinPilot(params[5], new_waypoint.Name))
    return false;

  // Latitude (e.g. 51:15.900N)
  if (!parseAngleWinPilot(params[1], new_waypoint.Location.Latitude, true))
    return false;

  // Longitude (e.g. 00715.900W)
  if (!parseAngleWinPilot(params[2], new_waypoint.Location.Longitude, false))
    return false;

  // Altitude (e.g. 458M)
  /// @todo configurable behaviour
  bool alt_ok = parseAltitudeWinPilot(params[3], new_waypoint.Altitude);
  // Load waypoint altitude from terrain
  double t_alt = terrain != NULL
    ? AltitudeFromTerrain(new_waypoint.Location, *terrain)
    : TERRAIN_INVALID;
  if (t_alt == TERRAIN_INVALID) {
    if (!alt_ok)
      new_waypoint.Altitude = fixed_zero;
  } else { // TERRAIN_VALID
    if (!alt_ok || abs((fixed)t_alt - new_waypoint.Altitude) > 100)
    new_waypoint.Altitude = (fixed)t_alt;
  }

  // Description (e.g. 119.750 Airport)
  parseStringWinPilot(params[6], new_waypoint.Comment);

  // Waypoint Flags (e.g. AT)
  parseFlagsWinPilot(params[4], new_waypoint.Flags);

  // if waypoint out of terrain range and should not be included
  // -> return without error condition
  if (terrain != NULL && !checkWaypointInTerrainRange(new_waypoint, *terrain))
    return true;

  // Append the new waypoint to the waypoint list and
  // return successful line parse
  way_points.append(new_waypoint);
  return true;
}

bool
WayPointParser::parseStringWinPilot(const TCHAR* src, tstring& dest)
{
  // Just assign and trim it
  dest.assign(src);
  trim_inplace(dest);
  return true;
}

bool
WayPointParser::parseAngleWinPilot(const TCHAR* src, fixed& dest, const bool lat)
{
  // Two format variants:
  // 51:47.841N (DD:MM.mmm // the usual)
  // 51:23:45N (DD:MM:SS)

  double val;
  char sign = 0;
  unsigned deg, min, sec;

  // Parse unusual string
  int s =_stscanf(src, _T("%u:%u:%u%c"), &deg, &min, &sec, &sign);
  // Hack: the E sign for east is interpreted as exponential sign
  if (s == 4 || (s == 3 && sign == 0)) {
    // Calculate angle
    val = deg + (fixed)min / 60 + (fixed)sec / 3600;
  } else {
    // Parse usual string
    s =_stscanf(src, _T("%u:%lf%c"), &deg, &val, &sign);
    // Hack: the E sign for east is interpreted as exponential sign
    if (!(s == 3 || (s == 2 && sign == 0)))
      return false;

    // Calculate angle
    unsigned minfrac = iround((val - (int)val) * 1000);
    min = (int)val;
    val = deg + ((fixed)min + (fixed)minfrac / 1000) / 60;
  }

  // Limit angle to +/- 90 degrees for Latitude or +/- 180 degrees for Longitude
  val = std::min(val, (lat ? 90.0 : 180.0));

  // Make angle negative if southern/western hemisphere
  if (sign == 'W' || sign == 'w' || sign == 'S' || sign == 's')
    val *= -1;

  // Save angle
  dest = (fixed)val;
  return true;
}

bool
WayPointParser::parseAltitudeWinPilot(const TCHAR* src, fixed& dest)
{
  double val;
  char unit;

  // Parse string
  if (_stscanf(src, _T("%lf%c"), &val, &unit) != 2)
    return false;

  // Convert to system unit if necessary
  if (unit == 'F' || unit == 'f')
    val = Units::ToSysUnit(val, unFeet);

  // Save altitude
  dest = (fixed)val;
  return true;
}

bool
WayPointParser::parseFlagsWinPilot(const TCHAR* src, WaypointFlags& dest)
{
  // A = Airport
  // T = Turnpoint
  // L = Landable
  // H = Home
  // S = Start point
  // F = Finish point
  // R = Restricted
  // W = Waypoint flag

  for (unsigned i = 0; src[i] != 0; i++) {
    switch (src[i]) {
    case 'A':
    case 'a':
      dest.Airport = true;
      break;
    case 'T':
    case 't':
      dest.TurnPoint = true;
      break;
    case 'L':
    case 'l':
      dest.LandPoint = true;
      break;
    case 'H':
    case 'h':
      dest.Home = true;
      break;
    case 'S':
    case 's':
      dest.StartPoint = true;
      break;
    case 'F':
    case 'f':
      dest.FinishPoint = true;
      break;
    case 'R':
    case 'r':
      dest.Restricted = true;
      break;
    case 'W':
    case 'w':
      dest.WaypointFlag = true;
      break;
    }
  }

  return true;
}

void
WayPointParser::saveFileWinPilot(FILE *fp, const Waypoints &way_points)
{
  // Iterate through the waypoint list and save each waypoint
  // into the file defined by fp
  /// @todo JMW: iteration ordered by ID would be preferred
  for (Waypoints::WaypointTree::const_iterator it = way_points.begin();
       it != way_points.end(); it++) {
    const Waypoint& wp = it->get_waypoint();
    if (wp.FileNum == filenum)
      _ftprintf(fp, _T("%s\n"), composeLineWinPilot(wp).c_str());
  }
}

tstring
WayPointParser::composeLineWinPilot(const Waypoint& wp)
{
  // Prepare waypoint id
  TCHAR buf[10];
  _stprintf(buf, _T("%u"), wp.id);

  // Attach the waypoint id to the output
  tstring dest = buf;
  dest += _T(",");
  // Attach the latitude to the output
  dest += composeAngleWinPilot(wp.Location.Latitude, true);
  dest += _T(",");
  // Attach the longitude id to the output
  dest += composeAngleWinPilot(wp.Location.Longitude, false);
  dest += _T(",");
  // Attach the altitude id to the output
  dest += composeAltitudeWinPilot(wp.Altitude);
  dest += _T(",");
  // Attach the waypoint flags to the output
  dest += composeFlagsWinPilot(wp.Flags);
  dest += _T(",");
  // Attach the waypoint name to the output
  dest += wp.Name;
  dest += _T(",");
  // Attach the waypoint description to the output
  dest += wp.Comment;
  return dest;
}

tstring
WayPointParser::composeAngleWinPilot(const fixed& src, const bool lat)
{
  TCHAR buffer[20];
  bool negative = src < 0;

  // Calculate degrees, minutes and seconds
  int deg = fabs(src);
  int min = (fabs(src) - deg) * 60;
  int sec = (((fabs(src) - deg) * 60) - min) * 60;

  // Save them into the buffer string
  _stprintf(buffer, (lat ? _T("%02d:%02d:%02d") : _T("%03d:%02d:%02d")),
            deg, min, sec);

  // Attach the buffer string to the output
  tstring dest = buffer;
  if (lat)
    dest += (negative ? _T("S") : _T("N"));
  else
    dest += (negative ? _T("W") : _T("E"));

  return dest;
}

tstring
WayPointParser::composeAltitudeWinPilot(const fixed& src)
{
  // Save the formatted altitude into the buffer string
  TCHAR buf[10];
  _stprintf(buf, _T("%d"), (int)src);

  // Attach the buffer string to the output string
  tstring dest = buf;
  // Attach the unit to the output string
  dest += _T("M");
  return dest;
}

tstring
WayPointParser::composeFlagsWinPilot(const WaypointFlags& src)
{
  tstring dest = _T("");

  if (src.Airport)
    dest += _T("A");
  if (src.TurnPoint)
    dest += _T("T");
  if (src.LandPoint)
    dest += _T("L");
  if (src.Home)
    dest += _T("H");
  if (src.StartPoint)
    dest += _T("S");
  if (src.FinishPoint)
    dest += _T("F");
  if (src.Restricted)
    dest += _T("R");
  if (src.WaypointFlag)
    dest += _T("W");

  // set as turnpoint by default if nothing else
  if (dest.length() < 1)
    dest = _T("T");

  return dest;
}

bool
WayPointParser::parseLineSeeYou(const TCHAR* line, const unsigned linenum,
    Waypoints &way_points, const RasterTerrain *terrain)
{
  TCHAR ctemp[255];
  const TCHAR *params[20];
  size_t n_params;

  static unsigned iName = 0, iCode = 1, iCountry = 2;
  static unsigned iLatitude = 3, iLongitude = 4, iElevation = 5;
  static unsigned iStyle = 6, iRWDir = 7, iRWLen = 8;
  static unsigned iFrequency = 9, iDescription = 10;

  static bool ignore_following = false;

  // If (end-of-file or comment)
  if (line[0] == '\0' || line[0] == 0x1a ||
      _tcsstr(line, _T("**")) == line ||
      _tcsstr(line, _T("*")) == line)
    // -> return without error condition
    return true;

  // Parse first line holding field order
  /// @todo linenum == 0 should be the first
  /// (not ignored) line, not just line 0
  if (linenum == 0) {
    // Get fields
    n_params = extractParameters(line, ctemp, params, 20);

    // Iterate through fields and save the field order
    for (unsigned i = 0; i < n_params; i++) {
      const TCHAR* value = params[i];

      if (!_tcscmp(value, _T("name")))
        iName = i;
      else if (!_tcscmp(value, _T("code")))
        iCode = i;
      else if (!_tcscmp(value, _T("country")))
        iCountry = i;
      else if (!_tcscmp(value, _T("lat")))
        iLatitude = i;
      else if (!_tcscmp(value, _T("lon")))
        iLongitude = i;
      else if (!_tcscmp(value, _T("elev")))
        iElevation = i;
      else if (!_tcscmp(value, _T("style")))
        iStyle = i;
      else if (!_tcscmp(value, _T("rwdir")))
        iRWDir = i;
      else if (!_tcscmp(value, _T("rwlen")))
        iRWLen = i;
      else if (!_tcscmp(value, _T("freq")))
        iFrequency = i;
      else if (!_tcscmp(value, _T("desc")))
        iDescription = i;
    }
    ignore_following = false;

    return true;
  }

  // If task marker is reached ignore all following lines
  if (_tcsstr(line, _T("-----Related Tasks-----")) == line)
    ignore_following = true;
  if (ignore_following)
    return true;

  // Create new waypoint (not appended to the waypoint list yet)
  Waypoint new_waypoint = way_points.create(GEOPOINT(fixed_zero, fixed_zero));
  // Set FileNumber
  new_waypoint.FileNum = filenum;
  // Set Zoom to zero as default
  new_waypoint.Zoom = 0;
  // Set flags to false as default
  setDefaultFlags(new_waypoint.Flags);

  // Get fields
  n_params = extractParameters(line, ctemp, params, 20);

  // Check if the basic fields are provided
  if (iName >= n_params)
    return false;
  if (iLatitude >= n_params)
    return false;
  if (iLongitude >= n_params)
    return false;

  // Name (e.g. "Some Turnpoint", with quotes)
  if (!parseStringSeeYou(params[iName], new_waypoint.Name))
    return false;

  // Latitude (e.g. 5115.900N)
  if (!parseAngleSeeYou(params[iLatitude], new_waypoint.Location.Latitude, true))
    return false;

  // Longitude (e.g. 00715.900W)
  if (!parseAngleSeeYou(params[iLongitude], new_waypoint.Location.Longitude, false))
    return false;

  // Elevation (e.g. 458.0m)
  /// @todo configurable behaviour
  bool alt_ok = parseAltitudeSeeYou(params[iElevation], new_waypoint.Altitude);
  // Load waypoint altitude from terrain
  double t_alt = terrain != NULL
    ? AltitudeFromTerrain(new_waypoint.Location, *terrain)
    : TERRAIN_INVALID;
  if (t_alt == TERRAIN_INVALID) {
    if (!alt_ok)
      new_waypoint.Altitude = fixed_zero;
  } else { // TERRAIN_VALID
    if (!alt_ok || abs((fixed)t_alt - new_waypoint.Altitude) > 100)
      new_waypoint.Altitude = (fixed)t_alt;
  }

  // Description (e.g. "Some Turnpoint", with quotes)
  /// @todo include frequency and rwdir/len
  if (iDescription < n_params)
    parseStringSeeYou(params[iDescription], new_waypoint.Comment);

  // Style (e.g. 5)
  /// @todo include peaks with peak symbols etc.
  if (iStyle < n_params)
    parseStyleSeeYou(params[iStyle], new_waypoint.Flags);

  // If the Style attribute did not state that this is an airport
  if (!new_waypoint.Flags.Airport) {
    // -> parse the runway length
    fixed rwlen;
    // Runway length (e.g. 546.0m)
    if (iRWLen < n_params && parseAltitudeSeeYou(params[iRWLen], rwlen)) {
      // If runway length is between 100m and 300m -> landpoint
      if (rwlen > 100 && rwlen <= 300)
        new_waypoint.Flags.LandPoint = true;
      // If runway length is higher then 300m -> airport
      if (rwlen > 300)
        new_waypoint.Flags.Airport = true;
    }
  }

  // if waypoint out of terrain range and should not be included
  // -> return without error condition
  if (terrain != NULL && !checkWaypointInTerrainRange(new_waypoint, *terrain))
    return true;

  // Append the new waypoint to the waypoint list and
  // return successful line parse
  way_points.append(new_waypoint);
  return true;
}

bool
WayPointParser::parseStringSeeYou(const TCHAR* src, tstring& dest)
{
  dest.assign(src);

  // Strip quote characters
  int len = dest.length();
  if ((src[0] == '"' || src[0] == '\'') && len >= 2)
    dest = dest.substr(1, len - 2);

  trim_inplace(dest);
  return true;
}

bool
WayPointParser::parseAngleSeeYou(const TCHAR* src, fixed& dest, const bool lat)
{
  double val;
  char sign = 0;

  // Parse string
  int s =_stscanf(src, _T("%lf%c"), &val, &sign);
  // Hack: the E sign for east is interpreted as exponential sign
  if (!(s == 2 || (s == 1 && sign == 0)))
    return false;

  // Calculate angle
  unsigned minfrac = iround((val - (int)val) * 1000);
  unsigned min = (int)val % 100;
  unsigned deg = ((int)val - min) * 0.01;
  val = deg + ((fixed)min + (fixed)minfrac / 1000) / 60;

  // Limit angle to +/- 90 degrees for Latitude or +/- 180 degrees for Longitude
  val = std::min(val, (lat ? 90.0 : 180.0));

  // Make angle negative if southern/western hemisphere
  if (sign == 'W' || sign == 'w' || sign == 'S' || sign == 's')
    val *= -1;

  // Save angle
  dest = (fixed)val;
  return true;
}

bool
WayPointParser::parseAltitudeSeeYou(const TCHAR* src, fixed& dest)
{
  double val;
  char unit;

  // Parse string
  if (_stscanf(src, _T("%lf%c"), &val, &unit) != 2)
    return false;

  // Convert to system unit if necessary
  if (unit == 'F' || unit == 'f')
    val = Units::ToSysUnit(val, unFeet);

  // Save altitude
  dest = (fixed)val;
  return true;
}

bool
WayPointParser::parseStyleSeeYou(const TCHAR* src, WaypointFlags& dest)
{
  // 1 - Normal
  // 2 - AirfieldGrass
  // 3 - Outlanding
  // 4 - GliderSite
  // 5 - AirfieldSolid ...
  unsigned style;

  // Parse string
  if (_stscanf(src, _T("%u"), &style) != 1)
    return false;

  // Update flags
  dest.LandPoint = (style == 3);
  dest.Airport = (style >= 2 && style <= 5);

  return true;
}

bool
WayPointParser::parseLineZander(const TCHAR* line, const unsigned linenum,
    Waypoints &way_points, const RasterTerrain *terrain)
{
  // If (end-of-file or comment)
  if (line[0] == '\0' || line[0] == 0x1a ||
      _tcsstr(line, _T("**")) == line ||
      _tcsstr(line, _T("*")) == line)
    // -> return without error condition
    return true;

  // Determine the length of the line
  size_t len = _tcslen(line);
  // If less then 34 characters -> something is wrong -> cancel
  if (len < 34)
    return false;

  // Create new waypoint (not appended to the waypoint list yet)
  Waypoint new_waypoint = way_points.create(GEOPOINT(fixed_zero, fixed_zero));
  // Set FileNumber
  new_waypoint.FileNum = filenum;
  // Set Zoom to zero as default
  new_waypoint.Zoom = 0;
  // Set flags to false as default
  setDefaultFlags(new_waypoint.Flags, true);

  // Name (Characters 0-12)
  if (!parseStringZander(line, new_waypoint.Name))
    return false;

  // Latitude (Characters 13-20 // DDMMSS(N/S))
  if (!parseAngleZander(line + 13, new_waypoint.Location.Latitude, true))
    return false;

  // Longitude (Characters 21-29 // DDDMMSS(E/W))
  if (!parseAngleZander(line + 21, new_waypoint.Location.Longitude, false))
    return false;

  // Altitude (Characters 30-34 // e.g. 1561 (in meters))
  /// @todo configurable behaviour
  bool alt_ok = parseAltitudeZander(line + 30, new_waypoint.Altitude);
  // Load waypoint altitude from terrain
  double t_alt = terrain != NULL
    ? AltitudeFromTerrain(new_waypoint.Location, *terrain)
    : TERRAIN_INVALID;
  if (t_alt == TERRAIN_INVALID) {
    if (!alt_ok)
      new_waypoint.Altitude = fixed_zero;
  } else { // TERRAIN_VALID
    if (!alt_ok || abs((fixed)t_alt - new_waypoint.Altitude) > 100)
    new_waypoint.Altitude = (fixed)t_alt;
  }

  // Description (Characters 35-44)
  if (len > 35)
    parseStringZander(line + 35, new_waypoint.Comment);

  // Flags (Characters 45-49)
  if (len > 45)
    parseFlagsZander(line + 45, new_waypoint.Flags);

  // if waypoint out of terrain range and should not be included
  // -> return without error condition
  if (terrain != NULL && !checkWaypointInTerrainRange(new_waypoint, *terrain))
    return true;

  // Append the new waypoint to the waypoint list and
  // return successful line parse
  way_points.append(new_waypoint);
  return true;
}

bool
WayPointParser::parseStringZander(const TCHAR* src, tstring& dest)
{
  if (src[0] == 0)
    return true;

  dest.assign(src);

  // Cut the string after the first space, tab or null character
  size_t found = dest.find_first_of(_T(" \t\0"));
  if (found != tstring::npos)
    dest = dest.substr(0, found);

  trim_inplace(dest);
  return true;
}

bool
WayPointParser::parseAngleZander(const TCHAR* src, fixed& dest, const bool lat)
{
  double val;
  char sign = 0;

  // Parse string
  int s =_stscanf(src, _T("%lf%c"), &val, &sign);
  // Hack: the E sign for east is interpreted as exponential sign
  if (!(s == 2 || (s == 1 && sign == 0)))
    return false;

  // Calculate angle
  unsigned deg = (int)(val * 0.0001);
  unsigned min = (int)((val - deg * 10000) * 0.01);
  unsigned sec = iround((int)val % 100);
  val = deg + ((fixed)min / 60) + ((fixed)sec / 3600);

  // Limit angle to +/- 90 degrees for Latitude or +/- 180 degrees for Longitude
  val = std::min(val, (lat ? 90.0 : 180.0));

  // Make angle negative if southern/western hemisphere
  if (sign == 'W' || sign == 'w' || sign == 'S' || sign == 's')
    val *= -1;

  // Save angle
  dest = (fixed)val;
  return true;
}

bool
WayPointParser::parseAltitudeZander(const TCHAR* src, fixed& dest)
{
  double val;

  // Parse string
  if (_stscanf(src, _T("%lf"), &val) != 1)
    return false;

  // Save altitude
  dest = (fixed)val;
  return true;
}

bool
WayPointParser::parseFlagsZander(const TCHAR* src, WaypointFlags& dest)
{
  // WP = Waypoint
  // HA = Home Field
  // WA = WP + Airfield
  // LF = Landing Field
  // WL = WP + Landing Field
  // RA = Restricted

  // If flags field exists (this function isn't called otherwise)
  // -> turnpoint needs to be declared as one
  dest.TurnPoint = false;

  if ((src[0] == 'W' || src[0] == 'w') &&
      (src[1] == 'P' || src[1] == 'p')) {
    dest.TurnPoint = true;
  }
  else if ((src[0] == 'H' || src[0] == 'h') &&
           (src[1] == 'A' || src[1] == 'a')) {
    dest.TurnPoint = true;
    dest.Airport = true;
    dest.Home = true;
  }
  else if ((src[0] == 'W' || src[0] == 'w') &&
           (src[1] == 'A' || src[1] == 'a')) {
    dest.TurnPoint = true;
    dest.Airport = true;
  }
  else if ((src[0] == 'L' || src[0] == 'l') &&
           (src[1] == 'F' || src[1] == 'f')) {
    dest.LandPoint = true;
  }
  else if ((src[0] == 'W' || src[0] == 'w') &&
           (src[1] == 'L' || src[1] == 'l')) {
    dest.TurnPoint = true;
    dest.LandPoint = true;
  }
  else if ((src[0] == 'R' || src[0] == 'r') &&
           (src[1] == 'A' || src[1] == 'a')) {
    dest.Restricted = true;
  }

  return true;
}

void
WayPointParser::setDefaultFlags(WaypointFlags& dest, bool turnpoint)
{
  dest.Airport = false;
  dest.TurnPoint = turnpoint;
  dest.LandPoint = false;
  dest.Home = false;
  dest.StartPoint = false;
  dest.FinishPoint = false;
  dest.Restricted = false;
  dest.WaypointFlag = false;
}

bool
WayPointParser::checkWaypointInTerrainRange(const Waypoint &way_point,
                                            const RasterTerrain &terrain)
{
  TCHAR sTmp[250];

  // If (YesToAll) -> include waypoint
  if (WaypointOutOfTerrainRangeDialogResult == wpTerrainBoundsYesAll)
    return true;

  // If (No Terrain) -> include waypoint
  if (!terrain.isTerrainLoaded())
    return true;

  // If (Waypoint in Terrain range) -> include waypoint
  if (terrain.WaypointIsInTerrainRange(way_point.Location))
    return true;

  // If (NoToAll) -> dont include waypoint
  if (WaypointOutOfTerrainRangeDialogResult == wpTerrainBoundsNoAll)
    return false;

  // Open Dialogbox
  _stprintf(sTmp, gettext(_T(
      "Waypoint #%d \"%s\" \r\nout of Terrain bounds\r\n\r\nLoad anyway?")),
      way_point.id, way_point.Name.c_str());

  WaypointOutOfTerrainRangeDialogResult = dlgWaypointOutOfTerrain(sTmp);

  // Execute result
  switch (WaypointOutOfTerrainRangeDialogResult) {
  case wpTerrainBoundsYesAll:
    SetToRegistry(szRegistryWaypointsOutOfRange, 1);
    Profile::StoreRegistry();
    return true;

  case wpTerrainBoundsNoAll:
    SetToRegistry(szRegistryWaypointsOutOfRange, 2);
    Profile::StoreRegistry();
    return false;

  case wpTerrainBoundsYes:
    return true;

  default:
  case mrCancel:
  case wpTerrainBoundsNo:
    WaypointOutOfTerrainRangeDialogResult = wpTerrainBoundsNo;
    return false;
  }
}

size_t
WayPointParser::extractParameters(const TCHAR *src, TCHAR *dst,
                                  const TCHAR **arr, size_t sz)
{
  TCHAR c, *p;
  size_t i = 0;

  _tcscpy(dst, src);
  p = dst;

  do {
    arr[i++] = p;
    p = _tcschr(p, _T(','));
    if (!p)
      break;
    c = *p;
    *p++ = _T('\0');
  } while (i != sz && c != _T('\0'));

  return i;
}

double
WayPointParser::AltitudeFromTerrain(GEOPOINT &location,
                                    const RasterTerrain &terrain)
{
  double alt = TERRAIN_INVALID;

  // If terrain not loaded yet -> return INVALID
  if (!terrain.GetMap())
    return TERRAIN_INVALID;

  // Get terrain height
  RasterRounding rounding(*terrain.GetMap(), 0, 0);
  alt = terrain.GetTerrainHeight(location, rounding);

  // If terrain altitude okay -> return terrain altitude
  if (alt > TERRAIN_INVALID)
    return alt;

  return TERRAIN_INVALID;
}
