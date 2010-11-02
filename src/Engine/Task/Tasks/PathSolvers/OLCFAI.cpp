#include "OLCFAI.hpp"
#include "Navigation/Flat/FlatRay.hpp"

/*
 @todo potential to use 3d convex hull to speed search

 2nd point inclusion rules:
   if min leg length is 25%, max is 45%
   pmin = 0.25
   pmax = 0.45
   ptot = 2*pmin+pmax+0.05
   with 2 legs:
      ptot = pmin+pmax


  0: start
  1: first leg start
  2: second leg start
  3: third leg start
  4: end
*/

OLCFAI::OLCFAI(const TracePointVector &_trace):
  ContestDijkstra(_trace, 3, 1000),
  is_closed(false),
  is_complete(false) {}


fixed
OLCFAI::leg_distance(const unsigned index) const
{  
  // leg 0: 1-2
  // leg 1: 2-3
  // leg 2: 3-1

  const GeoPoint &p_start = solution[index+1].get_location();
  const GeoPoint &p_dest = (index < 2)? 
    solution[index+2].get_location():
    solution[1].get_location();

  return p_start.distance(p_dest);
}


bool 
OLCFAI::triangle_closed() const
{
  // note this may fail if resolution of sampled trace is too low
  static const fixed fixed_1000(1000);
  static const ScanTaskPoint end(0, n_points-1);
  
  return solution[0].get_location().distance(get_point(end).get_location())
    <= fixed_1000;
}


bool 
OLCFAI::finish_satisfied(const ScanTaskPoint &sp) const
{
  //    && triangle_closed()
  return true;
}


unsigned 
OLCFAI::second_leg_distance(const ScanTaskPoint &destination) const
{
  // this is a heuristic to remove invalid triangles
  // we do as much of this in flat projection for speed

  const unsigned df_1 = solution[1].flat_distance(solution[2])/32;
  const unsigned df_2 = solution[2].flat_distance(get_point(destination))/32;
  const unsigned df_3 = get_point(destination).flat_distance(solution[1])/32;
  const unsigned df_total = df_1+df_2+df_3;

  // require some distance!
  if (df_total<20) {
    return 0;
  }
  const unsigned shortest = min(df_1, min(df_2, df_3));

  // require all legs to have distance
  if (!shortest) {
    return 0;
  }
  if (shortest*4<df_total) { // fails min < 25% worst-case rule!
    return 0;
  }

  const unsigned d = df_3+df_1;
  if (shortest*25>=df_total*7) { 
    // passes min > 28% rule,
    // this automatically means we pass max > 45% worst-case
    return d;
  }

  const unsigned longest = max(df_1, max(df_2, df_3));
  if (longest*20>df_total*9) { // fails max > 45% worst-case rule!
    return 0;
  }

  // passed basic tests, now detailed ones

  // find accurate min leg distance
  fixed leg(0);
  if (df_1 == shortest) {
    leg = leg_distance(0);
  } else if (df_2 == shortest) {
    leg = leg_distance(1);
  } else if (df_3 == shortest) {
    leg = leg_distance(2);
  }

  // estimate total distance by scaling.
  // this is a slight approximation, but saves having to do
  // three accurate distance calculations.

  const fixed d_total(df_total*leg/shortest);
  if (d_total>=fixed(500000)) {
    // long distance, ok that it failed 28% rule
    return d;
  }

  return 0;
}


void 
OLCFAI::add_edges(DijkstraTaskPoint &dijkstra, const ScanTaskPoint& origin)
{
  ScanTaskPoint destination(origin.first+1, origin.second+1);

  switch (destination.first) {
  case 1:
    is_complete = false;
    // don't award any points for first partial leg
    for (; destination.second < n_points; ++destination.second) {
      dijkstra.link(destination, origin, 1);
    }
    break;
  case 2:
    ContestDijkstra::add_edges(dijkstra, origin);
    break;
  case 3:
    find_solution(dijkstra, origin);

    // give first leg points to penultimate node
    for (; destination.second < n_points; ++destination.second) {
      solution[3] = get_point(destination);
      const unsigned d = second_leg_distance(destination);
      if (d) {
        dijkstra.link(destination, origin, get_weighting(origin.first)*d);
        is_complete = true;
      }
    }
    break;
  default:
    assert(1);
  }
}


fixed
OLCFAI::calc_distance() const
{
  if (is_complete) {
    return leg_distance(0)+leg_distance(1)+leg_distance(2);
  } else {
    return fixed_zero;
  }
}


fixed
OLCFAI::calc_time() const
{
  const ScanTaskPoint destination(0, n_points-1);
  return fixed(get_point(destination).time - solution[0].time);
}


fixed
OLCFAI::calc_score() const
{
  // @todo: apply handicap
  return calc_distance()*fixed(0.0003);
}
