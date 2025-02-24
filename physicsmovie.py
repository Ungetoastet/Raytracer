import subprocess
import os
import imageio
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
    return f'<camera position="{position.x}, {position.y}, {position.z}" lookAt="{lookAt.x}, {lookAt.y}, {lookAt.z}" fieldOfView="{fieldOfView}" />'

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
    return [s + o + "\n" for o in xml_objs]

def update_positions(iteration):
    print(f"Update Physics {iteration}")
    return f"Data for iteration {iteration}"

def build_xml(iteration):
    return

FPS = 30
FRAMECOUNT = 200
SPHERE_COUNT = 15

spheres = []

for i in range(FRAMECOUNT):
    update_positions()
    data = build_xml(i)
    
    # Write scene information to a file
    with open("test.scene", "w") as file:
        file.write(data + "\n")
    
    exe_path = "./main.exe"
    process = subprocess.Popen([exe_path], shell=True)
    process.wait()
    
    # Rename the output image to include the iteration number
    output_image = f"render_{i:04d}.ppm"
    os.rename("render.ppm", output_image)

# Combine the .ppm images into an .mp4 video using ffmpeg
output_video = "output_video.mp4"
image_files = [f"render_{i:04d}.ppm" for i in range(FRAMECOUNT)]

# Use imageio to create the video
with imageio.get_writer(output_video, fps=FPS) as writer:
    for img in image_files:
        image = imageio.imread(img)
        writer.append_data(image)

for ppm_file in image_files:
    os.remove(ppm_file)

print(f"Video saved as {output_video}")