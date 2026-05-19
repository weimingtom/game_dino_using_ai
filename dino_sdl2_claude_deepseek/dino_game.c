#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ------------------------------------------------------------------ */
/*  Constants                                                         */
/* ------------------------------------------------------------------ */
#define SCREEN_WIDTH      800
#define SCREEN_HEIGHT     400
#define GROUND_Y          320
#define FPS               60
#define GRAVITY           0.7f
#define JUMP_VELOCITY     -14.0f
#define INITIAL_SPEED     6.0f
#define MAX_SPEED         14.0f
#define SPEED_INCREMENT   0.002f
#define SPAWN_MIN         800
#define SPAWN_MAX         1800

#define DINO_W  44
#define DINO_H  48

#define OBS_W   20
#define NUM_OBS_HEIGHTS  3
#define OBS_HEIGHT_0 36
#define OBS_HEIGHT_1 48
#define OBS_HEIGHT_2 40

#define MAX_OBSTACLES  8
#define MAX_CLOUDS     6

/* colors */
static const SDL_Color CLR_BG       = {247, 247, 247, 255};
static const SDL_Color CLR_GROUND   = { 83,  83,  83, 255};
static const SDL_Color CLR_BODY     = { 50,  50,  50, 255};
static const SDL_Color CLR_EYE      = {247, 247, 247, 255};
static const SDL_Color CLR_PUPIL    = { 30,  30,  30, 255};
static const SDL_Color CLR_TEXT     = { 83,  83,  83, 255};
static const SDL_Color CLR_HI       = {160, 160, 160, 255};
static const SDL_Color CLR_CLOUD    = {200, 200, 200, 255};
static const SDL_Color CLR_HINT     = {150, 150, 150, 255};

/* ------------------------------------------------------------------ */
/*  Dinosaur                                                          */
/* ------------------------------------------------------------------ */
typedef struct {
    float x, y, vy;
    int   on_ground;
    int   leg_frame;
    int   leg_timer;
} Dinosaur;

static void dino_init(Dinosaur *d, int x, int ground_y) {
    d->x = (float)x;
    d->y = (float)(ground_y - DINO_H);
    d->vy = 0;
    d->on_ground = 1;
    d->leg_frame = 0;
    d->leg_timer = 0;
}

static void dino_jump(Dinosaur *d) {
    if (d->on_ground) {
        d->vy = JUMP_VELOCITY;
        d->on_ground = 0;
    }
}

static void dino_update(Dinosaur *d, int ground_y) {
    d->y += d->vy;
    d->vy += GRAVITY;
    if (d->y >= ground_y - DINO_H) {
        d->y = (float)(ground_y - DINO_H);
        d->vy = 0;
        d->on_ground = 1;
    }
    if (d->on_ground) {
        d->leg_timer++;
        if (d->leg_timer >= 6) {
            d->leg_timer = 0;
            d->leg_frame = !d->leg_frame;
        }
    }
}

static SDL_Rect dino_rect(const Dinosaur *d) {
    SDL_Rect r;
    r.x = (int)d->x;
    r.y = (int)d->y;
    r.w = DINO_W;
    r.h = DINO_H;
    return r;
}

static void set_color(SDL_Renderer *ren, SDL_Color c) {
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
}

static void fill_rounded(SDL_Renderer *ren, SDL_Rect r, int radius) {
    /* Simple rounded rect: fill the interior, then draw
       small filled circles at corners (approximation). */
    SDL_Rect inner = {r.x + radius, r.y, r.w - 2 * radius, r.h};
    SDL_RenderFillRect(ren, &inner);
    inner.x = r.x;
    inner.y = r.y + radius;
    inner.w = r.w;
    inner.h = r.h - 2 * radius;
    SDL_RenderFillRect(ren, &inner);

    /* corner circles (cheap approximation with small rects) */
    SDL_Rect cr;
    cr.w = cr.h = radius * 2;
    cr.x = r.x;  cr.y = r.y;  SDL_RenderFillRect(ren, &cr);
    cr.x = r.x + r.w - radius * 2;  SDL_RenderFillRect(ren, &cr);
    cr.y = r.y + r.h - radius * 2;  SDL_RenderFillRect(ren, &cr);
    cr.x = r.x;  SDL_RenderFillRect(ren, &cr);
}

static void dino_draw(const Dinosaur *d, SDL_Renderer *ren) {
    SDL_Rect r = dino_rect(d);
    set_color(ren, CLR_BODY);

    /* body */
    SDL_Rect body = {r.x + 2, r.y + 4, 34, 40};
    fill_rounded(ren, body, 6);

    /* head */
    SDL_Rect head = {r.x + 28, r.y + 2, 18, 18};
    fill_rounded(ren, head, 4);

    /* eye */
    set_color(ren, CLR_EYE);
    SDL_Rect eye = {r.x + 36, r.y + 5, 10, 10};
    SDL_RenderFillRect(ren, &eye);
    set_color(ren, CLR_PUPIL);
    eye = (SDL_Rect){r.x + 38, r.y + 7, 4, 4};
    SDL_RenderFillRect(ren, &eye);

    /* mouth */
    set_color(ren, CLR_BODY);
    SDL_RenderDrawLine(ren, r.x + 40, r.y + 16, r.x + 46, r.y + 13);

    /* tail */
    SDL_Point tail[4] = {
        {r.x + 2,  r.y + 20},
        {r.x - 8,  r.y + 14},
        {r.x - 6,  r.y + 24},
        {r.x + 2,  r.y + 28}
    };
    SDL_RenderDrawLines(ren, tail, 4);
    /* fill tail with additional lines */
    for (int i = r.y + 15; i <= r.y + 27; i++) {
        SDL_RenderDrawLine(ren, r.x + 2, i, r.x - 7, r.y + 19);
    }

    /* arm */
    SDL_Rect arm = {r.x + 8, r.y + 22, 4, 10};
    fill_rounded(ren, arm, 2);

    /* legs */
    if (d->leg_frame == 0) {
        SDL_Rect l1 = {r.x + 14, r.y + 38, 8, 10};
        fill_rounded(ren, l1, 2);
        SDL_Rect l2 = {r.x + 26, r.y + 40, 8, 8};
        fill_rounded(ren, l2, 2);
    } else {
        SDL_Rect l1 = {r.x + 14, r.y + 40, 8, 8};
        fill_rounded(ren, l1, 2);
        SDL_Rect l2 = {r.x + 26, r.y + 38, 8, 10};
        fill_rounded(ren, l2, 2);
    }
}

/* ------------------------------------------------------------------ */
/*  Obstacle (cactus)                                                 */
/* ------------------------------------------------------------------ */
static const int OBS_HEIGHTS[NUM_OBS_HEIGHTS] = {36, 48, 40};

typedef struct {
    float x;
    int   y, h;
    int   passed;
} Obstacle;

static void obs_init(Obstacle *o, float x, int ground_y, int size_idx) {
    o->x = x;
    o->h = OBS_HEIGHTS[size_idx];
    o->y = ground_y - o->h;
    o->passed = 0;
}

static void obs_update(Obstacle *o, float speed) {
    o->x -= speed;
}

static int obs_off_screen(const Obstacle *o) {
    return o->x + OBS_W < 0;
}

static SDL_Rect obs_rect(const Obstacle *o) {
    SDL_Rect r;
    r.x = (int)o->x;
    r.y = o->y;
    r.w = OBS_W;
    r.h = o->h;
    return r;
}

static void obs_draw(const Obstacle *o, SDL_Renderer *ren) {
    SDL_Rect r = obs_rect(o);
    set_color(ren, CLR_GROUND);

    /* trunk */
    SDL_Rect trunk = {r.x + 6, r.y, 8, r.h};
    fill_rounded(ren, trunk, 3);

    /* left arm */
    int ah = r.h * 35 / 100;
    if (ah < 6) ah = 6;
    SDL_Rect arm1 = {r.x, r.y + 6, 6, ah};
    fill_rounded(ren, arm1, 2);

    /* right arm */
    int ah2 = r.h * 25 / 100;
    if (ah2 < 4) ah2 = 4;
    SDL_Rect arm2 = {r.x + 14, r.y + 10, 6, ah2};
    fill_rounded(ren, arm2, 2);
}

/* ------------------------------------------------------------------ */
/*  Ground (scrolling)                                                */
/* ------------------------------------------------------------------ */
typedef struct {
    int   y, sw;
    float scroll;
} Ground;

static void ground_init(Ground *g, int y, int sw) {
    g->y = y;
    g->sw = sw;
    g->scroll = 0;
}

static void ground_update(Ground *g, float speed) {
    g->scroll += speed;
    if (g->scroll >= 12.0f) g->scroll -= 12.0f;
}

static void ground_draw(const Ground *g, SDL_Renderer *ren) {
    set_color(ren, CLR_GROUND);
    SDL_RenderDrawLine(ren, 0, g->y, g->sw, g->y);

    int bumps[] = {2, 0, 0, 2, 0, 2, 0, 0, 0, 2, 0, 1, 0, 3, 0, 0};
    int nb = sizeof(bumps) / sizeof(bumps[0]);
    int step = 12;
    int off = (int)g->scroll % step;
    for (int i = -off; i < g->sw + step; i += step) {
        if (i < 0) continue;
        int idx = (i / step) % nb;
        int h = bumps[idx];
        if (h > 0) {
            SDL_Rect peb = {i, g->y - h, 3, h};
            fill_rounded(ren, peb, 1);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Cloud                                                             */
/* ------------------------------------------------------------------ */
typedef struct {
    float x, y, speed;
} Cloud;

static void cloud_init(Cloud *c, int sw) {
    c->x = (float)(sw + rand() % 400);
    c->y = (float)(40 + rand() % 80);
    c->speed = 0.8f + (float)rand() / (float)RAND_MAX * 0.7f;
}

static void cloud_update(Cloud *c, float game_speed) {
    c->x -= c->speed * (game_speed / INITIAL_SPEED);
}

static int cloud_off_screen(const Cloud *c) {
    return c->x + 60 < 0;
}

static void cloud_draw(const Cloud *c, SDL_Renderer *ren) {
    set_color(ren, CLR_CLOUD);
    struct { int dx, dy, r; } pts[] = {
        {0, 0, 14}, {12, -4, 10}, {-10, 2, 12}, {18, 2, 8}
    };
    for (int i = 0; i < 4; i++) {
        int cx = (int)c->x + pts[i].dx;
        int cy = (int)c->y + pts[i].dy;
        /* filled circle approximation */
        for (int dy = -pts[i].r; dy <= pts[i].r; dy++) {
            int dx = (int)(0.5f + sqrtf((float)(pts[i].r*pts[i].r - dy*dy)));
            if (dx > 0) {
                SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Game state                                                        */
/* ------------------------------------------------------------------ */
typedef struct {
    Dinosaur  dino;
    Obstacle  obstacles[MAX_OBSTACLES];
    int       num_obstacles;
    Ground    ground;
    Cloud     clouds[MAX_CLOUDS];
    int       num_clouds;
    float     speed;
    float     score;
    int       high_score;
    int       game_over;
    float     next_spawn;
    float     dist_since_spawn;
    TTF_Font *font;
    TTF_Font *big_font;
} Game;

static void game_init(Game *g, TTF_Font *font, TTF_Font *big_font) {
    srand((unsigned)time(NULL));
    dino_init(&g->dino, 80, GROUND_Y);
    g->num_obstacles = 0;
    ground_init(&g->ground, GROUND_Y, SCREEN_WIDTH);
    g->num_clouds = 3;
    for (int i = 0; i < g->num_clouds; i++)
        cloud_init(&g->clouds[i], SCREEN_WIDTH);
    g->speed = INITIAL_SPEED;
    g->score = 0;
    g->high_score = 0;
    g->game_over = 0;
    g->next_spawn = (float)(SPAWN_MIN + rand() % (SPAWN_MAX - SPAWN_MIN));
    g->dist_since_spawn = 0;
    g->font = font;
    g->big_font = big_font;
}

static void game_reset(Game *g) {
    if ((int)g->score > g->high_score) g->high_score = (int)g->score;
    dino_init(&g->dino, 80, GROUND_Y);
    g->num_obstacles = 0;
    ground_init(&g->ground, GROUND_Y, SCREEN_WIDTH);
    g->num_clouds = 3;
    for (int i = 0; i < g->num_clouds; i++)
        cloud_init(&g->clouds[i], SCREEN_WIDTH);
    g->speed = INITIAL_SPEED;
    g->score = 0;
    g->game_over = 0;
    g->next_spawn = (float)(SPAWN_MIN + rand() % (SPAWN_MAX - SPAWN_MIN));
    g->dist_since_spawn = 0;
}

static void game_update(Game *g) {
    if (g->game_over) return;

    dino_update(&g->dino, GROUND_Y);

    /* speed ramp */
    g->speed = INITIAL_SPEED + g->score * SPEED_INCREMENT;
    if (g->speed > MAX_SPEED) g->speed = MAX_SPEED;

    /* spawn obstacles */
    g->dist_since_spawn += g->speed;
    if (g->dist_since_spawn >= g->next_spawn && g->num_obstacles < MAX_OBSTACLES) {
        int size = rand() % 3;
        obs_init(&g->obstacles[g->num_obstacles], (float)SCREEN_WIDTH, GROUND_Y, size);
        g->num_obstacles++;
        g->dist_since_spawn = 0;
        g->next_spawn = (float)(SPAWN_MIN + rand() % (SPAWN_MAX - SPAWN_MIN));
    }

    /* update obstacles */
    for (int i = 0; i < g->num_obstacles; i++) {
        obs_update(&g->obstacles[i], g->speed);
        if (!g->obstacles[i].passed &&
            g->obstacles[i].x + OBS_W < g->dino.x) {
            g->obstacles[i].passed = 1;
            g->score += 1.0f;
        }
    }
    /* remove off-screen obstacles */
    int write = 0;
    for (int i = 0; i < g->num_obstacles; i++) {
        if (!obs_off_screen(&g->obstacles[i]))
            g->obstacles[write++] = g->obstacles[i];
    }
    g->num_obstacles = write;

    /* ground */
    ground_update(&g->ground, g->speed);

    /* clouds */
    for (int i = 0; i < g->num_clouds; i++)
        cloud_update(&g->clouds[i], g->speed);
    write = 0;
    for (int i = 0; i < g->num_clouds; i++) {
        if (!cloud_off_screen(&g->clouds[i]))
            g->clouds[write++] = g->clouds[i];
    }
    g->num_clouds = write;
    while (g->num_clouds < 3) {
        cloud_init(&g->clouds[g->num_clouds], SCREEN_WIDTH);
        g->num_clouds++;
    }

    /* collision */
    SDL_Rect dr = dino_rect(&g->dino);
    dr.x += 8; dr.y += 6; dr.w -= 16; dr.h -= 12;
    for (int i = 0; i < g->num_obstacles; i++) {
        SDL_Rect orr = obs_rect(&g->obstacles[i]);
        orr.x += 4; orr.y += 4; orr.w -= 8; orr.h -= 8;
        if (SDL_HasIntersection(&dr, &orr)) {
            g->game_over = 1;
            if ((int)g->score > g->high_score)
                g->high_score = (int)g->score;
            break;
        }
    }
}

static void render_text(SDL_Renderer *ren, TTF_Font *font,
                        const char *text, SDL_Color color,
                        int x, int y, int center) {
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
    if (!tex) { SDL_FreeSurface(surf); return; }
    SDL_Rect dst;
    if (center) {
        dst.x = x - surf->w / 2;
        dst.y = y - surf->h / 2;
    } else {
        dst.x = x;
        dst.y = y;
    }
    dst.w = surf->w;
    dst.h = surf->h;
    SDL_RenderCopy(ren, tex, NULL, &dst);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

static void game_draw(const Game *g, SDL_Renderer *ren) {
    /* background */
    set_color(ren, CLR_BG);
    SDL_RenderClear(ren);

    /* clouds */
    for (int i = 0; i < g->num_clouds; i++)
        cloud_draw(&g->clouds[i], ren);

    /* ground */
    ground_draw(&g->ground, ren);

    /* obstacles */
    for (int i = 0; i < g->num_obstacles; i++)
        obs_draw(&g->obstacles[i], ren);

    /* dinosaur */
    dino_draw(&g->dino, ren);

    /* score */
    char buf[32];
    snprintf(buf, sizeof(buf), "Score: %d", (int)g->score);
    render_text(ren, g->font, buf, CLR_TEXT,
                SCREEN_WIDTH - 150, 20, 0);

    /* high score */
    snprintf(buf, sizeof(buf), "HI: %d", g->high_score);
    render_text(ren, g->font, buf, CLR_HI,
                SCREEN_WIDTH - 280, 20, 0);

    /* game over */
    if (g->game_over) {
        render_text(ren, g->big_font, "GAME OVER", CLR_TEXT,
                    SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20, 1);
        render_text(ren, g->font, "Press SPACE or R to restart", CLR_HINT,
                    SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 20, 1);
    }
}

/* ------------------------------------------------------------------ */
/*  Main                                                              */
/* ------------------------------------------------------------------ */
int main(int argc, char **argv) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        fprintf(stderr, "TTF_Init error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("Dino Run - SDL2 C",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!win) {
        fprintf(stderr, "Window error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        fprintf(stderr, "Renderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    /* Load font - try a few common paths */
    TTF_Font *font = NULL;
    TTF_Font *big_font = NULL;
    const char *font_paths[] = {
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/Consola.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/Arial.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        NULL
    };
    for (int i = 0; font_paths[i]; i++) {
        font = TTF_OpenFont(font_paths[i], 20);
        if (font) {
            big_font = TTF_OpenFont(font_paths[i], 32);
            break;
        }
    }
    if (!font) {
        fprintf(stderr, "Could not load any font\n");
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    Game game;
    game_init(&game, font, big_font);

    int running = 1;
    Uint32 last_tick = SDL_GetTicks();

    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT)
                running = 0;
            else if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_ESCAPE)
                    running = 0;
                else if (ev.key.keysym.sym == SDLK_SPACE ||
                         ev.key.keysym.sym == SDLK_UP) {
                    if (game.game_over)
                        game_reset(&game);
                    else
                        dino_jump(&game.dino);
                }
                else if (ev.key.keysym.sym == SDLK_r && game.game_over)
                    game_reset(&game);
            }
        }

        game_update(&game);
        game_draw(&game, ren);
        SDL_RenderPresent(ren);

        /* frame cap at FPS */
        Uint32 now = SDL_GetTicks();
        Uint32 elapsed = now - last_tick;
        Uint32 frame_ms = 1000 / FPS;
        if (elapsed < frame_ms)
            SDL_Delay(frame_ms - elapsed);
        last_tick = now;
    }

    TTF_CloseFont(font);
    TTF_CloseFont(big_font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
