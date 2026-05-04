import pygame
import sys
import os
from datetime import datetime

os.environ['SDL_VIDEO_WINDOW_POS'] = '512,0'

pygame.init()

WIDTH, HEIGHT = 512, 768
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption('Clock')
pygame.mouse.set_visible(False)

BLACK = (0, 0, 0)
GREEN = (0, 150, 255)
DIM_GREEN = (0, 60, 150)

font_big = pygame.font.SysFont('monospace', 90, bold=True)
font_small = pygame.font.SysFont('monospace', 50, bold=True)
font_date = pygame.font.SysFont('monospace', 28)

clock = pygame.time.Clock()

while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            sys.exit()
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_q or event.key == pygame.K_ESCAPE:
                pygame.quit()
                sys.exit()

    now = datetime.now()
    time_str = now.strftime('%H:%M')
    secs_str = now.strftime('%S')
    date_str = now.strftime('%A')
    date_str2 = now.strftime('%d %B %Y')

    screen.fill(BLACK)

    time_surf = font_big.render(time_str, True, GREEN)
    time_rect = time_surf.get_rect(center=(WIDTH//2, HEIGHT//2 - 60))
    screen.blit(time_surf, time_rect)

    secs_surf = font_small.render(secs_str, True, DIM_GREEN)
    secs_rect = secs_surf.get_rect(center=(WIDTH//2, HEIGHT//2 + 60))
    screen.blit(secs_surf, secs_rect)

    pygame.draw.line(screen, DIM_GREEN, (50, HEIGHT//2 + 110), (WIDTH-50, HEIGHT//2 + 110), 1)

    date_surf = font_date.render(date_str, True, DIM_GREEN)
    date_rect = date_surf.get_rect(center=(WIDTH//2, HEIGHT//2 + 150))
    screen.blit(date_surf, date_rect)

    date_surf2 = font_date.render(date_str2, True, DIM_GREEN)
    date_rect2 = date_surf2.get_rect(center=(WIDTH//2, HEIGHT//2 + 190))
    screen.blit(date_surf2, date_rect2)

    pygame.display.flip()
    clock.tick(1)
