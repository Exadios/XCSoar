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

#include "TaskAutoPilot.hpp"
#include "TaskAccessor.hpp"
#include "Task/TaskStats/ElementStat.hpp"
#include "GlideSolvers/GlidePolar.hpp"

#define fixed_1000 fixed(1000)
#define fixed_20 fixed(20)
#define fixed_1500 fixed(1500)


void
AutopilotParameters::ideal()
{
  bearing_noise= fixed_zero;
  turn_speed= fixed(90.0);
}

void
AutopilotParameters::realistic()
{
  bearing_noise= fixed(20.0);
  turn_speed= fixed(12.0);
}

TaskAutoPilot::TaskAutoPilot(const AutopilotParameters &_parms):
  parms(_parms),
  heading_filt(fixed(8)),
  climb_rate(2.0),
  speed_factor(1.0)
{

}

void
TaskAutoPilot::Stop()
{
}

void
TaskAutoPilot::Start(const TaskAccessor& task)
{
  // construct list of places we will fly to.
  // we dont do this dynamically so it is remembered even if
  // the task is advanced/retreated.

  if (task.is_ordered()) {
    // construct list

    // this pilot is inaccurate, he flies to a random point in the OZ,
    // and starts in the start OZ.

    w[1] = task.random_oz_point(0, parms.target_noise);
    w[0] = task.random_oz_point(1, parms.target_noise);

  } else {
    // for non-ordered tasks, start at the default location

    w[1] = location_start;
    if (task.is_empty()) {
      // go somewhere nearby
      w[0] = location_previous;
    } else {
      // go directly to the target
      w[0] = task.random_oz_point(0, parms.target_noise);
    }
  }

  // pick up the locations from the task to be used to initialise
  // the aircraft simulator

  location_start = get_start_location(task);
  location_previous = get_start_location(task, false);

  awp= 0;

  // reset the heading
  heading = Angle::Zero();
  heading_filt.Reset(fixed_zero);

  acstate = Cruise;
}

GeoPoint
TaskAutoPilot::get_start_location(const TaskAccessor& task, bool previous)
{
  if (!previous && (task.is_ordered())) {
    // set start location to 200 meters directly behind start
    // (otherwise start may fail to trigger)
    Angle brg = w[0].Bearing(w[1]);
    return GeoVector(fixed(200), brg).EndPoint(w[1]);
  } else {
    return w[0];
  }
}

bool
TaskAutoPilot::current_has_target(const TaskAccessor& task) const
{
  return parms.goto_target && (awp>0);
}

GeoPoint
TaskAutoPilot::target(const TaskAccessor& task) const
{
  if (current_has_target(task))
    // in this mode, we go directly to the target
    return task.getActiveTaskPointLocation();
  else
    // head towards the rough location
    return w[0];
}


Angle
TaskAutoPilot::heading_deviation()
{
  fixed noise_mag = acstate==Climb
    ? half(parms.bearing_noise)
    : parms.bearing_noise;
  fixed r = (fixed_two * rand() / RAND_MAX)-fixed_one;
  fixed deviation = fixed(heading_filt.Update(noise_mag*r));
  return Angle::Degrees(deviation).AsDelta();
}

fixed
TaskAutoPilot::target_height(const TaskAccessor& task) const
{
  return task.target_height();
}

void
TaskAutoPilot::update_mode(const TaskAccessor& task,
                           const AircraftState& state)
{
  switch (acstate) {
  case Cruise:
    /* XXX this condition is probably broken */
    if (awp > 0 && !negative(task.remaining_alt_difference())) {
      acstate = FinalGlide;
      on_mode_change();
    } else {
      if (state.altitude<=target_height(task)) {
        acstate = Climb;
        on_mode_change();
      }
    }
    break;
  case FinalGlide:
    if (task.remaining_alt_difference()<-fixed_20) {
      acstate = Climb;
      on_mode_change();
    }
    break;
  case Climb:
    /* XXX this condition is probably broken */
    if (awp > 0 && !negative(task.remaining_alt_difference())) {
      acstate = FinalGlide;
      on_mode_change();
    } else if (state.altitude>=fixed_1500) {
      acstate = Cruise;
      on_mode_change();
    }
    break;
  };
}


void
TaskAutoPilot::update_cruise_bearing(const TaskAccessor& task,
                                     const AircraftState& state,
                                     const fixed timestep)
{
  const ElementStat &stat = task.leg_stats();
  Angle bct = stat.solution_remaining.cruise_track_bearing;
  Angle bearing;

  if (current_has_target(task)) {
    bearing = stat.solution_remaining.vector.bearing;

    if (parms.enable_bestcruisetrack && (stat.solution_remaining.vector.distance>fixed_1000)) {
      bearing = bct;
    }

  } else {
    bearing = state.location.Bearing(target(task));
  }

  if (positive(state.wind.norm) && positive(state.true_airspeed)) {
    const fixed sintheta = (state.wind.bearing-bearing).sin();
    if (fabs(sintheta)>fixed(0.0001)) {
      bearing +=
        Angle::Radians(asin(sintheta * state.wind.norm / state.true_airspeed));
    }
  }
  Angle diff = (bearing-heading).AsDelta();
  fixed d = diff.Degrees();
  fixed max_turn = parms.turn_speed*timestep;
  heading += Angle::Degrees(max(-max_turn, min(max_turn, d)));
  if (positive(parms.bearing_noise)) {
    heading += heading_deviation()*timestep;
  }
  heading = heading.AsBearing();
}


void
TaskAutoPilot::update_state(const TaskAccessor& task,
                            AircraftState& state, const fixed timestep)
{
  const GlidePolar &glide_polar = task.get_glide_polar();

  switch (acstate) {
  case Cruise:
  case FinalGlide:
  {
    const ElementStat &stat = task.leg_stats();
    if (positive(stat.solution_remaining.v_opt)) {
      state.true_airspeed = stat.solution_remaining.v_opt*speed_factor;
    } else {
      state.true_airspeed = glide_polar.GetVBestLD();
    }
    state.indicated_airspeed = state.true_airspeed;
    state.vario = -glide_polar.SinkRate(state.true_airspeed)*parms.sink_factor;
    update_cruise_bearing(task, state, timestep);
  }
  break;
  case Climb:
  {
    state.true_airspeed = glide_polar.GetVMin();
    fixed d = parms.turn_speed*timestep;
    if (d< fixed_360) {
      heading += Angle::Degrees(d);
    }
    if (positive(parms.bearing_noise)) {
      heading += heading_deviation()*timestep;
    }
    heading = heading.AsBearing();
    state.vario = climb_rate*parms.climb_factor;
  }
    break;
  };
  state.netto_vario = state.vario+glide_polar.SinkRate(state.true_airspeed);
}



bool
TaskAutoPilot::far_from_target(const TaskAccessor& task, const AircraftState& state)
{
  // are we considered close to the target?

  if (task.is_empty() || !task.leg_stats().remaining.IsDefined())
    return w[0].Distance(state.location)>state.ground_speed;

  bool d_far = (task.leg_stats().remaining.GetDistance() > fixed(100));

  if (!task.is_ordered())
    // cheat for non-ordered tasks
    return d_far;

  bool entered = awp >= task.size() || task.has_entered(awp);

  if (current_has_target(task))
    return d_far || !entered;

  fixed dc = w[0].Distance(state.location);
  if (awp==0) {
    return (dc>state.ground_speed);
  }
  return (dc>state.ground_speed) || !entered;
}


bool
TaskAutoPilot::do_advance(TaskAccessor& task)
{
  if (task.is_ordered() && (awp==0)) {
    awp++;
  }
  awp++;
  if (has_finished(task))
    return false;
  task.setActiveTaskPoint(awp);
  get_awp(task);

  return true;
}


bool
TaskAutoPilot::has_finished(TaskAccessor& task)
{
  if (task.is_finished())
    return true;

  if (task.is_ordered()) {
    return awp>= task.size();
  } else {
    return awp>= 1;
  }
}


void
TaskAutoPilot::advance_if_required(TaskAccessor& task)
{
  bool manual_start = false;

  if (task.is_started() && (task.getActiveTaskPointIndex()==0)) {
    manual_start = true;
    awp++;
  }
  if (current_has_target(task) || manual_start) {
    if (task.getActiveTaskPointIndex() < awp) {
      // manual advance
      task.setActiveTaskPoint(awp);
      on_manual_advance();
      get_awp(task);
    }
  }
  if (task.getActiveTaskPointIndex() > awp) {
    awp = task.getActiveTaskPointIndex();
    get_awp(task);
  }
}

void
TaskAutoPilot::get_awp(TaskAccessor& task)
{
  w[0] = task.random_oz_point(awp, parms.target_noise);
}

bool
TaskAutoPilot::update_computer(TaskAccessor& task,
                               const AircraftState& state)
{
  if (!far_from_target(task, state)) {
    OnClose();
    return do_advance(task);
  }

  advance_if_required(task);

  return !has_finished(task);
}


bool
TaskAutoPilot::update_autopilot(TaskAccessor& task,
                                const AircraftState& state,
                                const AircraftState& state_last)
{
  update_mode(task, state);
  return update_computer(task, state);
}

