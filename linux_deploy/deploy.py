import os
import shutil
import subprocess

# Function to get the version from the .pro file
def get_version(file_path):
    with open(file_path, 'r') as file:
        for line in file:
            if line.startswith('VERSION'):
                return line.split('=')[1].strip()
    return None


# Get the version from the .pro file
version = get_version('DataPlotter.pro')
# Trim the version to three numbers
version = ".".join(version.split(".")[:3])

# Define the base directory
base_dir = os.path.join("linux_deploy",f"data-plotter_{version}_amd64")

if os.path.exists(base_dir):
    shutil.rmtree(base_dir)

# Define the directories to be created
dirs = [
    os.path.join(base_dir, "DEBIAN"),
    os.path.join(base_dir, "usr", "bin"),
    os.path.join(base_dir, "usr", "share", "doc", "data-plotter"),
    os.path.join(base_dir, "usr", "lib"),
    os.path.join(base_dir,"usr","share","icons","hicolor","256x256","apps"),
    os.path.join(base_dir,"usr","share","applications"),
]

# Create the directories
for dir in dirs:
    os.makedirs(dir, exist_ok=True)

# Define the files to be copied
files = {
    os.path.join("linux_deploy","control"): os.path.join(base_dir, "DEBIAN", "control"),
    os.path.join("linux_deploy","qt.conf"): os.path.join(base_dir, "usr", "bin", "qt.conf"),
    os.path.join("build","DataPlotter"): os.path.join(base_dir, "usr", "bin", "data-plotter"),
    os.path.join("documentation","license.txt"): os.path.join(base_dir, "usr", "share", "doc", "data-plotter", "copyright"),
    os.path.join("linux_deploy","icon.png"):os.path.join(base_dir,"usr","share","icons","hicolor","256x256","apps","data-plotter.png"),
    os.path.join("linux_deploy","data-plotter.desktop"):os.path.join(base_dir,"usr","share","applications","data-plotter.desktop")
}

# Copy the files
for src, dst in files.items():
    shutil.copy(src, dst)

with open(os.path.join(base_dir, "DEBIAN", "control"), "r") as file:
    content = file.read()
content = content.replace("{version}", f"{version}")
with open(os.path.join(base_dir, "DEBIAN", "control"), "w") as file:
    file.write(content)

# Run ldd on the executable
result = subprocess.run(["ldd", os.path.join("build","DataPlotter")], capture_output=True, text=True)

# Extract the paths of the shared libraries
libs = [line.split(" ")[2] for line in result.stdout.splitlines() if "=>" in line and not "linux-vdso" in line]

for lib in libs:
    shutil.copy(lib, os.path.join(base_dir, "usr", "lib"))

shutil.copytree(os.path.join("linux_deploy","plugins"),os.path.join(base_dir,"usr","lib","qt5","plugins"))
shutil.copytree(os.path.join("linux_deploy","qml"),os.path.join(base_dir,"usr","lib","qt5","qml"))

# Print a success message
print(f"The Debian package folder structure for {base_dir} has been created and the files have been copied.")

os.remove(base_dir + ".deb")

#result = subprocess.run(["dpkg-deb", "--build", base_dir], capture_output=True, text=True)
#print(result)