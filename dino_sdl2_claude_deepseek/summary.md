# Dino Run — C 语言 SDL2 实现

纯 C + SDL2 + SDL2_ttf 实现的 Chrome 恐龙跑酷游戏。

## 编译 & 运行

### 安装依赖

**MSYS2 / MinGW-w64 (Windows):**
```bash
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
```

**Linux (Debian/Ubuntu):**
```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev
```

**macOS:**
```bash
brew install sdl2 sdl2_ttf
```

### 编译
```bash
cd dino_c
make          # 编译
make run      # 编译并运行
```

或手动：
```bash
gcc -Wall -O2 -std=c99 -o dino_game dino_game.c -lSDL2 -lSDL2_ttf -lm
./dino_game
```

## 操作

| 按键 | 功能 |
|---|---|
| 空格 / 上箭头 | 跳跃 |
| R / 空格 | 游戏结束后重新开始 |
| ESC | 退出 |

## 文件结构

```
dino_c/
├── dino_game.c   (11.5 KB — 完整游戏源码)
├── Makefile       (构建脚本)
└── summary.md     (本文档)
```

## 代码结构

| 结构体 | 职责 |
|---|---|
| `Dinosaur` | 角色物理（重力、跳跃速度）、两帧跑步动画、矩形/圆形组合绘制 |
| `Obstacle` | 仙人掌障碍物，三种高度（36/48/40 像素），带枝干 |
| `Ground` | 滚动地面线 + 石子装饰纹理 |
| `Cloud` | 背景云朵，视差滚动，随机位置和速度 |
| `Game` | 聚合状态，spawn 逻辑，碰撞检测，分数管理 |

### 核心函数

| 函数 | 说明 |
|---|---|
| `dino_init / dino_jump / dino_update / dino_draw` | 恐龙生命周期 |
| `obs_init / obs_update / obs_draw` | 障碍物生命周期 |
| `game_init / game_reset / game_update / game_draw` | 主游戏逻辑 |
| `fill_rounded` | SDL2 无原生圆角矩形，使用多矩形组合近似 |
| `render_text` | 通过 SDL2_ttf 将字符串渲染为纹理 |

## 与 Python 版本的差异

| 方面 | Python (pygame) | C (SDL2) |
|---|---|---|
| 圆角矩形 | `pygame.draw.rect(border_radius=)` | 手动 `fill_rounded` |
| 圆形 | `pygame.draw.circle` | 逐行扫描填充圆 |
| 文字 | `font.render` 直接返回 Surface | `TTF_RenderUTF8_Blended` + `SDL_CreateTextureFromSurface` |
| 颜色 | `pygame.Color` | `SDL_Color` + `SDL_SetRenderDrawColor` |
| 内存管理 | GC | 手动管理（本游戏主要用栈分配） |

## 核心数值（与 Python 版一致）

| 参数 | 值 |
|---|---|
| 画面尺寸 | 800×400 |
| 帧率 | 60 FPS |
| 重力加速度 | 0.7 px/frame |
| 跳跃初速度 | -14 px/frame |
| 初始速度 | 6 px/frame |
| 最高速度 | 14 px/frame |
| 速度增量 | 0.002 / 得分 |
