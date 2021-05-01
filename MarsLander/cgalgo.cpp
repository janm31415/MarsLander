//#pragma GCC optimize("O3","unroll-loops","omit-frame-pointer","inline") //Optimization flags
//#pragma GCC option("arch=native","tune=native","no-zero-upper") //Enable AVX
//#pragma GCC target("avx2")  //Enable AVX

#include "cgalgo.h"

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
    if (angle>R+15)
        angle = R+15;
    else if (angle < R-15)
        angle = R-15;
#if defined(OUTOFBOUNDSCLAMPING)
    if (angle>90)
        angle = 90;
    if (angle<-90)
        angle = -90;
#endif
    return angle;
}

int clamp_thrust(int thrust, int P) {
    if (thrust > P+1)
        thrust = P+1;
    if (thrust < P-1)
        thrust = P-1;
#if defined(OUTOFBOUNDSCLAMPING)
    if (thrust < 0)
        thrust = 0;
    if (thrust > 4)
        thrust = 4;
#endif
    return thrust;
}

std::vector<vec2<int>> surface_points;

void simulate(sdata& sd, int angle, int thrust) {
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
}

void do_main(std::stringstream& strcin, std::stringstream& strerr)
{
    read_input(strcin, strerr);
    
    auto heights = compute_terrain_heights(surface_points);
    int land_x0, land_x1;
    find_landingzone(land_x0, land_x1, heights);
    strerr << "Landing zone: (" << land_x0 << ", " << land_x1 << ")" << std::endl;

    float g = 3.711f;
    float max_speed = 4.f;

    int max_angle = (int)std::floor(acos(g/max_speed)*180.f/pi);
    strerr << "Max angle: " << max_angle << std::endl;

    float max_vs = 40.f;
    float max_hs = 20.f;


    int angle=-20;
    int thrust=0;
    bool move_right = false;


    bool first_time = true;


    sdata sda;
    // game loop
    while (1) {

        angle=rand()%180-90;
        thrust=rand()%5;

        int X;
        int Y;
        int HS; // the horizontal speed (in m/s), can be negative.
        int VS; // the vertical speed (in m/s), can be negative.
        int F; // the quantity of remaining fuel in liters.
        int R; // the rotation angle in degrees (-90 to 90).
        int P; // the thrust power (0 to 4).
        strcin >> X >> Y >> HS >> VS >> F >> R >> P; strcin.ignore();

        if (!first_time) {

            int PX = (int)std::round(sda.p.x);
            int PY = (int)std::round(sda.p.y);
            int VX = (int)std::round(sda.v.x);
            int VY = (int)std::round(sda.v.y);
            if (PX != X || PY != Y || VX != HS || VY != VS || sda.F != F || sda.R != R || sda.P != P) {
                strerr << "X: " << sda.p.x << " vs " << X << std::endl;
                strerr << "Y: " << sda.p.y << " vs " << Y << std::endl;
                strerr << "HS: " << sda.v.x << " vs " << HS << std::endl;
                strerr << "VS: " << sda.v.y << " vs " << VS << std::endl;
                strerr << "F: " << sda.F << " vs " << F << std::endl;
                strerr << "R: " << sda.R << " vs " << R << std::endl;
                strerr << "P: " << sda.P << " vs " << P << std::endl;
            }
        } else {
            strerr << X << " " << Y << " " << HS << " " << VS << " " << F << " " << R << " " << P << std::endl;
            sda.F = F;
            sda.P = P;
            sda.R = R;
            sda.p.x = X;
            sda.p.y = Y;
            sda.v.x = HS;
            sda.v.y = VS;
        }

        simulate(sda, angle, thrust);


        std::cout << angle << " " << thrust << std::endl;
        first_time = false;
    }
}
