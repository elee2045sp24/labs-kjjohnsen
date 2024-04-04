import pygame
from pygame import Vector2,Rect
import numpy as np
import time
import paho.mqtt.client as mqtt
import uuid

# handles M5Stick tilt and button press
class ControllerHandler:
    def __init__(self):
        self.acc_topic =  "elee2045sp24/controller/kjohnsen/acc"
        self.but_topic =  "elee2045sp24/controller/kjohnsen/but"
        self.mqttClient = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2,str(uuid.uuid4()))
        self.mqttClient.username_pw_set("class_user","class_password")
        self.mqttClient.on_message = self.on_message
        self.mqttClient.connect("mqtt.ugavel.com")
        self.mqttClient.loop_start()
        self.mqttClient.subscribe(self.acc_topic)
        self.mqttClient.subscribe(self.but_topic)
        self.acc = [0,0,0]
        self.but = 0
    def on_message(self, client_obj: mqtt.Client, userdata, message: mqtt.MQTTMessage):
        if message.topic == self.acc_topic:
            self.acc = [float(x) for x in (message.payload.decode('utf8')).split(",")]
        if message.topic == self.but_topic:
            self.but = float(message.payload.decode('utf8'))

class Enemy:
    def __init__(self, pos: Vector2, speed):
        self.pos = pos
        self.speed = speed
        self.extents = Rect(-10,-10, 20, 20)  
        self.destroyed = False # set when you want to delete it
    
    def update(self, dt):
        self.pos += Vector2(0,self.speed)*dt
        self.rect = self.extents.move(self.pos)

    def draw(self, surface):
        pygame.draw.rect(surface,"#00FF00",self.rect)

class Asteroid:
    def __init__(self, pos: Vector2, speed):
        self.pos = pos
        self.speed = speed
        self.health = 100
        
    def update(self, dt):
        self.pos += Vector2(0,self.speed)*dt
        self.extents = Rect(-10,-10, self.health/5, self.health/5)  
        self.rect = self.extents.move(self.pos)

    def draw(self, surface):
        pygame.draw.rect(surface,"#00FFFF",self.rect)

class Player:
    def __init__(self, pos: Vector2):
        self.pos = pos
        self.vel = Vector2(0,0)
        self.extents = Rect(-10,-10,20,20)
        self.destroyed = False

    def update(self, dt, left:float, right:float):
        self.acc = (right-left)*Vector2(1000,0)
        self.vel += self.acc*dt
        self.vel.x = abs(right-left)*pygame.math.clamp(self.vel.x,-300,300)
        self.pos = self.pos + self.vel*dt
        self.pos.x = pygame.math.clamp(self.pos.x,0,800)
        self.rect = self.extents.move(self.pos)

    def draw(self, surface):
        pygame.draw.rect(surface, "#0000FF", self.rect)

class Missile:
    def __init__(self, pos: Vector2, speed: float):
        self.pos = pos.copy() # good practice to copy object references passed in that are part of our state
        self.speed = speed # pixels / s 
        self.direction = Vector2(0,-1)
        self.extents = Rect(-1,1,2,4)
        self.destroyed = False # set when you want to delete it

    def update(self, dt):
        self.pos = self.pos + self.speed*self.direction*dt
        self.rect = self.extents.move(self.pos)

    def draw(self, surface):
        pygame.draw.rect(surface,"#FF0000",self.rect, width = 1)
class Game:
    def __init__(self):
        self.missiles:list[Missile] = []
        self.player = Player(Vector2(screen_width/2, screen_height-50))
        self.score = 0
        self.enemies:list[Enemy] = []
        self.asteroids:list[Asteroid] = []
        self.state = "PLAYING"
        self.reset_time = 3
        self.end_time = 0

pygame.init()
running = True
screen_width, screen_height = 800,600
font = pygame.font.Font(pygame.font.get_default_font(), 36) # initialize a system font
main_surface = pygame.display.set_mode((screen_width,screen_height),pygame.SCALED | pygame.RESIZABLE,vsync=1)
clock = pygame.time.Clock()
        
G = Game()
C = ControllerHandler()
can_fire = True # needed to stop multiple firing from happening

while running:
    for evt in pygame.event.get():
        if evt.type == pygame.QUIT:
            running = False
        if evt.type == pygame.KEYDOWN:
            if evt.key == pygame.K_p:
                if G.state == "PLAYING":
                    G.state = "PAUSED"
                elif G.state == "PAUSED":
                    G.state = "PLAYING"
        if evt.type == pygame.MOUSEBUTTONDOWN and G.state == "PLAYING":
            G.missiles.append(Missile(Vector2(G.player.pos),300))

    # this part makes it so that you have to release the m5 button to fire
    if can_fire and C.but > 0:
        G.missiles.append(Missile(Vector2(G.player.pos),300))
        can_fire = False
    if not can_fire and C.but == 0:
        can_fire = True

    dt = clock.tick(60)/1000 # advance the tick

    if G.state == "PLAYING":
        # spawn enemies and asteroids randomly
        if np.random.rand() < .01:
            G.enemies.append(Enemy(Vector2(np.random.rand()*700+50,20),50))
        if np.random.rand() < .05:
            G.asteroids.append(Asteroid(Vector2(np.random.rand()*700+50,20),50))

        keys = pygame.key.get_pressed()
        # this part adds the acc tilt to the control signal, so you can still use the keys
        left_tilt = 0
        right_tilt = 0
        if C.acc[0] < 0:
            right_tilt = -C.acc[0]
        elif C.acc[0] > 0:
            left_tilt = C.acc[0]
        G.player.update(dt, (1 if keys[pygame.K_a] else 0)+left_tilt, (1 if keys[pygame.K_d] else 0)+right_tilt)

        for m in G.missiles:
            m.update(dt)
        for e in G.enemies:
            e.update(dt)
        for a in G.asteroids:
            a.update(dt)
        
        # detect collisions and handle effects
        # do any of the missiles hit any of the enemies?
        for m in G.missiles:
            if m.pos.y < 0:
                m.destroyed = True
            for e in G.enemies:
                if m.rect.colliderect(e.rect):
                    m.destroyed = True
                    e.destroyed = True
                    G.score += 1
            for a in G.asteroids:
                if m.rect.colliderect(a.rect):
                    m.destroyed = True
                    a.health -= 50 # reduce the health

        # do any of the asteroids or enemies hit the player
        for a in G.asteroids:
            if a.rect.colliderect(G.player.rect):
                G.player.destroyed = True
        for e in G.enemies:
            if e.rect.colliderect(G.player.rect):
                G.player.destroyed = True

        # remove any missiles or enemies that have been destroyed
        G.missiles = list(filter(lambda x: x.destroyed == False, G.missiles))
        G.enemies = list(filter(lambda x: x.destroyed == False, G.enemies))
        G.asteroids = list(filter(lambda x: x.health > 0, G.asteroids))
        if G.player.destroyed:
            G.end_time = time.time()
            G.state = "GAME OVER"

        # draw everything
        main_surface.fill("#000000")
        G.player.draw(main_surface)
        for m in G.missiles:
            m.draw(main_surface)
        for e in G.enemies:
            e.draw(main_surface)
        for a in G.asteroids:
            a.draw(main_surface)

        t = font.render(f"Score: {G.score}", True, "#FFFFFF")
        main_surface.blit(t, (5,5))
        
    elif G.state == "GAME OVER":

        if time.time() - G.end_time > G.reset_time:
            G = Game() # easy way to reset!

        t = font.render(f"Game Over!", True, "#FFFFFF")
        x,y = t.get_rect().center
        main_surface.blit(t,(screen_width/2-x, screen_height/2-y))
    
    elif G.state == "PAUSED":
        pass # just do nothing!

    pygame.display.flip()