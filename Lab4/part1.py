import pygame

class Planet:
    def __init__(self, pos:pygame.Vector2, mass, color):
        self.pos = pos
        self.mass = mass
        self.vel = pygame.Vector2(0,0)
        self.col = color

pygame.init()

main_surface = pygame.display.set_mode((800,600),pygame.SCALED | pygame.RESIZABLE,vsync=1)

running = True
clock = pygame.time.Clock()
planets:list[Planet] = []
colors = ["#FF0000","#00FF00","#0000FF"]

while running:
    for evt in pygame.event.get():
        if evt.type == pygame.QUIT:
            running = False
        if evt.type == pygame.KEYDOWN:
            if evt.key == pygame.K_r:
                planets = []
                main_surface.fill("#000000")
        if len(planets) < 3:
            if evt.type == pygame.MOUSEBUTTONDOWN:
                mass_charge = 0
            if evt.type == pygame.MOUSEBUTTONUP:
                planets.append(Planet(pygame.Vector2(pygame.mouse.get_pos()), mass_charge,colors[len(planets)]))    
        
    dt = .0001 # fixed dt to make the simulation stable
    if pygame.mouse.get_pressed()[0]:
        mass_charge += 100000000*dt

    if len(planets) > 2:
        # compute the forces on each plannet
        for p1 in planets:
            p1.force = pygame.Vector2(0,0)
            for p2 in planets:
                if p1 == p2: continue
                between = p2.pos - p1.pos
                r2 = between.magnitude_squared()
                force_magnitude = p1.mass*p2.mass/r2 # note, we don't actually need the G constant, because it just scales
                p1.force += force_magnitude*between.normalize()
        for p in planets:
            # make em move
            p.acc = p.force/p.mass
            p.vel += p.acc*dt
            p.pos += p.vel*dt 

            
    for p in planets:
        pygame.draw.circle(main_surface, p.col, p.pos, p.mass/1000000)

    pygame.display.flip()