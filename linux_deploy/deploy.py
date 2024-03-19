import os
import sys
import shutil
import subprocess
import multiprocessing
import logging

def get_version(file_path):
    """Extracts the version from the .pro file."""
    try:
        with open(file_path, 'r') as file:
            for line in file:
                if line.startswith('VERSION'):
                    return ".".join(line.split('=')[1].strip().split(".")[:3])
    except FileNotFoundError:
        logging.error(".pro file not found!")
        raise
    return None

def check_tool_installed(tool):
    """Checks if a required tool is installed."""
    if shutil.which(tool) is None:
        logging.error(f"{tool} is not installed. Please install it and try again.")
        raise Exception(f"{tool} is required but not installed.")

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

# Confirm current directory
current_dir = os.getcwd()
if not current_dir.endswith("/workspaces/DataPlotter"):
    raise Exception("Please make sure you are in the /workspaces/DataPlotter/ directory")

# Get the version
version = get_version('DataPlotter.pro')
if not version:
    raise Exception("Could not extract version from .pro file")

logging.info(f"Building DataPlotter version {version}")

# Define base directory
base_dir = os.path.join("linux_deploy", f"data-plotter_{version}_amd64")
if os.path.exists(base_dir):
    shutil.rmtree(base_dir)

# Create required directories
dirs = [
    os.path.join(base_dir, "DEBIAN"),
    os.path.join(base_dir, "usr", "bin"),
    os.path.join(base_dir, "usr", "share", "doc", "data-plotter"),
    os.path.join(base_dir, "usr", "lib"),
    os.path.join(base_dir, "usr", "share", "icons", "hicolor", "256x256", "apps"),
    os.path.join(base_dir, "usr", "share", "applications"),
]

for dir in dirs:
    os.makedirs(dir, exist_ok=True)

# Handle rebuild based on command-line argument -r
rebuild = '-r' in sys.argv
if rebuild:
    logging.info("Rebuild flag detected. Rebuilding project: deleting build directory.")
    if os.path.exists("build"):
        shutil.rmtree("build")
    os.makedirs("build")
else:
    if not os.path.exists("build"):
        os.makedirs("build")

# Build process
os.chdir("build")
subprocess.run(["qmake", ".."], check=True)
subprocess.run(["make", f"-j{multiprocessing.cpu_count()}"], check=True)
os.chdir("..")

# File copying
files = {
    os.path.join("linux_deploy", "control"): os.path.join(base_dir, "DEBIAN", "control"),
    os.path.join("linux_deploy", "qt.conf"): os.path.join(base_dir, "usr", "bin", "qt.conf"),
    os.path.join("build", "DataPlotter"): os.path.join(base_dir, "usr", "bin", "data-plotter"),
    os.path.join("documentation", "license.txt"): os.path.join(base_dir, "usr", "share", "doc", "data-plotter", "copyright"),
    os.path.join("linux_deploy", "icon.png"): os.path.join(base_dir, "usr", "share", "icons", "hicolor", "256x256", "apps", "data-plotter.png"),
    os.path.join("linux_deploy", "data-plotter.desktop"): os.path.join(base_dir, "usr", "share", "applications", "data-plotter.desktop")
}

for src, dst in files.items():
    try:
        shutil.copy2(src, dst)
    except FileNotFoundError:
        logging.error(f"Missing file: {src}")
        raise

# Check required tools
check_tool_installed("dpkg-shlibdeps")

# Generate dependencies
os.chdir(base_dir)
os.rename("DEBIAN", "debian")
shlib_deps_output = subprocess.check_output(["dpkg-shlibdeps", "-O", "usr/bin/data-plotter"])
shlib_deps = shlib_deps_output.decode().strip().split("=", 1)[1]

qml_modules = [
    "qml-module-qtquick-controls2",
    "qml-module-qtquick-controls",
    "qml-module-qtquick-layouts",
    "qml-module-qtquick-dialogs",
    "qml-module-qtgraphicaleffects",
    "qml-module-qtquick-window2",
    "qml-module-qtquick2",
    "qml-module-qtcharts"
]

all_deps = f"{shlib_deps}, {', '.join(qml_modules)}"

os.rename("debian", "DEBIAN")
os.chdir("../..")

# Update control file
file_path = os.path.join(base_dir, "DEBIAN", "control")
with open(file_path, "r") as file:
    content = file.read()
    content = content.replace("{version}", version)
    content = content.replace("{depends}", all_deps)

with open(file_path, "w") as file:
    file.write(content)

# Update desktop file
file_path = os.path.join(base_dir, "usr", "share", "applications", "data-plotter.desktop")
with open(file_path, "r") as file:
    content = file.read()
    content = content.replace("{version}", version)

with open(file_path, "w") as file:
    file.write(content)

# Set permissions
subprocess.run(["chmod", "755", os.path.join(base_dir, "usr", "bin", "data-plotter")], check=True)
subprocess.run(["chmod", "-R", "755", os.path.join(base_dir, "DEBIAN")], check=True)

# Handle postinst script
postinst_path = os.path.join("linux_deploy", "postinst")
shutil.copy2(postinst_path, os.path.join(base_dir, "DEBIAN", "postinst"))
os.chmod(os.path.join(base_dir, "DEBIAN", "postinst"), 0o755)

# Delete old .deb files
for file in os.listdir(current_dir):
    if file.endswith(".deb"):
        os.remove(file)
        logging.info(f"Deleted {file}")

# Build Debian package
subprocess.run(["dpkg-deb", "--build", base_dir], check=True)
logging.info("Debian package created successfully!")

# Delete temporary files
shutil.rmtree(base_dir)
