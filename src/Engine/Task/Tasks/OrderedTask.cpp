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

#include "OrderedTask.hpp"
#include "Task/TaskEvents.hpp"
#include "Task/TaskAdvance.hpp"
#include "BaseTask/OrderedTaskPoint.hpp"
#include "Task/TaskPoints/StartPoint.hpp"
#include "Task/TaskPoints/FinishPoint.hpp"
#include "TaskSolvers/TaskMacCreadyTravelled.hpp"
#include "TaskSolvers/TaskMacCreadyRemaining.hpp"
#include "TaskSolvers/TaskMacCreadyTotal.hpp"
#include "TaskSolvers/TaskCruiseEfficiency.hpp"
#include "TaskSolvers/TaskEffectiveMacCready.hpp"
#include "TaskSolvers/TaskBestMc.hpp"
#include "TaskSolvers/TaskMinTarget.hpp"
#include "TaskSolvers/TaskGlideRequired.hpp"
#include "TaskSolvers/TaskOptTarget.hpp"
#include "Task/Visitors/TaskPointVisitor.hpp"

#include "Task/Factory/RTTaskFactory.hpp"
#include "Task/Factory/FAITaskFactory.hpp"
#include "Task/Factory/FAITriangleTaskFactory.hpp"
#include "Task/Factory/FAIORTaskFactory.hpp"
#include "Task/Factory/FAIGoalTaskFactory.hpp"
#include "Task/Factory/AATTaskFactory.hpp"
#include "Task/Factory/MixedTaskFactory.hpp"
#include "Task/Factory/TouringTaskFactory.hpp"

#include "Waypoint/Waypoints.hpp"

#include "Navigation/Flat/FlatBoundingBox.hpp"
#include "Geo/GeoBounds.hpp"
#include "Task/TaskStats/TaskSummary.hpp"
#include "PathSolvers/TaskDijkstraMin.hpp"
#include "PathSolvers/TaskDijkstraMax.hpp"

static void
SetTaskBehaviour(OrderedTask::OrderedTaskPointVector &vector,
                 const TaskBehaviour &tb)
{
  for (auto i = vector.begin(), end = vector.end(); i != end; ++i)
    (*i)->SetTaskBehaviour(tb);
}

void
OrderedTask::SetTaskBehaviour(const TaskBehaviour &tb)
{
  AbstractTask::SetTaskBehaviour(tb);

  ::SetTaskBehaviour(task_points, tb);
  ::SetTaskBehaviour(optional_start_points, tb);
}

static void
UpdateObservationZones(OrderedTask::OrderedTaskPointVector &points,
                       const TaskProjection &task_projection)
{
  for (auto i = points.begin(), end = points.end(); i != end; ++i)
    (*i)->UpdateOZ(task_projection);
}

void
OrderedTask::update_geometry() 
{
  scan_start_finish();

  if (!HasStart() || !task_points[0])
    return;

  // scan location of task points
  for (auto begin = task_points.cbegin(), end = task_points.cend(), i = begin;
       i != end; ++i) {
    if (i == begin)
      task_projection.reset((*i)->GetLocation());

    (*i)->scan_projection(task_projection);
  }
  // ... and optional start points
  for (auto i = optional_start_points.cbegin(),
         end = optional_start_points.cend(); i != end; ++i)
    (*i)->scan_projection(task_projection);

  // projection can now be determined
  task_projection.update_fast();

  // update OZ's for items that depend on next-point geometry 
  UpdateObservationZones(task_points, task_projection);
  UpdateObservationZones(optional_start_points, task_projection);

  // now that the task projection is stable, and oz is stable,
  // calculate the bounding box in projected coordinates
  for (auto i = task_points.cbegin(), end = task_points.cend(); i != end; ++i)
    (*i)->update_boundingbox(task_projection);

  for (auto i = optional_start_points.cbegin(),
         end = optional_start_points.cend(); i != end; ++i)
    (*i)->update_boundingbox(task_projection);

  // update stats so data can be used during task construction
  /// @todo this should only be done if not flying! (currently done with has_entered)
  if (!taskpoint_start->HasEntered()) {
    GeoPoint loc = taskpoint_start->GetLocation();
    UpdateStatsDistances(loc, true);
    if (HasFinish()) {
      /// @todo: call AbstractTask::update stats methods with fake state
      /// so stats are updated
    }
  }
}

// TIMES

fixed 
OrderedTask::ScanTotalStartTime(const AircraftState &)
{
  if (taskpoint_start)
    return taskpoint_start->GetEnteredState().time;

  return fixed_zero;
}

fixed 
OrderedTask::ScanLegStartTime(const AircraftState &)
{
  if (active_task_point)
    return task_points[active_task_point-1]->GetEnteredState().time;

  return -fixed_one;
}

// DISTANCES

fixed
OrderedTask::scan_distance_min(const GeoPoint &location, bool full)
{
  if (full) {
    if (dijkstra_min == NULL)
      dijkstra_min = new TaskDijkstraMin(*this);

    SearchPoint ac(location, task_projection);
    if (dijkstra_min->DistanceMin(ac)) {
      for (unsigned i = GetActiveIndex(), end = TaskSize(); i != end; ++i)
        set_tp_search_min(i, dijkstra_min->GetSolution(i));
    }

    m_location_min_last = location;
  }
  return taskpoint_start->scan_distance_min();
}

fixed
OrderedTask::scan_distance_max()
{
  if (task_points.empty()) // nothing to do!
    return fixed_zero;

  assert(active_task_point < task_points.size());

  // for max calculations, since one can still travel further in the
  // sector, we pretend we are on the previous turnpoint so the
  // search samples will contain the full boundary
  const unsigned atp = active_task_point;
  if (atp) {
    active_task_point--;
    taskpoint_start->scan_active(task_points[active_task_point]);
  }

  if (dijkstra_max == NULL)
    dijkstra_max = new TaskDijkstraMax(*this);

  if (dijkstra_max->DistanceMax()) {
    for (unsigned i = 0, active = GetActiveIndex(), end = TaskSize();
         i != end; ++i) {
      const SearchPoint &solution = dijkstra_max->GetSolution(i);
      set_tp_search_max(i, solution);
      if (i <= active)
        set_tp_search_achieved(i, solution);
    }
  }

  if (atp) {
    active_task_point = atp;
    taskpoint_start->scan_active(task_points[active_task_point]);
  }
  return taskpoint_start->scan_distance_max();
}

void
OrderedTask::ScanDistanceMinMax(const GeoPoint &location, bool force,
                                fixed *dmin, fixed *dmax)
{
  if (!taskpoint_start)
    return;

  if (force)
    *dmax = scan_distance_max();

  bool force_min = force || distance_is_significant(location, m_location_min_last);
  *dmin = scan_distance_min(location, force_min);
}

fixed
OrderedTask::ScanDistanceNominal()
{
  if (taskpoint_start)
    return taskpoint_start->scan_distance_nominal();

  return fixed_zero;
}

fixed
OrderedTask::ScanDistanceScored(const GeoPoint &location)
{
  if (taskpoint_start)
    return taskpoint_start->scan_distance_scored(location);

  return fixed_zero;
}

fixed
OrderedTask::ScanDistanceRemaining(const GeoPoint &location)
{
  if (taskpoint_start)
    return taskpoint_start->scan_distance_remaining(location);

  return fixed_zero;
}

fixed
OrderedTask::ScanDistanceTravelled(const GeoPoint &location)
{
  if (taskpoint_start)
    return taskpoint_start->scan_distance_travelled(location);

  return fixed_zero;
}

fixed
OrderedTask::ScanDistancePlanned()
{
  if (taskpoint_start)
    return taskpoint_start->scan_distance_planned();

  return fixed_zero;
}

// TRANSITIONS

bool 
OrderedTask::CheckTransitions(const AircraftState &state,
                              const AircraftState &state_last)
{
  if (!taskpoint_start)
    return false;

  taskpoint_start->scan_active(task_points[active_task_point]);

  if (!state.flying)
    return false;

  const int n_task = task_points.size();

  if (!n_task)
    return false;

  FlatBoundingBox bb_last(task_projection.project(state_last.location),1);
  FlatBoundingBox bb_now(task_projection.project(state.location),1);

  bool last_started = TaskStarted();
  const bool last_finished = TaskFinished();

  const int t_min = max(0, (int)active_task_point - 1);
  const int t_max = min(n_task - 1, (int)active_task_point);
  bool full_update = false;

  for (int i = t_min; i <= t_max; i++) {

    bool transition_enter = false;
    bool transition_exit = false;

    if (i==0) {
      full_update |= check_transition_optional_start(state, state_last, bb_now, bb_last, 
                                                     transition_enter, transition_exit,
                                                     last_started);
    }

    full_update |= check_transition_point(*task_points[i], 
                                          state, state_last, bb_now, bb_last, 
                                          transition_enter, transition_exit,
                                          last_started, i==0);

    if (i == (int)active_task_point) {
      const bool last_request_armed = task_advance.request_armed();

      if (task_advance.ready_to_advance(*task_points[i], state,
                                        transition_enter,
                                        transition_exit)) {
        task_advance.set_armed(false);
        
        if (i + 1 < n_task) {
          i++;
          SetActiveTaskPoint(i);
          taskpoint_start->scan_active(task_points[active_task_point]);
          
          if (task_events != NULL)
            task_events->ActiveAdvanced(*task_points[i], i);

          // on sector exit, must update samples since start sector
          // exit transition clears samples
          full_update = true;
        }
      } else if (!last_request_armed && task_advance.request_armed()) {
        if (task_events != NULL)
          task_events->RequestArm(*task_points[i]);
      }
    }
  }

  taskpoint_start->scan_active(task_points[active_task_point]);

  stats.task_finished = TaskFinished();
  stats.task_started = TaskStarted();

  if (stats.task_started)
    taskpoint_finish->set_fai_finish_height(GetStartState().altitude - fixed(1000));

  if (task_events != NULL) {
    if (stats.task_started && !last_started)
      task_events->TaskStart();

    if (stats.task_finished && !last_finished)
      task_events->TaskFinish();
  }

  return full_update;
}


bool
OrderedTask::check_transition_optional_start(const AircraftState &state, 
                                             const AircraftState &state_last,
                                             const FlatBoundingBox& bb_now,
                                             const FlatBoundingBox& bb_last,
                                             bool &transition_enter,
                                             bool &transition_exit,
                                             bool &last_started)
{
  bool full_update = false;

  for (auto begin = optional_start_points.cbegin(),
         end = optional_start_points.cend(), i = begin; i != end; ++i) {
    full_update |= check_transition_point(**i,
                                          state, state_last, bb_now, bb_last, 
                                          transition_enter, transition_exit,
                                          last_started, true);

    if (transition_enter || transition_exit) {
      // we have entered or exited this optional start point, so select it.
      // user has no choice in this: rules for multiple start points are that
      // the last start OZ flown through is used for scoring
      
      select_optional_start(std::distance(begin, i));

      return full_update;
    }
  }
  return full_update;
}


bool
OrderedTask::check_transition_point(OrderedTaskPoint& point,
                                    const AircraftState &state, 
                                    const AircraftState &state_last,
                                    const FlatBoundingBox& bb_now,
                                    const FlatBoundingBox& bb_last,
                                    bool &transition_enter,
                                    bool &transition_exit,
                                    bool &last_started,
                                    const bool is_start)
{
  const bool nearby = point.boundingbox_overlaps(bb_now) || point.boundingbox_overlaps(bb_last);

  if (nearby && point.TransitionEnter(state, state_last)) {
    transition_enter = true;

    if (task_events != NULL)
      task_events->EnterTransition(point);
  }
  
  if (nearby && point.TransitionExit(state, state_last, task_projection)) {
    transition_exit = true;

    if (task_events != NULL)
      task_events->ExitTransition(point);
    
    // detect restart
    if (is_start && last_started)
      last_started = false;
  }
  
  if (is_start) 
    update_start_transition(state, point);
  
  return nearby ? point.UpdateSampleNear(state, task_events, task_projection) :
                  point.UpdateSampleFar(state, task_events, task_projection);
}

// ADDITIONAL FUNCTIONS

bool 
OrderedTask::UpdateIdle(const AircraftState &state,
                        const GlidePolar &glide_polar)
{
  bool retval = AbstractTask::UpdateIdle(state, glide_polar);

  if (HasStart()
      && (task_behaviour.optimise_targets_range)
      && (get_ordered_task_behaviour().aat_min_time > fixed_zero)) {

    CalcMinTarget(state, glide_polar,
                    get_ordered_task_behaviour().aat_min_time + fixed(task_behaviour.optimise_targets_margin));

    if (task_behaviour.optimise_targets_bearing) {
      if (task_points[active_task_point]->GetType() == TaskPoint::AAT) {
        AATPoint *ap = (AATPoint *)task_points[active_task_point];
        // very nasty hack
        TaskOptTarget tot(task_points, active_task_point, state,
                          task_behaviour.glide, glide_polar,
                          *ap, task_projection, taskpoint_start);
        tot.search(fixed(0.5));
      }
    }
    retval = true;
  }
  
  return retval;
}

bool 
OrderedTask::UpdateSample(const AircraftState &state,
                          const GlidePolar &glide_polar,
                           const bool full_update)
{
  return true;
}

// TASK

void
OrderedTask::set_neighbours(unsigned position)
{
  OrderedTaskPoint* prev = NULL;
  OrderedTaskPoint* next = NULL;

  if (!task_points[position])
    // nothing to do if this is deleted
    return;

  if (position >= task_points.size())
    // nothing to do
    return;

  if (position > 0)
    prev = task_points[position - 1];

  if (position + 1 < task_points.size())
    next = task_points[position + 1];

  task_points[position]->set_neighbours(prev, next);

  if (position==0) {
    for (auto i = optional_start_points.begin(),
           end = optional_start_points.end(); i != end; ++i)
      (*i)->set_neighbours(prev, next);
  }
}

bool
OrderedTask::CheckTask() const
{
  return this->GetFactory().Validate();
}

AATPoint*
OrderedTask::GetAATTaskPoint(unsigned TPindex) const
{
 if (TPindex > task_points.size() - 1) {
   return NULL;
 }
 if (task_points[TPindex]) {
    if (task_points[TPindex]->GetType() == TaskPoint::AAT)
      return (AATPoint*) task_points[TPindex];
    else
      return (AATPoint*)NULL;
 }
 return NULL;
}

bool
OrderedTask::scan_start_finish()
{
  /// @todo also check there are not more than one start/finish point
  if (!task_points.size()) {
    taskpoint_start = NULL;
    taskpoint_finish = NULL;
    return false;
  }

  taskpoint_start = task_points[0]->GetType() == TaskPoint::START
    ? (StartPoint *)task_points[0]
    : NULL;

  taskpoint_finish = task_points.size() > 1 &&
    task_points[task_points.size() - 1]->GetType() == TaskPoint::FINISH
    ? (FinishPoint *)task_points[task_points.size() - 1]
    : NULL;

  return HasStart() && HasFinish();
}

void
OrderedTask::erase(const unsigned index)
{
  delete task_points[index];
  task_points.erase(task_points.begin() + index);
}

void
OrderedTask::erase_optional_start(const unsigned index)
{
  delete optional_start_points[index];
  optional_start_points.erase(optional_start_points.begin() + index);
}

bool
OrderedTask::Remove(const unsigned position)
{
  if (position >= task_points.size())
    return false;

  if (active_task_point > position ||
      (active_task_point > 0 && active_task_point == task_points.size() - 1))
    active_task_point--;

  erase(position);

  set_neighbours(position);
  if (position)
    set_neighbours(position - 1);

  update_geometry();
  return true;
}

bool
OrderedTask::RemoveOptionalStart(const unsigned position)
{
  if (position >= optional_start_points.size())
    return false;

  erase_optional_start(position);

  if (task_points.size()>1)
    set_neighbours(0);

  update_geometry();
  return true;
}

bool 
OrderedTask::Append(const OrderedTaskPoint &new_tp)
{
  if (/* is the new_tp allowed in this context? */
      (!task_points.empty() && !new_tp.predecessor_allowed()) ||
      /* can a tp be appended after the last one? */
      (task_points.size() >= 1 && !task_points[task_points.size() - 1]->successor_allowed()))
    return false;

  task_points.push_back(new_tp.clone(task_behaviour, m_ordered_behaviour));
  if (task_points.size() > 1)
    set_neighbours(task_points.size() - 2);
  else {
    // give it a value when we have one tp so it is not uninitialised
    m_location_min_last = new_tp.GetLocation();
  }

  set_neighbours(task_points.size() - 1);
  update_geometry();
  return true;
}

bool 
OrderedTask::AppendOptionalStart(const OrderedTaskPoint &new_tp)
{
  optional_start_points.push_back(new_tp.clone(task_behaviour, m_ordered_behaviour));
  if (task_points.size() > 1)
    set_neighbours(0);
  update_geometry();
  return true;
}

bool 
OrderedTask::Insert(const OrderedTaskPoint &new_tp, const unsigned position)
{
  if (position >= task_points.size())
    return Append(new_tp);

  if (/* is the new_tp allowed in this context? */
      (position > 0 && !new_tp.predecessor_allowed()) ||
      !new_tp.successor_allowed() ||
      /* can a tp be inserted at this position? */
      (position > 0 && !task_points[position - 1]->successor_allowed()) ||
      !task_points[position]->predecessor_allowed())
    return false;

  if (active_task_point >= position)
    active_task_point++;

  task_points.insert(task_points.begin() + position,
             new_tp.clone(task_behaviour, m_ordered_behaviour));

  if (position)
    set_neighbours(position - 1);

  set_neighbours(position);
  set_neighbours(position + 1);

  update_geometry();
  return true;
}

bool 
OrderedTask::Replace(const OrderedTaskPoint &new_tp, const unsigned position)
{
  if (position >= task_points.size())
    return false;

  if (task_points[position]->equals(&new_tp))
    // nothing to do
    return true;

  /* is the new_tp allowed in this context? */
  if ((position > 0 && !new_tp.predecessor_allowed()) ||
      (position + 1 < task_points.size() && !new_tp.successor_allowed()))
    return false;

  delete task_points[position];
  task_points[position] = new_tp.clone(task_behaviour, m_ordered_behaviour);

  if (position)
    set_neighbours(position - 1);

  set_neighbours(position);
  if (position + 1 < task_points.size())
    set_neighbours(position + 1);

  update_geometry();
  return true;
}


bool 
OrderedTask::ReplaceOptionalStart(const OrderedTaskPoint &new_tp,
                                  const unsigned position)
{
  if (position >= optional_start_points.size())
    return false;

  if (optional_start_points[position]->equals(&new_tp))
    // nothing to do
    return true;

  delete optional_start_points[position];
  optional_start_points[position] = new_tp.clone(task_behaviour, m_ordered_behaviour);

  set_neighbours(0);
  update_geometry();
  return true;
}


void 
OrderedTask::SetActiveTaskPoint(unsigned index)
{
  if (index < task_points.size()) {
    if (active_task_point != index)
      task_advance.set_armed(false);

    active_task_point = index;
  } else if (task_points.empty()) {
    active_task_point = 0;
  }
}

TaskWaypoint*
OrderedTask::GetActiveTaskPoint() const
{
  if (active_task_point < task_points.size())
    return task_points[active_task_point];

  return NULL;
}

bool 
OrderedTask::IsValidTaskPoint(const int index_offset) const
{
  unsigned index = active_task_point + index_offset;
  return (index < task_points.size());
}


void
OrderedTask::GlideSolutionRemaining(const AircraftState &aircraft,
                                      const GlidePolar &polar,
                                      GlideResult &total,
                                      GlideResult &leg)
{
  TaskMacCreadyRemaining tm(task_points, active_task_point,
                            task_behaviour.glide, polar);
  total = tm.glide_solution(aircraft);
  leg = tm.get_active_solution();
}

void
OrderedTask::GlideSolutionTravelled(const AircraftState &aircraft,
                                    const GlidePolar &glide_polar,
                                      GlideResult &total,
                                      GlideResult &leg)
{
  TaskMacCreadyTravelled tm(task_points, active_task_point,
                            task_behaviour.glide, glide_polar);
  total = tm.glide_solution(aircraft);
  leg = tm.get_active_solution();
}

void
OrderedTask::GlideSolutionPlanned(const AircraftState &aircraft,
                                  const GlidePolar &glide_polar,
                                    GlideResult &total,
                                    GlideResult &leg,
                                    DistanceStat &total_remaining_effective,
                                    DistanceStat &leg_remaining_effective,
                                    const GlideResult &solution_remaining_total,
                                    const GlideResult &solution_remaining_leg)
{
  TaskMacCreadyTotal tm(task_points, active_task_point,
                        task_behaviour.glide, glide_polar);
  total = tm.glide_solution(aircraft);
  leg = tm.get_active_solution();

  if (solution_remaining_total.IsOk())
    total_remaining_effective.set_distance(tm.effective_distance(solution_remaining_total.time_elapsed));
  else
    total_remaining_effective.Reset();

  if (solution_remaining_leg.IsOk())
    leg_remaining_effective.set_distance(tm.effective_leg_distance(solution_remaining_leg.time_elapsed));
  else
    leg_remaining_effective.Reset();
}

// Auxiliary glide functions

fixed
OrderedTask::CalcRequiredGlide(const AircraftState &aircraft,
                               const GlidePolar &glide_polar) const
{
  TaskGlideRequired bgr(task_points, active_task_point, aircraft,
                        task_behaviour.glide, glide_polar);
  return bgr.search(fixed_zero);
}

bool
OrderedTask::CalcBestMC(const AircraftState &aircraft,
                        const GlidePolar &glide_polar,
                        fixed &best) const
{
  // note setting of lower limit on mc
  TaskBestMc bmc(task_points, active_task_point, aircraft,
                 task_behaviour.glide, glide_polar);
  return bmc.search(glide_polar.GetMC(), best);
}


bool
OrderedTask::allow_incremental_boundary_stats(const AircraftState &aircraft) const
{
  if (!active_task_point)
    return false;
  assert(task_points[active_task_point]);
  bool in_sector = task_points[active_task_point]->IsInSector(aircraft);
  if (active_task_point>0) {
    in_sector |= task_points[active_task_point-1]->IsInSector(aircraft);
  }
  return (task_points[active_task_point]->IsBoundaryScored() || !in_sector);
}

bool
OrderedTask::CalcCruiseEfficiency(const AircraftState &aircraft,
                                  const GlidePolar &glide_polar,
                                  fixed &val) const
{
  if (allow_incremental_boundary_stats(aircraft)) {
    TaskCruiseEfficiency bce(task_points, active_task_point, aircraft,
                             task_behaviour.glide, glide_polar);
    val = bce.search(fixed_one);
    return true;
  } else {
    val = fixed_one;
    return false;
  }
}

bool 
OrderedTask::CalcEffectiveMC(const AircraftState &aircraft,
                             const GlidePolar &glide_polar,
                             fixed &val) const
{
  if (allow_incremental_boundary_stats(aircraft)) {
    TaskEffectiveMacCready bce(task_points, active_task_point, aircraft,
                               task_behaviour.glide, glide_polar);
    val = bce.search(glide_polar.GetMC());
    return true;
  } else {
    val = glide_polar.GetMC();
    return false;
  }
}


fixed
OrderedTask::CalcMinTarget(const AircraftState &aircraft,
                           const GlidePolar &glide_polar,
                           const fixed t_target)
{
  if (stats.distance_max > stats.distance_min) {
    // only perform scan if modification is possible
    const fixed t_rem = max(fixed_zero, t_target - stats.total.time_elapsed);

    TaskMinTarget bmt(task_points, active_task_point, aircraft,
                      task_behaviour.glide, glide_polar,
                      t_rem, taskpoint_start);
    fixed p = bmt.search(fixed_zero);
    return p;
  }

  return fixed_zero;
}


fixed 
OrderedTask::CalcGradient(const AircraftState &state) const
{
  if (task_points.size() < 1)
    return fixed_zero;

  // Iterate through remaining turnpoints
  fixed distance = fixed_zero;
  for (auto i = task_points.cbegin(), end = task_points.cend(); i != end; ++i)
    // Sum up the leg distances
    distance += (*i)->GetVectorRemaining(state.location).distance;

  if (!distance)
    return fixed_zero;

  // Calculate gradient to the last turnpoint of the remaining task
  return (state.altitude - task_points[task_points.size() - 1]->GetElevation()) / distance;
}

// Constructors/destructors

OrderedTask::~OrderedTask()
{
  for (auto v = task_points.begin(); v != task_points.end();) {
    delete *v;
    task_points.erase(v);
  }

  for (auto v = optional_start_points.begin(); v != optional_start_points.end();) {
    delete *v;
    optional_start_points.erase(v);
  }

  delete active_factory;

#if defined(__clang__) || GCC_VERSION >= 40700
  /* no, TaskDijkstra{Min,Max} really don't need a virtual
     destructor */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#endif

  delete dijkstra_min;
  delete dijkstra_max;

#if defined(__clang__) || GCC_VERSION >= 40700
#pragma GCC diagnostic pop
#endif
}

OrderedTask::OrderedTask(const TaskBehaviour &tb):
  AbstractTask(ORDERED, tb),
  taskpoint_start(NULL),
  taskpoint_finish(NULL),
  factory_mode(TaskFactoryType::RACING),
  active_factory(NULL),
  m_ordered_behaviour(tb.ordered_defaults),
  task_advance(m_ordered_behaviour),
  dijkstra_min(NULL), dijkstra_max(NULL)
{
  active_factory = new RTTaskFactory(*this, task_behaviour);
  active_factory->UpdateOrderedTaskBehaviour(m_ordered_behaviour);
}

static void
Visit(const OrderedTask::OrderedTaskPointVector &points,
      TaskPointConstVisitor &visitor)
{
  for (auto it = points.begin(), end = points.end(); it != end; ++it)
    visitor.Visit(**it);
}

static void
VisitReverse(const OrderedTask::OrderedTaskPointVector &points,
             TaskPointConstVisitor &visitor)
{
  for (auto it = points.rbegin(), end = points.rend(); it != end; ++it)
    visitor.Visit(**it);
}

void 
OrderedTask::AcceptTaskPointVisitor(TaskPointConstVisitor& visitor, const bool reverse) const
{
  if (!reverse) {
    Visit(task_points, visitor);
  } else {
    VisitReverse(task_points, visitor);
  }
}

void 
OrderedTask::AcceptStartPointVisitor(TaskPointConstVisitor& visitor, const bool reverse) const
{
  if (!reverse) {
    Visit(optional_start_points, visitor);
  } else {
    VisitReverse(optional_start_points, visitor);
  }
}

static void
ResetPoints(OrderedTask::OrderedTaskPointVector &points)
{
  for (auto it = points.begin(), end = points.end(); it != end; ++it)
    (*it)->Reset();
}

void
OrderedTask::Reset()
{  
  /// @todo also reset data in this class e.g. stats?
  ResetPoints(task_points);
  ResetPoints(optional_start_points);

  AbstractTask::Reset();
  stats.task_finished = false;
  stats.task_started = false;
  task_advance.reset();
  SetActiveTaskPoint(0);
}

bool 
OrderedTask::TaskFinished() const
{
  if (taskpoint_finish)
    return (taskpoint_finish->HasEntered());

  return false;
}

bool 
OrderedTask::TaskStarted(bool soft) const
{
  if (taskpoint_start) {
    // have we really started?
    if (taskpoint_start->HasExited()) 
      return true;

    // if soft starts allowed, consider started if we progressed to next tp
    if (soft && (active_task_point>0))
      return true;
  }

  return false;
}

/**
 * Test whether two points (as previous search locations) are significantly
 * different to warrant a new search
 *
 * @param a1 First point to compare
 * @param a2 Second point to compare
 * @param dist_threshold Threshold distance for significance
 *
 * @return True if distance is significant
 */
gcc_pure
static bool
distance_is_significant(const SearchPoint &a1, const SearchPoint &a2,
                        const unsigned dist_threshold = 1)
{
  return a1.FlatSquareDistance(a2) > (dist_threshold * dist_threshold);
}

bool 
OrderedTask::distance_is_significant(const GeoPoint &location,
                                     const GeoPoint &location_last) const
{
  SearchPoint a1(location, task_projection);
  SearchPoint a2(location_last, task_projection);
  return ::distance_is_significant(a1, a2);
}


const SearchPointVector& 
OrderedTask::get_tp_search_points(unsigned tp) const 
{
  return task_points[tp]->GetSearchPoints();
}

void 
OrderedTask::set_tp_search_min(unsigned tp, const SearchPoint &sol) 
{
  if (!tp && !task_points[0]->HasExited())
    return;

  task_points[tp]->SetSearchMin(sol);
}

void 
OrderedTask::set_tp_search_achieved(unsigned tp, const SearchPoint &sol) 
{
  if (task_points[tp]->HasSampled())
    set_tp_search_min(tp, sol);
}

void 
OrderedTask::set_tp_search_max(unsigned tp, const SearchPoint &sol) 
{
  task_points[tp]->SetSearchMax(sol);
}

unsigned 
OrderedTask::TaskSize() const 
{
  return task_points.size();
}

unsigned 
OrderedTask::GetActiveIndex() const
{
  return active_task_point;
}

void
OrderedTask::update_start_transition(const AircraftState &state, OrderedTaskPoint& start)
{
  if (active_task_point == 0) {
    // find boundary point that produces shortest
    // distance from state to that point to next tp point
    taskpoint_start->find_best_start(state, *task_points[1], task_projection);
  } else if (!start.HasExited() && !start.IsInSector(state)) {
    start.Reset();
    // reset on invalid transition to outside
    // point to nominal start point
  }
  // @todo: modify this for optional start?
}

AircraftState 
OrderedTask::GetStartState() const
{
  if (HasStart() && TaskStarted())
    return taskpoint_start->GetEnteredState();

  // @todo: modify this for optional start?

  AircraftState null_state;
  return null_state;
}

AircraftState 
OrderedTask::GetFinishState() const
{
  if (HasFinish() && TaskFinished())
    return taskpoint_finish->GetEnteredState();

  AircraftState null_state;
  return null_state;
}

bool
OrderedTask::HasTargets() const
{
  for (auto i = task_points.cbegin(), end = task_points.cend(); i != end; ++i)
    if ((*i)->HasTarget())
      return true;

  return false;
}

fixed
OrderedTask::GetFinishHeight() const
{
  if (taskpoint_finish)
    return taskpoint_finish->GetElevation();

  return fixed_zero;
}

GeoPoint 
OrderedTask::GetTaskCenter(const GeoPoint& fallback_location) const
{
  if (!HasStart() || !task_points[0])
    return fallback_location;

  return task_projection.get_center();
}

fixed 
OrderedTask::GetTaskRadius(const GeoPoint& fallback_location) const
{ 
  if (!HasStart() || !task_points[0])
    return fixed_zero;

  return task_projection.ApproxRadius();
}

OrderedTask* 
OrderedTask::Clone(const TaskBehaviour &tb) const
{
  OrderedTask* new_task = new OrderedTask(tb);

  new_task->m_ordered_behaviour = m_ordered_behaviour;

  new_task->SetFactory(factory_mode);
  for (auto i = task_points.cbegin(), end = task_points.cend(); i != end; ++i)
    new_task->Append(**i);

  for (auto i = optional_start_points.cbegin(),
         end = optional_start_points.cend(); i != end; ++i)
    new_task->AppendOptionalStart(**i);

  new_task->active_task_point = active_task_point;
  new_task->update_geometry();
  return new_task;
}

void
OrderedTask::CheckDuplicateWaypoints(Waypoints& waypoints,
                                     OrderedTaskPointVector& points,
                                     const bool is_task)
{
  for (auto begin = points.cbegin(), end = points.cend(), i = begin;
       i != end; ++i) {
    const Waypoint &wp =
      waypoints.CheckExistsOrAppend((*i)->GetWaypoint());

    const OrderedTaskPoint *new_tp = (*i)->clone(task_behaviour,
                                                 m_ordered_behaviour,
                                                 &wp);
    if (is_task)
      Replace(*new_tp, std::distance(begin, i));
    else
      ReplaceOptionalStart(*new_tp, std::distance(begin, i));
    delete new_tp;
  }
}

void
OrderedTask::CheckDuplicateWaypoints(Waypoints& waypoints)
{
  CheckDuplicateWaypoints(waypoints, task_points, true);
  CheckDuplicateWaypoints(waypoints, optional_start_points, false);
}

bool
OrderedTask::Commit(const OrderedTask& that)
{
  bool modified = false;

  // change mode to that one 
  SetFactory(that.factory_mode);

  // copy across behaviour
  m_ordered_behaviour = that.m_ordered_behaviour;

  // remove if that task is smaller than this one
  while (TaskSize() > that.TaskSize()) {
    Remove(TaskSize() - 1);
    modified = true;
  }

  // ensure each task point made identical
  for (unsigned i = 0; i < that.TaskSize(); ++i) {
    if (i >= TaskSize()) {
      // that task is larger than this
      Append(*that.task_points[i]);
      modified = true;
    } else if (!task_points[i]->equals(that.task_points[i])) {
      // that task point is changed
      Replace(*that.task_points[i], i);
      modified = true;
    }
  }

  // remove if that optional start list is smaller than this one
  while (optional_start_points.size() > that.optional_start_points.size()) {
    RemoveOptionalStart(optional_start_points.size() - 1);
    modified = true;
  }

  // ensure each task point made identical
  for (unsigned i = 0; i < that.optional_start_points.size(); ++i) {
    if (i >= optional_start_points.size()) {
      // that task is larger than this
      AppendOptionalStart(*that.optional_start_points[i]);
      modified = true;
    } else if (!optional_start_points[i]->equals(that.optional_start_points[i])) {
      // that task point is changed
      ReplaceOptionalStart(*that.optional_start_points[i], i);
      modified = true;
    }
  }

  if (modified)
    update_geometry();
    // @todo also re-scan task sample state,
    // potentially resetting task

  return modified;
}

bool
OrderedTask::RelocateOptionalStart(const unsigned position, const Waypoint& waypoint)
{
  if (position >= optional_start_points.size())
    return false;

  OrderedTaskPoint *new_tp = optional_start_points[position]->clone(task_behaviour,
                                                  m_ordered_behaviour,
                                                  &waypoint);
  delete optional_start_points[position];
  optional_start_points[position]= new_tp;
  return true;
}

bool
OrderedTask::Relocate(const unsigned position, const Waypoint& waypoint)
{
  if (position >= TaskSize())
    return false;

  OrderedTaskPoint *new_tp = task_points[position]->clone(task_behaviour,
                                                  m_ordered_behaviour,
                                                  &waypoint);
  bool success = Replace(*new_tp, position);
  delete new_tp;
  return success;
}

TaskFactoryType
OrderedTask::SetFactory(const TaskFactoryType the_factory)
{
  // detect no change
  if (factory_mode == the_factory)
    return factory_mode;

  if (the_factory != TaskFactoryType::MIXED) {
    // can switch from anything to mixed, otherwise need reset
    Reset();

    /// @todo call into task_events to ask if reset is desired on
    /// factory change
  }
  factory_mode = the_factory;

  delete active_factory;

  switch (factory_mode) {
  case TaskFactoryType::RACING:
    active_factory = new RTTaskFactory(*this, task_behaviour);
    break;
  case TaskFactoryType::FAI_GENERAL:
    active_factory = new FAITaskFactory(*this, task_behaviour);
    break;
  case TaskFactoryType::FAI_TRIANGLE:
    active_factory = new FAITriangleTaskFactory(*this, task_behaviour);
    break;
  case TaskFactoryType::FAI_OR:
    active_factory = new FAIORTaskFactory(*this, task_behaviour);
    break;
  case TaskFactoryType::FAI_GOAL:
    active_factory = new FAIGoalTaskFactory(*this, task_behaviour);
    break;
  case TaskFactoryType::AAT:
    active_factory = new AATTaskFactory(*this, task_behaviour);
    break;
  case TaskFactoryType::MIXED:
    active_factory = new MixedTaskFactory(*this, task_behaviour);
    break;
  case TaskFactoryType::TOURING:
    active_factory = new TouringTaskFactory(*this, task_behaviour);
    break;
  };
  active_factory->UpdateOrderedTaskBehaviour(m_ordered_behaviour);

  return factory_mode;
}

void 
OrderedTask::set_ordered_task_behaviour(const OrderedTaskBehaviour& ob)
{
  m_ordered_behaviour = ob;
}

bool 
OrderedTask::IsScored() const
{
  return m_ordered_behaviour.task_scored;
}

std::vector<TaskFactoryType>
OrderedTask::GetFactoryTypes(bool all) const
{
  /// @todo: check transform types if all=false
  std::vector<TaskFactoryType> f_list;
  f_list.push_back(TaskFactoryType::RACING);
  f_list.push_back(TaskFactoryType::AAT);
  f_list.push_back(TaskFactoryType::FAI_GENERAL);
  return f_list;
}

void 
OrderedTask::Clear()
{
  while (task_points.size())
    erase(0);

  while (optional_start_points.size())
    erase_optional_start(0);

  Reset();
  m_ordered_behaviour = task_behaviour.ordered_defaults;
}

OrderedTaskPoint* 
OrderedTask::get_tp(const unsigned position)
{
  if (position >= TaskSize())
    return NULL;

  return task_points[position];
}

FlatBoundingBox 
OrderedTask::get_bounding_box(const GeoBounds& bounds) const
{
  if (!TaskSize()) {
    // undefined!
    return FlatBoundingBox(FlatGeoPoint(0,0),FlatGeoPoint(0,0));
  }
  FlatGeoPoint ll = task_projection.project(GeoPoint(bounds.west, bounds.south));
  FlatGeoPoint lr = task_projection.project(GeoPoint(bounds.east, bounds.south));
  FlatGeoPoint ul = task_projection.project(GeoPoint(bounds.west, bounds.north));
  FlatGeoPoint ur = task_projection.project(GeoPoint(bounds.east, bounds.north));
  FlatGeoPoint fmin(min(ll.Longitude,ul.Longitude), min(ll.Latitude,lr.Latitude));
  FlatGeoPoint fmax(max(lr.Longitude,ur.Longitude), max(ul.Latitude,ur.Latitude));
  // note +/- 1 to ensure rounding keeps bb valid 
  fmin.Longitude-= 1; fmin.Latitude-= 1;
  fmax.Longitude+= 1; fmax.Latitude+= 1;
  return FlatBoundingBox (fmin, fmax);
}

FlatBoundingBox 
OrderedTask::get_bounding_box(const GeoPoint& point) const
{
  if (!TaskSize()) {
    // undefined!
    return FlatBoundingBox(FlatGeoPoint(0,0),FlatGeoPoint(0,0));
  }
  return FlatBoundingBox (task_projection.project(point), 1);
}

void 
OrderedTask::RotateOptionalStarts()
{
  if (!TaskSize())
    return;
  if (!optional_start_points.size()) 
    return;

  select_optional_start(0);
}

void
OrderedTask::select_optional_start(unsigned pos) 
{
  assert(pos< optional_start_points.size());

  // put task start onto end
  optional_start_points.push_back(task_points[0]);
  // set task start from top optional item
  task_points[0] = optional_start_points[pos];
  // remove top optional item from list
  optional_start_points.erase(optional_start_points.begin()+pos);

  // update neighbour links
  set_neighbours(0);
  if (task_points.size()>1)
    set_neighbours(1);

  // we've changed the task, so update geometry
  update_geometry();
}

const OrderedTaskPoint *
OrderedTask::get_optional_start(unsigned pos) const
{
  if (pos >= optional_start_points.size())
    return NULL;

  return optional_start_points[pos];
}

void
OrderedTask::update_summary(TaskSummary& ordered_summary) const
{
  ordered_summary.clear();

  ordered_summary.active = active_task_point;

  for (auto begin = task_points.cbegin(), end = task_points.cend(), i = begin;
       i != end; ++i) {
    const OrderedTaskPoint &tp = **i;

    TaskSummaryPoint tsp;
    tsp.d_planned = tp.GetVectorPlanned().distance;
    if (i == begin) {
      tsp.achieved = tp.HasExited();
    } else {
      tsp.achieved = tp.HasSampled();
    }
    ordered_summary.append(tsp);
  }

  if (stats.total.remaining.IsDefined() && stats.total.planned.IsDefined())
    ordered_summary.update(stats.total.remaining.get_distance(),
                           stats.total.planned.get_distance());
}

unsigned 
OrderedTask::OptionalStartsSize() const {
  return optional_start_points.size();
}
