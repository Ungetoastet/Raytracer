from __future__ import annotations

import subprocess
import os
import math
import random

class vec3:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x = x
        self.y = y
        self.z = z

    def __repr__(self):
        return f"vec3({self.x}, {self.y}, {self.z})"

    def __add__(self, other):
        return vec3(self.x + other.x, self.y + other.y, self.z + other.z)

    def __sub__(self, other):
        return vec3(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, scalar):
        return vec3(self.x * scalar, self.y * scalar, self.z * scalar)

    def __truediv__(self, scalar):
        return vec3(self.x / scalar, self.y / scalar, self.z / scalar)

    def dot(self, other):
        return self.x * other.x + self.y * other.y + self.z * other.z

    def magnitude(self):
        return math.sqrt(self.x**2 + self.y**2 + self.z**2)

    def normalize(self):
        mag = self.magnitude()
        return vec3(self.x / mag, self.y / mag, self.z / mag)

    def __neg__(self):
        return vec3(-self.x, -self.y, -self.z)

    def __eq__(self, other):
        return self.x == other.x and self.y == other.y and self.z == other.z

    def __ne__(self, other):
        return not self.__eq__(other)
    
    def abs(self):
        return vec3(abs(self.x), abs(self.y), abs(self.z))
    
    @staticmethod
    def random_on_sphere():
        # Generate random spherical coordinates
        theta = random.uniform(0, 2 * math.pi)  # Azimuthal angle [0, 2π]
        phi = math.acos(random.uniform(-1, 1))  # Polar angle [0, π]
        # Convert spherical coordinates to Cartesian coordinates
        x = math.sin(phi) * math.cos(theta)
        y = math.sin(phi) * math.sin(theta)
        z = math.cos(phi)
        return vec3(x, y, z)
    
    @staticmethod
    def random_in_box(min, max):
        x = random.uniform(min.x, max.x)
        y = random.uniform(min.y, max.y)
        z = random.uniform(min.z, max.z)
        return vec3(x, y, z)


def xml_material(id, color, reflection, roughness):
    return f'<material id="{id}" color="{color.x}, {color.y}, {color.z}" reflection="{reflection}" roughness="{roughness}" />'

def xml_plane(position, rotation, scale, material_id):
    return f'<Plane position="{position.x}, {position.y}, {position.z}" rotation="{rotation.x}, {rotation.y}, {rotation.z}" scale="{scale.x}, {scale.y}, {scale.z}" material="{material_id}" />'

def xml_sphere(position, radius, material_id):
    return f'<Sphere position="{position.x}, {position.y}, {position.z}" radius="{radius}" material="{material_id}" />'

def xml_camera(position, lookAt, fieldOfView):
    return f'<camera position="{position.x}, {position.y}, {position.z}" lookAt="{lookAt.x}, {lookAt.y}, {lookAt.z}" fieldOfView="{fieldOfView}" skybox="true"/>'

def xml_root(materials, objects, camera):
    return  f"""<scene>
    <materials>
        {materials}
    </materials>
    <objects>
        {objects}
    </objects>
    {camera}
</scene>"""

def merge(xml_objs):
    s = ""
    for o in xml_objs:
        s += o + "\n"
    return s

def update_positions(iteration, timestep):
    print(f"Update Physics {iteration}")
    for b in balls:
        b.update_position(timestep, iteration)
    for i in range(len(balls)-1):
        for j in range(i+1, len(balls)):
            balls[i].resolve_collision(balls[j])

def build_xml(iteration, materials, balls):
    print(f"Building XML {iteration}")
    x_mats = [xml_material(m.id, m.color, m.reflection, m.roughness) for m in materials]
    x_mats.append(xml_material(base_mat.id, base_mat.color, base_mat.reflection, base_mat.roughness))
    x_balls = [xml_sphere(s.position, s.size, s.mat) for s in balls]
    x_balls.append(xml_plane(vec3(), vec3(90, 0, 0), vec3(20, 20, 20), base_mat.id))
    x_cam = xml_camera(vec3(12, 10, 12), vec3(5, 5, 5), 45)
    return xml_root(
        merge(x_mats),
        merge(x_balls),
        x_cam
    )

class Ball:
    def __init__(self, position: vec3, velocity: vec3, size, materialID, mass):
        self.position = position
        self.velocity = velocity
        self.size = size
        self.mat = materialID
        self.mass = mass
        
    def update_position(self, timestep, iteration) -> None:
        self.velocity += vec3(0, -GRAVITY * timestep, 0)
        self.position += self.velocity * timestep
        if self.position.y < self.size:
            self.position.y = self.size
            self.velocity.y *= -1
        if self.position.x < self.size - 10:
            self.position.x = self.size - 10
            self.velocity.x *= -1
        elif self.position.x > 10 - self.size:
            self.position.x = 10 - self.size
            self.velocity.x *= -1
        if self.position.z < self.size - 10:
            self.position.z = self.size - 10
            self.velocity.z *= -1
        elif self.position.z > 10 - self.size:
            self.position.z = 10 - self.size
            self.velocity.z *= -1

    def resolve_collision(self, other: Ball) -> None:
        mindist = self.size + other.size
        dist = (self.position - other.position).magnitude()
        
        if dist >= mindist:
            return None
        
        normal = (other.position - self.position).normalize()
        relative_velocity = other.velocity - self.velocity
        velocity_along_normal = relative_velocity.dot(normal)
        if velocity_along_normal > 0:
            return
        
        e = 1.0  # Assume perfectly elastic collision
        j = -(1 + e) * velocity_along_normal
        j /= (1 / self.mass + 1 / other.mass)

        # Update overlaps
        overlap = (mindist - dist) + 1e-4
        self.position = self.position - (normal * (overlap / 2))
        other.position = other.position + (normal * (overlap / 2))

        # Update velocity
        self.velocity = self.velocity - (normal * (j / self.mass))
        other.velocity = other.velocity + (normal * (j / other.mass))

class Material:
    def __init__(self, id, color:vec3, reflection, roughness):
        self.id = id
        self.color = color
        self.reflection = reflection
        self.roughness = roughness
    
FPS = 60
MOVIE_SECONDS = 10
GRAVITY = 1
TIMESCALE = 3
FRAME_COUNT = FPS*MOVIE_SECONDS

balls: list[Ball] = []
base_mat = Material("green", vec3(0.3, 0.7, 0.3), 0.3, 0.5)
materials = [
    Material("emissive_white", vec3(3.0, 3.0, 3.0), 0.3, -1),
    Material("green", vec3(0.3, 0.7, 0.3), 0.3, 0.5),
    Material("lblue", vec3(0.1, 0.1, 1), 0.6, 0.5),
    Material("blue", vec3(0.5, 0.5, 1), 0.2, 0.5),
    Material("red", vec3(1, 0.1, 0.1), 0.6, 0.5),
    Material("emissive_red", vec3(2.0, 0.0, 0.0), 0.3, -1),
    Material("white", vec3(1, 1, 1), 0.8, 0.5),
    Material("white_diff", vec3(1, 1, 1), 0.0, 0.5),
    Material("mirror", vec3(1, 1, 1), 1.0, 0.1),
    Material("black", vec3(1, 0.1, 0.1), 0.4, 0.6)
]


for m in materials:
    newball = Ball(
        vec3.random_in_box(
            vec3(-10, 0, -10), vec3(10, 10, 10)
        ),
        vec3.random_on_sphere() * 5,
        1,
        m.id,
        1
    )
    balls.append(newball)

for i in range(FRAME_COUNT):
    print(f"\nRendering Frame {i+1}/{FRAME_COUNT}")
    update_positions(i, TIMESCALE/FPS)
    data = build_xml(i, materials, balls)
    
    # Write scene information to a file
    with open("movie.scene", "w") as file:
        file.write(data + "\n")
    
    exe_path = ".\\Build\\main.exe"
    process = subprocess.Popen([exe_path, "movie.scene", "Templates/settings_quality.xml"], shell=True)
    process.wait()
    
    # Rename the output image to include the iteration number
    output_image = f"render_{i:04d}.ppm"
    os.rename("render.ppm", output_image)

# Combine the .ppm images into an .mp4 video using ffmpeg
output_video = "output_video.mp4"
image_files = [f"render_{i:04d}.ppm" for i in range(FRAME_COUNT)]

os.system(f"ffmpeg -y -r {FPS} -i render_%04d.ppm -c:v libx264 -pix_fmt yuv420p {output_video} -loglevel error -hide_banner")

for ppm_file in image_files:
    os.remove(ppm_file)

print(f"Video saved as {output_video}")