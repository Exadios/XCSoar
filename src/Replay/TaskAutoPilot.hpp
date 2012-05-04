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
#ifndef TASK_AUTOPILOT_HPP
#define TASK_AUTOPILOT_HPP

#include "Math/fixed.hpp"
#include "Navigation/Aircraft.hpp"
#include "Math/Angle.hpp"
#include "Util/Filter.hpp"
#include <vector>

struct AutopilotParameters {
  fixed bearing_noise;
  fixed target_noise;
  fixed turn_speed;
  fixed sink_factor;
  fixed climb_factor;
  fixed start_alt;
  bool enable_bestcruisetrack;
  bool goto_target;

  AutopilotParameters():
    target_noise(0.1),
    sink_factor(1.0),
    climb_factor(1.0),
    start_alt(1500.0),
    enable_bestcruisetrack(false),
    goto_target(false)
    {
      realistic();
    };

  void ideal();
  void realistic();
};

class AbstractAutoPilot {
public:
  GeoPoint location_start;
  GeoPoint location_previous;
  Angle heading;

  void set_default_location(const GeoPoint& default_location) {
    location_start = default_location;
    location_previous = default_location;
    location_previous.latitude-= Angle::Degrees(fixed(1.0));
  }

protected:
  virtual void on_manual_advance() {};
  virtual void on_mode_change() {};
  virtual void OnClose() {};
};

struct ElementStat;
class TaskAccessor;

class TaskAutoPilot: public AbstractAutoPilot {
public:
  enum AcState {
    Climb = 0,
    Cruise,
    FinalGlide
  };

protected:
  AcState acstate;
  unsigned awp;

private:
  const AutopilotParameters &parms;
  Filter heading_filt;
  fixed climb_rate;
  fixed speed_factor;
  bool short_flight;
  GeoPoint w[2];

public:
  TaskAutoPilot(const AutopilotParameters &_parms);

  virtual void Start(const TaskAccessor& task);
  virtual void Stop();
  virtual void update_mode(const TaskAccessor& task,
                           const AircraftState& state);

  virtual void update_state(const TaskAccessor& task,
                            AircraftState& state, const fixed timestep=fixed_one);

  bool update_autopilot(TaskAccessor& task,
                        const AircraftState& state,
                        const AircraftState& state_last);

  gcc_pure
  GeoPoint target(const TaskAccessor& task) const;
  void set_speed_factor(fixed f) {
    speed_factor = f;
  }

private:
  bool do_advance(TaskAccessor& task);
  void advance_if_required(TaskAccessor& task);
  bool has_finished(TaskAccessor& task);
  void get_awp(TaskAccessor& task);

  bool current_has_target(const TaskAccessor& task) const;

  virtual GeoPoint get_start_location(const TaskAccessor& task,
                                      bool previous=false);
  void update_cruise_bearing(const TaskAccessor& task,
                             const AircraftState& state,
                             const fixed timestep);
  fixed target_height(const TaskAccessor& task) const;
  Angle heading_deviation();
  bool update_computer(TaskAccessor& task, const AircraftState& state);
  bool far_from_target(const TaskAccessor& task,
                       const AircraftState& state);
};

#endif


