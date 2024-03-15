import pygame
from pygame import Vector2,Rect,Surface
import numpy as np

class City: 
    def __init__(self, pos: Vector2, surface: Surface):
        self.pos = pos.copy()
        self.surface = surface
        self.bounds = surface.get_rect().move(self.pos)
        self.destroyed = False
    def draw(self, surface: Surface):
        if not self.destroyed:
            surface.blit(self.surface,self.pos)
        else:
            pass 

class Missile:
    def __init__(self, pos: Vector2, target: Vector2, speed, is_enemy=False):
        self.is_enemy = is_enemy
        self.pos = pos.copy() # good practice to copy object references passed in that are part of our state
        self.start_pos = pos.copy()
        self.target_pos = target.copy()
        self.speed = speed # pixels / s 
        self.distance_traveled = 0
        self.explode_distance = (self.target_pos - self.start_pos).magnitude()
        self.direction = (self.target_pos - self.start_pos).normalize()
        self.exploded = False
        self.explosion_timer = .2
        self.explosion_speed = 300
        self.explosion_radius = 0

    def update(self, dt):

        # traveling to target
        if not self.exploded:
            self.distance_traveled += self.speed*dt
            self.pos = self.start_pos + self.distance_traveled*self.direction
            if self.distance_traveled >= self.explode_distance:
                self.exploded = True
            return

        # in the exploded state
        self.explosion_timer -= dt
        self.explosion_radius += 300*dt

        # check other missiles to see if you hit them
        for m in missiles:
            if m == self: continue
            if not m.exploded and (m.pos - self.pos).magnitude() < self.explosion_radius:
                m.exploded = True
        
        # check if your missile hit a city
        for c in cities:
            if not c.destroyed and c.bounds.collidepoint(self.pos):
                c.destroyed = True
            

    def draw(self, surface):
        if not self.exploded:
            pygame.draw.aaline(surface,"#FF0000" if self.is_enemy else "#0000FF",self.start_pos,self.pos,2)
        else:
            pygame.draw.circle(surface,"#FF0000",self.pos,self.explosion_radius, width = 1)


pygame.init()

main_surface = pygame.display.set_mode((800,600),pygame.SCALED | pygame.RESIZABLE,vsync=1)

# this code illustrates how to resize an image that is the wrong width
city_surface = pygame.image.load("city.jpeg")
city_width = city_surface.get_rect().width
city_surface = pygame.transform.scale_by(city_surface,100/city_width)
running = True
missiles:list[Missile] = []
cities:list[City] = [City(Vector2(50,550), city_surface), City(Vector2(650,550), city_surface)]
clock = pygame.time.Clock()

while running:
    for evt in pygame.event.get():
        if evt.type == pygame.QUIT:
            running = False
        if evt.type == pygame.MOUSEBUTTONDOWN:
            missiles.append(Missile(Vector2(400,600), Vector2(pygame.mouse.get_pos()),300))
        

    dt = clock.tick(60)/1000

    # some random probability of sending a missile
    if np.random.random() < .01: # can adjust higher to increase difficulty
        # choose a random city to attack
        city_number = np.random.randint(0,len(cities))
        missiles.append(Missile(Vector2(np.random.random()*800,0),Vector2(cities[city_number].bounds.center),speed=50, is_enemy=True))
    
    for m in missiles:
        m.update(dt)
    

    #  delete any missiles that should be deleted
    missiles = list(filter(lambda x: x.explosion_timer > 0, missiles))


    main_surface.fill("#000000")
    for m in missiles:
        m.draw(main_surface)
    for c in cities:
        c.draw(main_surface)

    pygame.display.flip()