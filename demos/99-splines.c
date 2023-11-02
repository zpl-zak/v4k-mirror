// @readme: https://gamedev.stackexchange.com/questions/14985/determine-arc-length-of-a-catmull-rom-spline-to-move-at-a-constant-speed/14995#14995

#include "v4k.h"

// [ref] https://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline

vec3 catmull( vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t ) {
    float t2 = t*t;
    float t3 = t*t*t;

    vec3 c;
    c.x = 0.5 * ((2 * p1.x) + (-p0.x + p2.x) * t + (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3);
    c.y = 0.5 * ((2 * p1.y) + (-p0.y + p2.y) * t + (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3);
    c.z = 0.5 * ((2 * p1.z) + (-p0.z + p2.z) * t + (2 * p0.z - 5 * p1.z + 4 * p2.z - p3.z) * t2 + (-p0.z + 3 * p1.z - 3 * p2.z + p3.z) * t3);
    return c;
}

// [ref] https://gamedevnotesblog.wordpress.com/2017/09/08/constant-time-algorithm-for-parametric-curves/
// - rlyeh, public domain. based on pseudo-code by Matrefeytontias

typedef struct curve {
    array(float) lengths;
    array(unsigned) colors;
    array(vec3)  samples;
    array(vec3)  points;
    array(int)   indices;
} curve;

    void curve_add(curve *c, vec3 pt) {
        // @fixme: do not add dupes into list
        array_push(c->points, pt);
    }

    void curve_finish(curve *c, int k ) {
        assert( k > 0 );

        c->lengths = 0;
        c->samples = 0;
        c->indices = 0;
        c->colors = 0;

        // refit points[N] to samples[K]
        int N = array_count(c->points);
        for( int i = 0; i < N; ++i) {
            printf("P%d) ", i); print3(c->points[i]); puts("");
        }
        if( k < N ) {

            // truncate: expected k-points lesser or equal than existing N points
            for( int i = 0; i <= k; ++i ) {
                float s = (float)i / k;
                int t = s * (N-1);
                array_push(c->samples, c->points[t]);

                float p = fmod(i, N-1) / (N-1); // [0..1)
                int is_control_point = p <= 0 || p >= 1;
                array_push(c->colors, is_control_point ? ORANGE: BLUE);
            }

        } else {
            // interpolate: expected k-points greater than existing N-points
//
--N;
printf("k%d, N%d\n", k, N);
int upper = N - (k%N);
int lower = (k%N);
if(upper < lower)
k += upper;
else
k -= lower;
printf("k%d, N%d\n", k, N);
//k -= 2; // begin/end points do not compute below

//array_push(c->samples, c->points[0]);
int points_per_segment = (k / N);
++N;

int looped = len3sq(sub3(c->points[0], *array_back(c->points))) < 0.1;

            for( int i = 0; i <= k; ++i ) {
#if 1
                int point = i % points_per_segment;
                float p = point / (float)points_per_segment; // [0..1)
#else
                float p = fmod(i, N) / N; // [0..1)
#endif
                int t = i / points_per_segment;

                // linear
                vec3 l = mix3(c->points[t], c->points[t+(i!=k)], p);

                // printf("%d) %d>%d %f\n", i, t, t+(i!=k), p);
                ASSERT(p <= 1);

#if 1
                // catmull
                int p0 = t - 1;
                int p1 = t + 0;
                int p2 = t + 1;
                int p3 = t + 2;
                if( looped )
                {
                    int M = N-1;
                    if(p0<0) p0+=M; else if(p0>=M) p0-=M;
                    if(p1<0) p1+=M; else if(p1>=M) p1-=M;
                    if(p2<0) p2+=M; else if(p2>=M) p2-=M;
                    if(p3<0) p3+=M; else if(p3>=M) p3-=M;
                }
                else
                {
                    int M = N-1;
                    if(p0<0) p0=0; else if(p0>=M) p0=M;
                    if(p1<0) p1=0; else if(p1>=M) p1=M;
                    if(p2<0) p2=0; else if(p2>=M) p2=M;
                    if(p3<0) p3=0; else if(p3>=M) p3=M;
                }
                vec3 m = catmull(c->points[p0],c->points[p1],c->points[p2],c->points[p3],p);
                l = m;
#endif

                array_push(c->samples, l);

                int is_control_point = p <= 0 || p >= 1;
                array_push(c->colors, is_control_point ? ORANGE: BLUE);
            }
//array_push(c->samples, *array_back(c->points));
        }

        array_push(c->lengths, 0 );
        for( int i = 1; i <= k; ++i ) {
            // approximate curve length at every sample point
            array_push(c->lengths, len3(sub3(c->samples[i], c->samples[i-1])) + c->lengths[i-1] );
        }
        // normalize lengths to be between 0 and 1
        float maxv = c->lengths[k];
        for( int i = 1; i <= k; ++i ) c->lengths[i] /= maxv;

#if 0
        for( int i = 0; i <= k; ++i) {
            printf("L%d)%f,", i, c->lengths[i]);
        } puts("");
#endif

        array_push(c->indices, 0 );
        for( int i = 0/*1*/; i </*=*/ k; ++i ) {
            float s = (float)i / (k-1); //k;
            int j; // j = so that lengths[j] <= s < lengths[j+1];
#if 0
            for( j = 0; j <= k; ++j ) {
                if( c->lengths[j] <= s && s < c->lengths[j+1] ) {
                    break;
                }
            }
#else
            // j = Index of the highest length that is less or equal to s
            // Can be optimized with a binary search instead
            for( j = *array_back(c->indices) + 1; j </*=*/ k; ++j ) {
                if( c->lengths[j] </*=*/ s ) continue;
                break;
            }
#endif
            if (c->lengths[j] > 0.01)
            array_push(c->indices, j );
        }

        for( int i = 0; i < array_count(c->indices); ++i ) {
            printf("I%d,", c->indices[i]);
        } puts("");
    }

    vec3 curve_eval(curve *c, float dt, unsigned *color) {
        unsigned nil; if(!color) color = &nil;
        if(dt <= 0) dt = 0; // return *color = c->colors[0], c->samples[0];
        if(dt >= 1) dt = 1; // return *color = *array_back(c->colors), *array_back(c->samples);
        int l = (int)(array_count(c->indices) - 1);
        int p = (int)(dt * l);
#if 1
        int t = c->indices[p];
#else
        int t = p;
#endif
        t %= (array_count(c->indices)-1);
        vec3 pos = mix3(c->samples[t], c->samples[t+1], dt * l - p);
        *color = c->colors[t];

        puts/*window_title*/(stringf("dt=%5.2f P%dI%d %5f %5f %5f", dt, p,t, pos.x,pos.y,pos.z));

        return pos;
    }


int main() {
    window_create(0.85, WINDOW_MSAA4);

    camera_t cam = camera();

    curve cv = {0};
    for(float t = 0; t <= 360; t += 36) {
        curve_add(&cv, vec3(cos(t*TO_RAD)*5,0,sin(t*TO_RAD)*5));
//        curve_add(&cv, vec3(cos(t*TO_RAD)*5,0,sin(t*TO_RAD)*5));
//        curve_add(&cv, vec3(cos(t*TO_RAD)*5,0,sin(t*TO_RAD)*5));
//        curve_add(&cv, vec3(cos(t*TO_RAD)*5,0,sin(t*TO_RAD)*5));
    }
    int num_points = 11; // beware with these: 8,11,17,20,61,100,200
    curve_finish(&cv, num_points);

    while(window_swap() && !input_down(KEY_ESC)) {
        // fps camera
        bool active = ui_hover() || ui_active() || gizmo_active() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdecq = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-(input(KEY_C)||input(KEY_Q)),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_moveby(&cam, wasdecq);
        camera_fps(&cam, mouse.x,mouse.y);

        // 3d
        ddraw_grid(0);

        for( int i = 0, end = array_count(cv.samples) - 1; i < end; ++i) {
            int vis = (int)fmod(window_time(), array_count(cv.samples) - 1);
            ddraw_color(i == vis ? BLUE : YELLOW);
            ddraw_point(cv.samples[i]);
            ddraw_color(YELLOW);
            if(i==vis)
            ddraw_line(cv.samples[i], cv.samples[i+1]);
        }

        static int delay = 5;
        float dt = fmod(window_time(), delay) / delay;
        vec3 pos = curve_eval(&cv, dt, NULL);
        ddraw_color(PURPLE);
        ddraw_sphere(pos, 0.5);

        if( ui_panel("Path",PANEL_OPEN)) {
            if(ui_int("Points", &num_points)) { if(num_points < 6) num_points = 6; curve_finish(&cv, num_points); }
            if(ui_int("Delay", &delay)) { if(delay <= 0) delay = 1; }
            ui_panel_end();
        }
    }
}
