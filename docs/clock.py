import pygame
import sys
import os
from datetime import datetime



pygame.init()

# reading display info
info = pygame.display.Info()


#we want window on the right half of the screen
WIDTH = info.current_w//2
HEIGHT = info.current_h

#center of the window
CX = WIDTH // 2
CY = HEIGHT // 2

os.environ['SDL_VIDEO_WINDOW_POS'] = '%d,0' % WIDTH
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption('Clock')
pygame.mouse.set_visible(False)


#colors
BLACK = (0, 0, 0)
BLUE = (0, 150, 255)
DIM_BLUE = (0, 60, 150)


# font size
FONT_SIZE_BIG = 90
FONT_SIZE_SMALL = 50
FONT_SIZE_DATE = 28


# elemnts on screen (offset from the center)
TIME_Y_OFFSET = 60
SECS_Y_OFFSET = 60
LINE_Y_OFFSET = 110
DATE_Y_OFFSET = 150
DATE2_Y_OFFSET = 190
LINE_MARGIN = 50
LINE_THICKNESS = 1

TICK_RATE = 1 


#fonts
font_big = pygame.font.SysFont('monospace', FONT_SIZE_BIG, bold=True)
font_small = pygame.font.SysFont('monospace', FONT_SIZE_SMALL, bold=True)
font_date = pygame.font.SysFont('monospace', FONT_SIZE_DATE)

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

    #current date and time
    now = datetime.now()
    time_str = now.strftime('%H:%M')
    secs_str = now.strftime('%S')
    date_str = now.strftime('%A')
    date_str2 = now.strftime('%d %B %Y')

    screen.fill(BLACK)

    #time
    time_surf = font_big.render(time_str, True, BLUE)
    time_rect = time_surf.get_rect(center=(CX, CY - TIME_Y_OFFSET))
    screen.blit(time_surf, time_rect)

    #seconds
    secs_surf = font_small.render(secs_str, True, DIM_BLUE)
    secs_rect = secs_surf.get_rect(center=(CX, CY + SECS_Y_OFFSET))
    screen.blit(secs_surf, secs_rect)

    pygame.draw.line(screen, DIM_BLUE, (LINE_MARGIN, CY + LINE_Y_OFFSET),(WIDTH - LINE_MARGIN, CY + LINE_Y_OFFSET),LINE_THICKNESS)

    #day in the week
    date_surf = font_date.render(date_str, True, DIM_BLUE)
    date_rect = date_surf.get_rect(center=(CX, CY + DATE_Y_OFFSET))
    screen.blit(date_surf, date_rect)

    #full date
    date_surf2 = font_date.render(date_str2, True, DIM_BLUE)
    date_rect2 = date_surf2.get_rect(center=((CX, CY + DATE2_Y_OFFSET)))
    screen.blit(date_surf2, date_rect2)

    pygame.display.flip()
    clock.tick(TICK_RATE)
