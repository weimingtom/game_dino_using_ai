import pygame
import random
import sys

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 400
GROUND_Y = 320                     # Y coordinate of the ground line
FPS = 60
GRAVITY = 0.7
JUMP_VELOCITY = -14
INITIAL_SPEED = 6
MAX_SPEED = 14
SPEED_INCREMENT = 0.002
OBSTACLE_SPAWN_MIN = 800
OBSTACLE_SPAWN_MAX = 1800          # pixels travelled before next obstacle
BACKGROUND_COLOR = (247, 247, 247)              # off-white like Chrome
GROUND_COLOR = (83, 83, 83)                     # dark gray ground line
TEXT_COLOR = (83, 83, 83)

# ---------------------------------------------------------------------------
# Dinosaur
# ---------------------------------------------------------------------------
class Dinosaur:
    WIDTH = 44
    HEIGHT = 48

    def __init__(self, x, ground_y):
        self.x = x
        self.ground_y = ground_y
        self.y = ground_y - self.HEIGHT
        self.vy = 0
        self.on_ground = True
        self.leg_frame = 0              # 0 or 1 for the two running poses
        self.leg_timer = 0

    def jump(self):
        if self.on_ground:
            self.vy = JUMP_VELOCITY
            self.on_ground = False

    def update(self):
        self.y += self.vy
        self.vy += GRAVITY

        if self.y >= self.ground_y - self.HEIGHT:
            self.y = self.ground_y - self.HEIGHT
            self.vy = 0
            self.on_ground = True

        # Trot animation while on ground
        if self.on_ground:
            self.leg_timer += 1
            if self.leg_timer >= 6:
                self.leg_timer = 0
                self.leg_frame = (self.leg_frame + 1) % 2

    def rect(self):
        return pygame.Rect(self.x, self.y, self.WIDTH, self.HEIGHT)

    def draw(self, screen):
        r = self.rect()
        body_color = (50, 50, 50)
        eye_color = BACKGROUND_COLOR
        pupil_color = (30, 30, 30)

        # Body (a simple rounded dinosaur shape built from rectangles)
        body_rect = pygame.Rect(r.x + 2, r.y + 4, 34, 40)
        pygame.draw.rect(screen, body_color, body_rect, border_radius=6)

        # Head
        head_rect = pygame.Rect(r.x + 28, r.y + 2, 18, 18)
        pygame.draw.rect(screen, body_color, head_rect, border_radius=4)

        # Eye
        eye_x = r.x + 38
        eye_y = r.y + 8
        pygame.draw.circle(screen, eye_color, (eye_x, eye_y), 5)
        pygame.draw.circle(screen, pupil_color, (eye_x + 1, eye_y), 2)

        # Mouth line
        mouth_start = (r.x + 40, r.y + 16)
        mouth_end = (r.x + 46, r.y + 13)
        pygame.draw.line(screen, body_color, mouth_start, mouth_end, 2)

        # Tail
        tail_points = [
            (r.x + 2, r.y + 20),
            (r.x - 8, r.y + 14),
            (r.x - 6, r.y + 24),
            (r.x + 2, r.y + 28),
        ]
        pygame.draw.polygon(screen, body_color, tail_points)

        # Arms (small rectangle)
        arm_y = r.y + 22
        pygame.draw.rect(screen, body_color, (r.x + 8, arm_y, 4, 10), border_radius=2)

        # Legs (animate between two frames)
        if self.leg_frame == 0:
            # Standing leg
            pygame.draw.rect(screen, body_color, (r.x + 14, r.y + 38, 8, 10), border_radius=2)
            pygame.draw.rect(screen, body_color, (r.x + 26, r.y + 40, 8, 8), border_radius=2)
        else:
            # Rear leg forward
            pygame.draw.rect(screen, body_color, (r.x + 14, r.y + 40, 8, 8), border_radius=2)
            pygame.draw.rect(screen, body_color, (r.x + 26, r.y + 38, 8, 10), border_radius=2)


# ---------------------------------------------------------------------------
# Obstacle (cactus)
# ---------------------------------------------------------------------------
class Obstacle:
    WIDTH = 20
    HEIGHTS = [36, 48, 40]            # three cactus sizes

    def __init__(self, x, ground_y, size_idx):
        self.x = x
        self.ground_y = ground_y
        self.h = self.HEIGHTS[size_idx]
        self.y = ground_y - self.h
        self.passed = False

    def update(self, speed):
        self.x -= speed

    def off_screen(self):
        return self.x + self.WIDTH < 0

    def rect(self):
        return pygame.Rect(self.x, self.y, self.WIDTH, self.h)

    def draw(self, screen):
        r = self.rect()
        color = (83, 83, 83)

        # Main trunk
        pygame.draw.rect(screen, color, (r.x + 6, r.y, 8, r.h), border_radius=3)

        # Left arm
        arm_h = max(6, int(r.h * 0.35))
        pygame.draw.rect(screen, color, (r.x, r.y + 6, 6, arm_h), border_radius=2)
        # Right arm (staggered higher)
        arm2_h = max(4, int(r.h * 0.25))
        pygame.draw.rect(screen, color, (r.x + 14, r.y + 10, 6, arm2_h), border_radius=2)


# ---------------------------------------------------------------------------
# Ground (scrolling)
# ---------------------------------------------------------------------------
class Ground:
    def __init__(self, y, screen_width):
        self.y = y
        self.sw = screen_width
        self.scroll = 0
        self.bump_pattern = [2, 0, 0, 2, 0, 2, 0, 0, 0, 2, 0, 1, 0, 3, 0, 0]

    def update(self, speed):
        self.scroll = (self.scroll + speed) % 12

    def draw(self, screen):
        y = self.y
        # Draw the ground line (2px)
        pygame.draw.line(screen, GROUND_COLOR, (0, y), (self.sw, y), 2)

        # Draw little bumps / pebbles scrolling
        step = 12
        offset = int(self.scroll) % step
        for i in range(-offset, self.sw + step, step):
            if i < 0:
                continue
            # Add a tiny random-looking bump
            idx = (i // step) % len(self.bump_pattern)
            h = self.bump_pattern[idx]
            if h > 0:
                pygame.draw.rect(screen, GROUND_COLOR, (i, y - h, 3, h), border_radius=1)


# ---------------------------------------------------------------------------
# Cloud
# ---------------------------------------------------------------------------
class Cloud:
    def __init__(self, screen_width):
        self.x = random.randint(screen_width, screen_width + 400)
        self.y = random.randint(40, 120)
        self.speed = random.uniform(0.8, 1.5)

    def update(self, game_speed):
        self.x -= self.speed * (game_speed / INITIAL_SPEED)

    def off_screen(self):
        return self.x + 60 < 0

    def draw(self, screen):
        color = (200, 200, 200)
        x, y = self.x, self.y
        for dx, dy, r in [(0, 0, 14), (12, -4, 10), (-10, 2, 12), (18, 2, 8)]:
            pygame.draw.circle(screen, color, (int(x + dx), int(y + dy)), r)


# ---------------------------------------------------------------------------
# Game
# ---------------------------------------------------------------------------
class Game:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
        pygame.display.set_caption("Dino Run - SDL2 / Pygame")
        self.clock = pygame.time.Clock()
        self.font = pygame.font.SysFont("Arial", 20, bold=True)
        self.big_font = pygame.font.SysFont("Arial", 32, bold=True)

        self.reset()

    def reset(self):
        self.dino = Dinosaur(80, GROUND_Y)
        self.obstacles = []
        self.ground = Ground(GROUND_Y, SCREEN_WIDTH)
        self.clouds = [Cloud(SCREEN_WIDTH) for _ in range(3)]
        self.speed = INITIAL_SPEED
        self.score = 0
        self.high_score = max(self.score, getattr(self, 'high_score', 0))
        self.game_over = False
        self.next_spawn = random.randint(OBSTACLE_SPAWN_MIN, OBSTACLE_SPAWN_MAX)
        self.distance_since_spawn = 0

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE or event.key == pygame.K_UP:
                    if self.game_over:
                        self.reset()
                    else:
                        self.dino.jump()
                if event.key == pygame.K_r and self.game_over:
                    self.reset()
                if event.key == pygame.K_ESCAPE:
                    return False
        return True

    def update(self):
        if self.game_over:
            return

        self.dino.update()

        # Increase speed
        self.speed = min(MAX_SPEED, INITIAL_SPEED + self.score * SPEED_INCREMENT)

        # Spawn obstacles
        self.distance_since_spawn += self.speed
        if self.distance_since_spawn >= self.next_spawn:
            size = random.randint(0, 2)
            self.obstacles.append(Obstacle(SCREEN_WIDTH, GROUND_Y, size))
            self.distance_since_spawn = 0
            self.next_spawn = random.randint(OBSTACLE_SPAWN_MIN, OBSTACLE_SPAWN_MAX)

        # Update obstacles
        for obs in list(self.obstacles):
            obs.update(self.speed)
            if obs.off_screen():
                self.obstacles.remove(obs)
            elif not obs.passed and obs.x + Obstacle.WIDTH < self.dino.x:
                obs.passed = True
                self.score += 1

        # Update ground
        self.ground.update(self.speed)

        # Clouds
        for c in list(self.clouds):
            c.update(self.speed)
            if c.off_screen():
                self.clouds.remove(c)
        while len(self.clouds) < 3:
            self.clouds.append(Cloud(SCREEN_WIDTH))

        # Collision detection
        dino_rect = self.dino.rect().inflate(-8, -6)
        for obs in self.obstacles:
            obs_rect = obs.rect().inflate(-4, -4)
            if dino_rect.colliderect(obs_rect):
                self.game_over = True
                if self.score > self.high_score:
                    self.high_score = self.score

    def draw(self):
        self.screen.fill(BACKGROUND_COLOR)

        # Clouds
        for c in self.clouds:
            c.draw(self.screen)

        # Ground
        self.ground.draw(self.screen)

        # Obstacles
        for obs in self.obstacles:
            obs.draw(self.screen)

        # Dinosaur
        self.dino.draw(self.screen)

        # Score
        score_surf = self.font.render(f"Score: {int(self.score)}", True, TEXT_COLOR)
        self.screen.blit(score_surf, (SCREEN_WIDTH - 150, 20))

        # High score
        hs_surf = self.font.render(f"HI: {int(self.high_score)}", True, (160, 160, 160))
        self.screen.blit(hs_surf, (SCREEN_WIDTH - 280, 20))

        # Game over overlay
        if self.game_over:
            overlay = pygame.Surface((SCREEN_WIDTH, SCREEN_HEIGHT), pygame.SRCALPHA)
            overlay.fill((255, 255, 255, 0))
            self.screen.blit(overlay, (0, 0))

            go_surf = self.big_font.render("GAME OVER", True, TEXT_COLOR)
            go_rect = go_surf.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 - 20))
            self.screen.blit(go_surf, go_rect)

            restart_surf = self.font.render("Press SPACE or R to restart", True, (150, 150, 150))
            restart_rect = restart_surf.get_rect(center=(SCREEN_WIDTH // 2, SCREEN_HEIGHT // 2 + 20))
            self.screen.blit(restart_surf, restart_rect)

    def run(self):
        running = True
        while running:
            running = self.handle_events()
            self.update()
            self.draw()
            pygame.display.flip()
            self.clock.tick(FPS)

        pygame.quit()
        sys.exit()


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------
if __name__ == "__main__":
    Game().run()
