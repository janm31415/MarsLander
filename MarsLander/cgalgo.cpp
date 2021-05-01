//#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline") //Optimization flags
//#pragma GCC option("arch=native","tune=native","no-zero-upper") //Enable AVX
//#pragma GCC target("avx2")  //Enable AVX

/*
 Based on the following blog post
 https://www.codingame.com/blog/genetic-algorithm-mars-lander/
 */

#include "cgalgo.h"

std::vector<vec2<int>> surface_points;
std::vector<int> heights;
RKISS rkiss;
int landing_zone_x0, landing_zone_x1;
simulation_data simdata;

std::vector<int> compute_terrain_heights(const std::vector<vec2<int>>& pts) {
  std::vector<int> heights;
  heights.reserve(W);
  auto it = pts.begin();
  auto it_next = it;
  ++it_next;
  for (int x = 0; x < W; ++x) {
    if (x > it_next->x) {
      ++it;
      ++it_next;
    }
    
    float alpha = (float)(x - it->x)/(float)(it_next->x - it->x);
    float y_value = (1.f-alpha)*it->y + alpha*it_next->y;
    heights.push_back((int)std::ceil(y_value));
  }
  
  return heights;
}

void find_landingzone(int& x0, int& x1, const std::vector<int>& heights) {
  x0 = x1 = 0;
  int startcandidate = 0;
  for (int i = 1; i < heights.size(); ++i) {
    if (heights[i] != heights[startcandidate]) {
      int new_length = i-1-startcandidate;
      int old_length = x1-x0;
      if (new_length > old_length) {
        x0 = startcandidate;
        x1 = i-1;
      }
      startcandidate = i;
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

chromosome generate_random_chromosome(int init_R, int init_P) {
  chromosome c;
  c.reserve(chromosome_size);
  int last_R = init_R;
  int last_P = init_P;
  for (int i = 0; i < chromosome_size; ++i) {
    int R = last_R + (int)(rkiss.rand64()%(2*maximum_angle_rotation+1))-maximum_angle_rotation;
    R = R<-maximum_angle?-maximum_angle:R>maximum_angle?maximum_angle:R;
    int P = last_P + (int)(rkiss.rand64()%(2*maximum_thrust_change+1))-maximum_thrust_change;
    P = P<0?0:P>maximum_thrust?maximum_thrust:P;
    gene g;
    g.angle = R;
    g.thrust = P;
    last_R = R;
    last_P = P;
    c.push_back(g);
  }
  return c;
}

population generate_random_population(int init_R, int init_P) {
  population p;
  p.reserve(population_size);
  for (int i = 0; i < population_size; ++i) {
    p.emplace_back(generate_random_chromosome(init_R, init_P));
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
  heights = compute_terrain_heights(surface_points);
  find_landingzone(landing_zone_x0, landing_zone_x1, heights);
  
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

#define score_zero_angle_is_not_possible 500
#define large_speed_penalty 100
#define fuel_consumption_multiplier 10
#define did_not_reach_solid_ground_multiplier 5
#define lz_buffer 5

int evaluate(std::vector<vec2<float>>& path, const chromosome& c) {
  int score = 0;
  path.clear();
  path.reserve(chromosome_size);
  simulation_data sd = simdata;
  simulation_data sd_prev = sd;
  simulation_data sd_prev2 = sd_prev;
  for (int i = 0; i < chromosome_size; ++i) {
    simulate(sd, c[i].angle, c[i].thrust);
    int X = (int)std::round(sd.p[0]);
    int Y = (int)std::round(sd.p[1]);
    int HS = (int)std::round(sd.v[0]);
    int VS = (int)std::round(sd.v[1]);
    path.emplace_back(X,Y);
    if (Y<=heights[X]) {
      while (path.size()<chromosome_size)
        path.emplace_back(X,Y);
      break;
    }
    sd_prev2 = sd_prev;
    sd_prev = sd;
  }
  int X = (int)std::round(sd.p[0]);
  int Y = (int)std::round(sd.p[1]);
  if (Y > heights[X])
    score += (Y-heights[X])*did_not_reach_solid_ground_multiplier;
  
  vec2<float> lz;
  //vec2<float> lz((landing_zone_x0+landing_zone_x1)*0.5, heights[landing_zone_x0]);
  
  if (X < landing_zone_x0+lz_buffer) {
    lz = vec2<float>(landing_zone_x0+lz_buffer, heights[landing_zone_x0]);
  } else if (X > landing_zone_x1-lz_buffer) {
    lz = vec2<float>(landing_zone_x1-lz_buffer, heights[landing_zone_x0]);
  } else {
    lz = vec2<float>(X, heights[X]);
  }
  
  score += (int)distance(sd.p, lz)*500;
  
  if (std::abs(sd_prev2.R)>maximum_angle_rotation)
    score += score_zero_angle_is_not_possible;
  if (std::abs(sd_prev.v[0])>maximum_horizontal_speed)
    score += (std::abs(sd_prev.v[0])-maximum_horizontal_speed)*large_speed_penalty;
  if (std::abs(sd_prev.v[1])>maximum_vertical_speed)
    score += (std::abs(sd_prev.v[1])-maximum_vertical_speed)*large_speed_penalty;
  score += (simdata.F - sd.F)*fuel_consumption_multiplier;
  return score;
}

std::vector<double> normalize_scores_roulette_wheel(const std::vector<int>& score) {
  int M = *std::max_element(score.begin(), score.end());
  int sum = 0;
  for (auto s : score)
    sum += (M-s);
  std::vector<std::pair<double, int>> temp;
  temp.reserve(score.size());
  for (int i = 0; i < score.size(); ++i) {
    double new_score = (double)(M-score[i])/(double)sum;
    temp.emplace_back(new_score, i);
  }
  std::sort(temp.begin(), temp.end(), [](const auto& left, const auto& right) { return left.first > right.first;});
  std::vector<double> out(score.size(), 0.0);
  
  for (int i = 0; i < temp.size(); ++i) {
    for (int j = i; j < temp.size(); ++j) {
      out[temp[i].second] += temp[j].first;
    }
  }
  
  return out;
}

#define elitarism_factor 0.1
#define mutation_chance 0.01

int clamp_angle(int R) {
  return R<-maximum_angle?-maximum_angle:R>maximum_angle?maximum_angle:R;
}

int clamp_thrust(int P) {
  return P<0?0:P>maximum_thrust?maximum_thrust:P;
}

void make_children(chromosome& child1, chromosome& child2, const chromosome& parent1, const chromosome& parent2) {
  child1.reserve(chromosome_size);
  child2.reserve(chromosome_size);
  child1.clear();
  child2.clear();
  double r = (double)(rkiss.rand64()%100000)/100000.0;
  double r2 = 1.0-r;
  for (int i = 0; i < chromosome_size; ++i) {
    gene g1;
    g1.angle = parent1[i].angle*r+parent2[i].angle*r2;
    g1.thrust = parent1[i].thrust*r+parent2[i].thrust*r2;
    gene g2;
    g2.angle = parent1[i].angle*r2+parent2[i].angle*r;
    g2.thrust = parent1[i].thrust*r2+parent2[i].thrust*r;
    
    double r3 = (double)(rkiss.rand64()%100000)/100000.0;
    if (r3 < mutation_chance) {
      g1.angle = rkiss.rand64()%(2*maximum_angle+1)-maximum_angle;
      g1.thrust = rkiss.rand64()%(maximum_thrust+1);
    }
    double r4 = (double)(rkiss.rand64()%100000)/100000.0;
    if (r4 < mutation_chance) {
      g2.angle = rkiss.rand64()%(2*maximum_angle+1)-maximum_angle;
      g2.thrust = rkiss.rand64()%(maximum_thrust+1);
    }
    
    g1.angle = clamp_angle(g1.angle);
    g1.thrust = clamp_thrust(g1.thrust);
    g2.angle = clamp_angle(g2.angle);
    g2.thrust = clamp_thrust(g2.thrust);
    
    if (i > 0) {
      const auto& pg1 = child1[i-1];
      const auto& pg2 = child2[i-1];
      if (g1.angle>pg1.angle+maximum_angle_rotation)
        g1.angle = pg1.angle+maximum_angle_rotation;
      if (g1.angle < pg1.angle-maximum_angle_rotation)
        g1.angle = pg1.angle-maximum_angle_rotation;
      if (g1.thrust>pg1.thrust+maximum_thrust_change)
        g1.thrust = pg1.thrust+maximum_thrust_change;
      if (g1.thrust < pg1.thrust-maximum_thrust_change)
        g1.thrust = pg1.thrust-maximum_thrust_change;
        
      if (g2.angle>pg2.angle+maximum_angle_rotation)
        g2.angle = pg2.angle+maximum_angle_rotation;
      if (g2.angle < pg2.angle-maximum_angle_rotation)
        g2.angle = pg2.angle-maximum_angle_rotation;
      if (g2.thrust>pg2.thrust+maximum_thrust_change)
        g2.thrust = pg2.thrust+maximum_thrust_change;
      if (g2.thrust < pg2.thrust-maximum_thrust_change)
        g2.thrust = pg2.thrust-maximum_thrust_change;
              
    }
  
    child1.push_back(g1);
    child2.push_back(g2);
    
  }
}

population make_next_generation(const population& current, const std::vector<double>& score) {
  population new_pop;
  new_pop.reserve(current.size());
  std::vector<std::pair<double, int>> score_index;
  score_index.reserve(score.size());
  for (int i = 0; i < score.size(); ++i)
  score_index.emplace_back(score[i], i);
  std::sort(score_index.begin(), score_index.end(), [](const auto& left, const auto& right) { return left.first > right.first;});
  int elitair_chromosomes_to_copy = (int)(score.size()*elitarism_factor);
  if ((score.size()-elitair_chromosomes_to_copy)%2)
    ++elitair_chromosomes_to_copy;
  
  for (int i=0; i < elitair_chromosomes_to_copy; ++i) {
    new_pop.push_back(current[score_index[i].second]);
  }
  
  const int children = (current.size() - elitair_chromosomes_to_copy);
  
  for (int i = 0; i < children/2; ++i) {
    double r = (double)(rkiss.rand64()%100000)/100000.0;
    int idx = 0;
    while (score_index[idx].first>r && idx < score_index.size())
      ++idx;
    --idx;
    int first_parent_index = score_index[idx].second;
    int second_parent_index = first_parent_index;
    while (second_parent_index == first_parent_index) {
      r = (double)(rkiss.rand64()%100000)/100000.0;
      idx = 0;
      while (score_index[idx].first>r && idx < score_index.size())
        ++idx;
      --idx;
      second_parent_index = score_index[idx].second;
    }
    chromosome c1, c2;
    make_children(c1, c2, current[first_parent_index], current[second_parent_index]);
    new_pop.push_back(c1);
    new_pop.push_back(c2);
  }
    
  return new_pop;
}
