#define FWK_IMPLEMENTATION      // unrolls single-header implementation
#include "engine/joint/fwk.h"   // single-header file
#include "fwk_network.h"



int main() {
    struct player_t {
        uint64_t seen_until;
        int x,y,z;
        uint32_t color;
    };
    struct world_t {
        struct player_t player[MAX_CLIENTS];
    } world = {0};

    // network setup
    network_create("127.0.0.1", 0, flag("--client") ? NETWORK_CONNECT : 0);
    int64_t self_id = network_get(NETWORK_RANK);

    uint32_t colors[] = { ORANGE,GREEN,RED,CYAN,PURPLE,YELLOW,GRAY,PINK,AQUA };
    for (int64_t i=0; i<MAX_CLIENTS; ++i) {
        world.player[i].color = colors[i%(sizeof colors / sizeof colors[0])];
        network_buffer(&world.player[i], sizeof(struct player_t), i!=self_id ? NETWORK_RECV : NETWORK_SEND, i /* each client owns exactly 1 buffer */);
    }

    // game setup
    camera_t cam = camera();
    window_create( 0.35f, WINDOW_MSAA8|WINDOW_SQUARE );
    struct player_t *self = &world.player[self_id];

    // game loop
    while( window_swap() && !input(KEY_ESC) ) {
        // network sync
        char **event = network_sync(0); // timeout_ms:0
        while(*event) printf( "network event: %s\n", *event++ );

        struct player_t *other = self;
        float ldist = 0.f;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            struct player_t *p = &world.player[i];
            if (p == self) continue; /* skip self */
            if (p->seen_until < date_epoch()) continue; /* skip inactive players */
            float dist = len3(sub3(vec3(p->x,p->y,p->z), vec3(self->x,self->y,self->z)));
            if (dist > ldist) {
                ldist = dist;
                other = p;
            }
        }
        
        // camera tracking
        vec3 target = mix3(vec3(self->x,0,self->z), vec3(other->x,0,other->z), 0.40f); // weight:40%
        // vec3 target = vec3(self->x,0,self->z);
        vec3 dir = norm3(sub3(target, cam.position));
        camera_lookat(&cam, target);
        float distance = len3(sub3(vec3(self->x,0,self->z), vec3(other->x,0,other->z)));
        target = add3(target, scale3(dir, -(distance < 10 ? 10 : distance))); // min_distance:10
        cam.position = mix3(cam.position, target, 0.01f); // smoothing:0.01f
        // cam.position = mix3(add3(cam.position, vec3(0,0.5f,0)), target, 0.01f); // smoothing:0.01f

        // input - move player
        self->x += input(KEY_RIGHT) - input(KEY_LEFT);
        self->z += input(KEY_DOWN) - input(KEY_UP);
        self->seen_until = date_epoch() + 4;

        // background - draw grid
        ddraw_grid(0);

        // foreground - draw all players
        for( int id = 0; id < MAX_CLIENTS; ++id ) {
            struct player_t *p = &world.player[id];
            if (p->seen_until < date_epoch()) continue; /* skip inactive players */
            ddraw_color( p->color );
            ddraw_capsule(vec3(p->x,0,p->z), vec3(p->x,2,p->z), 1);
            ddraw_text(vec3(p->x,4,p->z), 0.01, stringf("player #%d", id));
        }

        // stats
        char title[64];
        sprintf(title, "player #%lld", self_id);
        window_title(title);
    }
}
