/* Copyright_License {

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

#ifndef OBSERVATIONZONECLIENT_HPP
#define OBSERVATIONZONECLIENT_HPP

#include "ObservationZone.hpp"
#include "Math/fixed.hpp"

class ObservationZonePoint;
class OZBoundary;
class TaskPoint;
struct GeoPoint;
struct AircraftState;

/**
 * Class holding an ObzervationZonePoint, directing calls to it
 */
class ObservationZoneClient: 
  public virtual ObservationZone
{
  ObservationZonePoint *oz_point;

public:
  /**
   * Constructor.  Transfers ownership of the OZ to this object.
   *
   * @param _oz The OZ to store
   */
  ObservationZoneClient(ObservationZonePoint* _oz_point):oz_point(_oz_point) {}

  virtual ~ObservationZoneClient();

  /**
   * Accessor for OZ (for modifying parameters etc)
   *
   * @return Observation zone
   */
  ObservationZonePoint* GetOZPoint() const {
    return oz_point;
  }

  /**
   * Test whether aircraft is inside observation zone.
   *
   * @param ref Aircraft state to test
   *
   * @return True if aircraft is inside observation zone
   */
  virtual bool IsInSector(const AircraftState &ref) const;

  /**
   * If zone when used for start can trigger task start via vertical exit
   *
   * @return True if zone type can have a valid start through top
   */
  virtual bool CanStartThroughTop() const;

  /**
   * Generate a random location inside the OZ (to be used for testing)
   *
   * @param mag proportional magnitude of error from center (0-1)
   *
   * @return Location of point
   */
  GeoPoint GetRandomPointInSector(const fixed mag) const;

  /**
   * Calculate distance reduction for achieved task point,
   * to calcuate scored distance.
   *
   * @return Distance reduction once achieved
   */
  virtual fixed ScoreAdjustment() const;

  /**
   * Calculate boundary point from parametric border
   *
   * @param t t value (0,1) of parameter
   *
   * @return Boundary point
   */
  GeoPoint GetBoundaryParametric(fixed t) const;

  virtual OZBoundary GetBoundary() const;

protected:
  /**
   * Check transition constraints
   *
   * @param ref_now Current aircraft state
   * @param ref_last Previous aircraft state
   *
   * @return True if constraints are satisfied
   */
  virtual bool TransitionConstraint(const AircraftState & ref_now, 
                                    const AircraftState & ref_last) const;

  /**
   * Set previous/next taskpoints to allow OZ to update its geometry
   *
   * @param previous Origin tp of inbound leg
   * @param current Tp of this OZ
   * @param next Destination of outbound leg
   */
  virtual void SetLegs(const TaskPoint *previous,
                       const TaskPoint *current,
                       const TaskPoint *next);
};


#endif
