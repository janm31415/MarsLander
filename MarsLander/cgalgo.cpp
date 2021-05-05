//#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline") //Optimization flags
//#pragma GCC option("arch=native","tune=native","no-zero-upper") //Enable AVX
//#pragma GCC target("avx2")  //Enable AVX

/*
 Based on the following blog post
 https://www.codingame.com/blog/genetic-algorithm-mars-lander/
 */

#include "cgalgo.h"

template <class T>
inline T sqr(T a) { return a*a; }

std::vector<vec2<int>> surface_points;
RKISS rkiss;
int landing_zone_x0, landing_zone_x1, landing_zone_y;
simulation_data simdata;

void find_landingzone(int& x0, int& x1, int& y, const std::vector<vec2<int>>& pts) {
  x0 = x1 = 0;
  for (int i = 1; i < pts.size(); ++i) {
    if (pts[i].y == pts[i-1].y && std::abs(pts[i].x-pts[i-1].x)>=1000) {
      x0 = pts[i].x;
      x1 = pts[i-1].x;
      y = pts[i].y;
      if (x1<x0)
        std::swap(x0,x1);
      return;
    }
  }
}

//#define OUTOFBOUNDSCLAMPING

int clamp_angle(int angle, int R) {
  if (angle>R+maximum_angle_rotation)
    angle = R+maximum_angle_rotation;
  else if (angle < R-maximum_angle_rotation)
    angle = R-maximum_angle_rotation;
#if defined(OUTOFBOUNDSCLAMPING)
  if (angle>maximum_angle)
    angle = maximum_angle;
  if (angle<-maximum_angle)
    angle = -maximum_angle;
#endif
  return angle;
}

int clamp_thrust(int thrust, int P) {
  if (thrust > P+maximum_thrust_change)
    thrust = P+maximum_thrust_change;
  if (thrust < P-maximum_thrust_change)
    thrust = P-maximum_thrust_change;
#if defined(OUTOFBOUNDSCLAMPING)
  if (thrust < 0)
    thrust = 0;
  if (thrust > maximum_thrust)
    thrust = maximum_thrust;
#endif
  return thrust;
}

int clamp_angle(int R) {
  return R<-maximum_angle?-maximum_angle:R>maximum_angle?maximum_angle:R;
}

int clamp_thrust(int P) {
  return P<0?0:P>maximum_thrust?maximum_thrust:P;
}

gene generate_random_gene() {
  gene g;
  g.angle = (int)(rkiss.rand64()%(2*maximum_angle_rotation+1))-maximum_angle_rotation;
  g.thrust = (int)(rkiss.rand64()%(2*maximum_thrust_change+1))-maximum_thrust_change;
  return g;
}

chromosome generate_random_chromosome() {
  chromosome c;
  c.reserve(chromosome_size);
  for (int i = 0; i < chromosome_size; ++i) {
    c.push_back(generate_random_gene());
  }
  return c;
}

population generate_random_population() {
  population p;
  p.reserve(population_size);
  for (int i = 0; i < population_size; ++i) {
    p.emplace_back(generate_random_chromosome());
  }
  return p;
}

void simulate(simulation_data& sd, int angle, int thrust) {
  const vec2<float> g(0, -3.711f);
  angle = clamp_angle(angle, sd.R);
  float ang = (float)angle*(float)pi/180.f;
  vec2<float> f(std::cos(pi/2.f+ang), std::sin(pi/2.f+ang));
  thrust = clamp_thrust(thrust, sd.P);
  f = f*(float)thrust;
  sd.P = thrust;
  sd.R = angle;
  sd.F -= thrust;
  vec2<float> a = g+f;
  sd.p = sd.p + sd.v + a*0.5f;
  sd.v = sd.v + a;
}

void read_input(std::stringstream& strcin, std::stringstream& strerr) {
  surface_points.clear();
  int N; // the number of points used to draw the surface of Mars.
  strcin >> N; strcin.ignore();
  strerr << N << std::endl;
  for (int i = 0; i < N; i++) {
    int landX; // X coordinate of a surface point. (0 to 6999)
    int landY; // Y coordinate of a surface point. By linking all the points together in a sequential fashion, you form the surface of Mars.
    strcin >> landX >> landY; strcin.ignore();
    strerr << landX << " " << landY << std::endl;
    surface_points.emplace_back(landX, landY);
  }
  find_landingzone(landing_zone_x0, landing_zone_x1, landing_zone_y, surface_points);
  
  int X;
  int Y;
  int HS; // the horizontal speed (in m/s), can be negative.
  int VS; // the vertical speed (in m/s), can be negative.
  int F; // the quantity of remaining fuel in liters.
  int R; // the rotation angle in degrees (-90 to 90).
  int P; // the thrust power (0 to 4).
  strcin >> X >> Y >> HS >> VS >> F >> R >> P;
  
  simdata.p = vec2<float>(X,Y);
  simdata.v = vec2<float>(HS,VS);
  simdata.F = F;
  simdata.R = R;
  simdata.P = P;
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(const vec2<int>& p, const vec2<int>& q, const vec2<int>& r)
{
  if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
      q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y))
    return true;
  
  return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(const vec2<int>& p, const vec2<int>& q, const vec2<int>& r)
{
  // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
  // for details of below formula.
  int val = (q.y - p.y) * (r.x - q.x) -
  (q.x - p.x) * (r.y - q.y);
  
  if (val == 0) return 0;  // colinear
  
  return (val > 0)? 1: 2; // clock or counterclock wise
}

bool intersects(const vec2<int>& p1, const vec2<int>& q1, const vec2<int>& p2, const vec2<int>& q2) {
  // Find the four orientations needed for general and
  // special cases
  int o1 = orientation(p1, q1, p2);
  int o2 = orientation(p1, q1, q2);
  int o3 = orientation(p2, q2, p1);
  int o4 = orientation(p2, q2, q1);
  
  // General case
  if (o1 != o2 && o3 != o4)
    return true;
  
  // Special Cases
  // p1, q1 and p2 are colinear and p2 lies on segment p1q1
  if (o1 == 0 && onSegment(p1, p2, q1)) return true;
  
  // p1, q1 and q2 are colinear and q2 lies on segment p1q1
  if (o2 == 0 && onSegment(p1, q2, q1)) return true;
  
  // p2, q2 and p1 are colinear and p1 lies on segment p2q2
  if (o3 == 0 && onSegment(p2, p1, q2)) return true;
  
  // p2, q2 and q1 are colinear and q1 lies on segment p2q2
  if (o4 == 0 && onSegment(p2, q1, q2)) return true;
  
  return false; // Doesn't fall in any of the above cases
}

bool crashed_or_landed(int X, int Y, int PX, int PY) {
  /*
   if (X==PX)
   return Y<=heights[X];
   if (X>PX) {
   std::swap(X,PX);
   std::swap(Y,PY);
   }
   for (int x = X; x <= PX; ++x) {
   double alpha = (double)(x-X)/(double)(PX-X);
   double y = Y+alpha*(PY-Y);
   if ((int)std::round(y) <= heights[x])
   return true;
   }
   return false;
   */
  vec2<int> p2(X,Y), q2(PX,PY);
  for (int i = 1; i < surface_points.size(); ++i) {
    if (intersects(surface_points[i-1], surface_points[i], p2, q2))
      return true;
  }
  return false;
}

#define score_zero_angle_is_not_possible 500
#define large_speed_penalty 500
#define fuel_consumption_multiplier 10
#define did_not_reach_solid_ground_multiplier 5
#define lz_buffer 50

#define GENERATE_PATH

void run_chromosome(simulation_data& sd, simulation_data& prev_sd, const chromosome& c) {
  sd = simdata;
  prev_sd = simdata;
  bool crashed = false;
  int PX=(int)std::round(sd.p[0]); // previous X
  int PY=(int)std::round(sd.p[1]); // previous Y
  int i = 0;
  int angle = sd.R;
  int thrust = sd.P;
  for (; i < chromosome_size; ++i) {
    angle += c[i].angle;
    thrust += c[i].thrust;
    angle = clamp_angle(angle);
    thrust = clamp_thrust(thrust);
    simulate(sd, angle, thrust);
    int X = (int)std::round(sd.p[0]);
    int Y = (int)std::round(sd.p[1]);
    int HS = (int)std::round(sd.v[0]);
    int VS = (int)std::round(sd.v[1]);
    crashed = crashed_or_landed(X, Y, PX, PY);
    if (crashed) {
      break;
    }
    PX=X;
    PY=Y;
    prev_sd = sd;
  }
}

#define EVALUATION_A

#if defined(EVALUATION_A)

int64_t evaluate(std::vector<vec2<float>>& path, chromosome& c) {
  int64_t score = 0;
#if defined(GENERATE_PATH)
  path.clear();
  path.reserve(chromosome_size);
#endif
  simulation_data sd = simdata;
  simulation_data sd_prev = sd;
  simulation_data sd_prev2 = sd_prev;
  bool crashed = false;
  int PX=(int)std::round(sd.p[0]); // previous X
  int PY=(int)std::round(sd.p[1]); // previous Y
  int i = 0;
  int angle = sd.R;
  int thrust = sd.P;
  for (; i < chromosome_size; ++i) {
    angle += c[i].angle;
    thrust += c[i].thrust;
    angle = clamp_angle(angle);
    thrust = clamp_thrust(thrust);
    simulate(sd, angle, thrust);
    int X = (int)std::round(sd.p[0]);
    int Y = (int)std::round(sd.p[1]);
    int HS = (int)std::round(sd.v[0]);
    int VS = (int)std::round(sd.v[1]);
#if defined(GENERATE_PATH)
    path.emplace_back(X,Y);
#endif
    crashed = crashed_or_landed(X, Y, PX, PY);
    if (crashed) {
#if defined(GENERATE_PATH)
      while (path.size()<chromosome_size)
        path.emplace_back(X,Y);
#endif
      break;
    }
    sd_prev2 = sd_prev;
    sd_prev = sd;
    PX=X;
    PY=Y;
  }
  
  const int landing_error_penalty = 3000;
  
  if (std::abs(sd_prev2.R)>maximum_angle_rotation) {
    score -= landing_error_penalty*(std::abs(sd_prev2.R)-maximum_angle_rotation);
  } else if (i>0) {
    c[i-1].angle = -sd_prev2.R;
    c[i].angle = 0;
    sd = sd_prev2;
    int t = clamp_thrust(sd_prev2.P + c[i-1].thrust);
    simulate(sd, 0, t);
    simulate(sd, 0, clamp_thrust(t+c[i].thrust));
  } else if (i==0) {
    c[i].angle = 0;
  }
  
  if (std::abs(sd_prev.v[1])>maximum_vertical_speed)
    score -= landing_error_penalty*(std::abs(sd_prev.v[1])-maximum_vertical_speed);
  
  if (std::abs(sd_prev.v[0])>maximum_horizontal_speed)
    score -= landing_error_penalty*(std::abs(sd_prev.v[0])-maximum_horizontal_speed);
  
  if (std::abs(sd.v[1])>maximum_vertical_speed)
    score -= landing_error_penalty*(std::abs(sd.v[1])-maximum_vertical_speed);
  
  if (std::abs(sd.v[0])>maximum_horizontal_speed)
    score -= landing_error_penalty*(std::abs(sd.v[0])-maximum_horizontal_speed);
  
  int X = (int)std::round(sd.p[0]);
  int Y = (int)std::round(sd.p[1]);
  /*
  int x0 = landing_zone_x0 + lz_buffer;
  int x1 = landing_zone_x1 - lz_buffer;
  float dsqr = 0;
  if (X < x0) {
    dsqr = distance_sqr(sd.p, vec2<float>(x0, landing_zone_y));
  } else if (X > x1) {
    dsqr = distance_sqr(sd.p, vec2<float>(x1, landing_zone_y));
  }
  */
  
  float dsqr = distance_sqr(sd.p, vec2<float>((landing_zone_x0+landing_zone_x1)*0.5f, landing_zone_y));
  
  if (dsqr < 100*100)
    score += sd.F;
  
  if ((int64_t)dsqr < W*W)
    score += W*W-(int64_t)dsqr;
    
  if (X>0 && X<W-1 && Y>0 && Y < H-1 && (X>landing_zone_x1 || X<landing_zone_x0))
    score += sqr(Y);
  else
    score += H*H;
  
  return score < 0 ? 0 : score;
}

#elif defined(EVALUATION_B)

int64_t evaluate(std::vector<vec2<float>>& path, chromosome& c) {
  int64_t score = 0;
#if defined(GENERATE_PATH)
  path.clear();
  path.reserve(chromosome_size);
#endif
  simulation_data sd = simdata;
  simulation_data sd_prev = sd;
  simulation_data sd_prev2 = sd_prev;
  bool crashed = false;
  int PX=(int)std::round(sd.p[0]); // previous X
  int PY=(int)std::round(sd.p[1]); // previous Y
  int i = 0;
  int angle = sd.R;
  int thrust = sd.P;
  for (; i < chromosome_size; ++i) {
    angle += c[i].angle;
    thrust += c[i].thrust;
    angle = clamp_angle(angle);
    thrust = clamp_thrust(thrust);
    simulate(sd, angle, thrust);
    int X = (int)std::round(sd.p[0]);
    int Y = (int)std::round(sd.p[1]);
    int HS = (int)std::round(sd.v[0]);
    int VS = (int)std::round(sd.v[1]);
#if defined(GENERATE_PATH)
    path.emplace_back(X,Y);
#endif
    crashed = crashed_or_landed(X, Y, PX, PY);
    if (crashed) {
#if defined(GENERATE_PATH)
      while (path.size()<chromosome_size)
        path.emplace_back(X,Y);
#endif
      break;
    }
    sd_prev2 = sd_prev;
    sd_prev = sd;
    PX=X;
    PY=Y;
  }
  
  
  const int landing_error_penalty = 3000;
  
  if (std::abs(sd_prev2.R)>maximum_angle_rotation) {
    score -= landing_error_penalty*(std::abs(sd_prev2.R)-maximum_angle_rotation);
  } else if (i>0) {
    c[i-1].angle = -sd_prev2.R;
    c[i].angle = 0;
    sd = sd_prev2;
    int t = clamp_thrust(sd_prev2.P + c[i-1].thrust);
    simulate(sd, 0, t);
    simulate(sd, 0, clamp_thrust(t+c[i].thrust));
  } else if (i==0) {
    c[i].angle = 0;
  }
  
  score += (simdata.F-sd.F)*100;
  
  if (std::abs(sd_prev.v[1])>maximum_vertical_speed)
    score += landing_error_penalty*(std::abs(sd_prev.v[1])-maximum_vertical_speed);
  
  if (std::abs(sd_prev.v[0])>maximum_horizontal_speed)
    score += landing_error_penalty*(std::abs(sd_prev.v[0])-maximum_horizontal_speed);
  
  if (std::abs(sd.v[1])>maximum_vertical_speed)
    score += landing_error_penalty*(std::abs(sd.v[1])-maximum_vertical_speed);
  
  if (std::abs(sd.v[0])>maximum_horizontal_speed)
    score += landing_error_penalty*(std::abs(sd.v[0])-maximum_horizontal_speed);
  
  int X = (int)std::round(sd.p[0]);
  int Y = (int)std::round(sd.p[1]);
  
  //if (Y > heights[X])
  //  score += (Y-heights[X])*did_not_reach_solid_ground_multiplier;
  
  vec2<float> lz((landing_zone_x0+landing_zone_x1)/2, landing_zone_y);
  
  int64_t dsqr = (int64_t)distance_sqr(sd.p, lz);
  /*
   int x0 = landing_zone_x0 + lz_buffer;
   int x1 = landing_zone_x1 - lz_buffer;
   float dsqr = 0;
   if (X < x0) {
   dsqr = distance_sqr(sd.p, vec2<float>(x0, heights[x0]));
   } else if (X > x1) {
   dsqr = distance_sqr(sd.p, vec2<float>(x1, heights[x1]));
   }
   */
  score += (int64_t)dsqr;
  return score;
}

#endif

bool is_a_valid_landing(const simulation_data& sd, const simulation_data& prev_sd) {
  if (sd.R != 0)
    return false;
  if (sd.p[0] < landing_zone_x0)
    return false;
  if (sd.p[0] > landing_zone_x1)
    return false;
  if (abs(sd.v[0])>maximum_horizontal_speed)
    return false;
  if (abs(sd.v[1])>maximum_vertical_speed)
    return false;
  if (prev_sd.p[1] <= landing_zone_y)
    return false;
  if (sd.p[1]>landing_zone_y)
    return false;
  return true;
}

void normalize_scores_roulette_wheel(std::vector<double>& out, const std::vector<int64_t>& score) {
#if defined(EVALUATION_B)
  int64_t M = *std::max_element(score.begin(), score.end());
#endif
  int64_t sum = 0;
  for (auto s : score)
#if defined(EVALUATION_A)
    sum += s;
#elif defined(EVALUATION_B)
  sum += (int64_t)(M-s);
#endif
  static std::vector<std::pair<double, int>> temp(score.size());
  for (int i = 0; i < score.size(); ++i) {
#if defined(EVALUATION_A)
    double new_score = (double)(score[i])/(double)sum;
#elif defined(EVALUATION_B)
    double new_score = (double)(M-score[i])/(double)sum;
#endif
    temp[i] = std::pair<double, int>(new_score, i);
  }
  std::sort(temp.begin(), temp.end(), [](const auto& left, const auto& right) { return left.first > right.first;});
  if (out.size() != score.size())
    out.resize(score.size());
  
  double accumulated_score=0.0;
  
  for (int i = temp.size()-1; i>=0; --i) {
    accumulated_score += temp[i].first;
    out[temp[i].second] = accumulated_score;
  }
  
}

double elitarism_factor = 0.1;
double mutation_chance = 0.01;

inline double rand_double() {
  return (double)(rkiss.rand64()%100000)/100000.0;
}

void make_children(chromosome& child1, chromosome& child2, const chromosome& parent1, const chromosome& parent2) {
  if (child1.size() != chromosome_size)
    child1.resize(chromosome_size);
  if (child2.size() != chromosome_size)
    child2.resize(chromosome_size);
  double r = rand_double();
  double r2 = 1.0-r;
  for (int i = 0; i < chromosome_size; ++i) {
    gene g1, g2;
    double r3 = rand_double();
    if (r3 < mutation_chance)
      g1 = generate_random_gene();
    else {
      g1.angle = (int)std::round(parent1[i].angle*r+parent2[i].angle*r2);
      g1.thrust = (int)std::round(parent1[i].thrust*r+parent2[i].thrust*r2);
    }
    double r4 = rand_double();
    if (r4 < mutation_chance)
      g2 = generate_random_gene();
    else {
      g2.angle = std::round(parent1[i].angle*r2+parent2[i].angle*r);
      g2.thrust = std::round(parent1[i].thrust*r2+parent2[i].thrust*r);
    }
    
    child1[i] = g1;
    child2[i] = g2;
    
  }
}

void make_next_generation(population& new_pop, const population& current, const std::vector<double>& score) {
  if (new_pop.size() != current.size())
    new_pop.resize(current.size());
  static std::vector<std::pair<double, int>> score_index;
  if (score_index.size() != score.size())
    score_index.resize(score.size());
  for (int i = 0; i < score.size(); ++i)
  score_index[i] = std::pair<double, int>(score[i], i);
  std::sort(score_index.begin(), score_index.end(), [](const auto& left, const auto& right) { return left.first > right.first;});
  int elitair_chromosomes_to_copy = (int)(score.size()*elitarism_factor);
  if ((score.size()-elitair_chromosomes_to_copy)%2)
    ++elitair_chromosomes_to_copy;
  
  for (int i=0; i < elitair_chromosomes_to_copy; ++i) {
    new_pop[i] = current[score_index[i].second];
  }
  
  const int children = (current.size() - elitair_chromosomes_to_copy);
  
  for (int i = 0; i < children/2; ++i) {
    double r = rand_double();
    int idx = 0;
    while (score_index[idx].first>r && idx < score_index.size())
      ++idx;
    --idx;
    int first_parent_index = score_index[idx].second;
    int second_parent_index = first_parent_index;
    while (second_parent_index == first_parent_index) {
      r = rand_double();
      idx = 0;
      while (score_index[idx].first>r && idx < score_index.size())
        ++idx;
      --idx;
      second_parent_index = score_index[idx].second;
    }
    make_children(new_pop[2*i+elitair_chromosomes_to_copy], new_pop[2*i+1+elitair_chromosomes_to_copy], current[first_parent_index], current[second_parent_index]);
  }
  
}
