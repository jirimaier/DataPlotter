import os
import sys
import shutil
import subprocess
import multiprocessing
import logging
import re

os.chdir(os.path.dirname(os.path.realpath(__file__)))

def get_version(file_path):
    """Extracts the version from the CMakeLists.txt file."""
    try:
        with open(file_path, 'r') as file:
            content = file.read()
            match = re.search(r'project\(\s*\w+\s+VERSION\s+([\d.]+)', content, re.IGNORECASE)
            if match:
                return match.group(1)
    except FileNotFoundError:
        logging.error("CMakeLists.txt file not found!")
        raise
    
    logging.error("Could not extract version from CMakeLists.txt")
    return None

def check_tool_installed(tool):
    """Checks if a required tool is installed."""
    if shutil.which(tool) is None:
        logging.error(f"{tool} is not installed. Please install it and try again.")
        raise Exception(f"{tool} is required but not installed.")

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

# Get the version
version = get_version('../CMakeLists.txt')
if not version:
    raise Exception("Could not extract version from CMakeLists file")

logging.info(f"Building DataPlotter version {version}")

# Define base directory
deploy_dir = os.path.join("deploy", f"data-plotter_{version.replace('.','_')}_amd64")
if os.path.exists(deploy_dir):
    shutil.rmtree(deploy_dir)

# Create required directories
dirs = [
    os.path.join(deploy_dir, "DEBIAN"),
    os.path.join(deploy_dir, "usr", "bin"),
    os.path.join(deploy_dir, "usr", "share", "doc", "data-plotter"),
    os.path.join(deploy_dir, "usr", "lib"),
    os.path.join(deploy_dir, "usr", "share", "icons", "hicolor", "256x256", "apps"),
    os.path.join(deploy_dir, "usr", "share", "applications"),
]

for dir in dirs:
    os.makedirs(dir, exist_ok=True)

# File copying
files = {
    os.path.join("control"): os.path.join(deploy_dir, "DEBIAN", "control"),
    os.path.join("qt.conf"): os.path.join(deploy_dir, "usr", "bin", "qt.conf"),
    os.path.join("..", "build", "DataPlotter"): os.path.join(deploy_dir, "usr", "bin", "data-plotter"),
    os.path.join("..", "documentation", "license.txt"): os.path.join(deploy_dir, "usr", "share", "doc", "data-plotter", "copyright"),
    os.path.join("icon.png"): os.path.join(deploy_dir, "usr", "share", "icons", "hicolor", "256x256", "apps", "data-plotter.png"),
    os.path.join("data-plotter.desktop"): os.path.join(deploy_dir, "usr", "share", "applications", "data-plotter.desktop")
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
os.chdir(deploy_dir)
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
os.chdir(os.path.dirname(os.path.realpath(__file__)))

# Update control file
file_path = os.path.join(deploy_dir, "DEBIAN", "control")
with open(file_path, "r") as file:
    content = file.read()
    content = content.replace("{version}", version)
    content = content.replace("{depends}", all_deps)

with open(file_path, "w") as file:
    file.write(content)

# Update desktop file
file_path = os.path.join(deploy_dir, "usr", "share", "applications", "data-plotter.desktop")
with open(file_path, "r") as file:
    content = file.read()
    content = content.replace("{version}", version)

with open(file_path, "w") as file:
    file.write(content)

# Set permissions
subprocess.run(["chmod", "755", os.path.join(deploy_dir, "usr", "bin", "data-plotter")], check=True)
subprocess.run(["chmod", "-R", "755", os.path.join(deploy_dir, "DEBIAN")], check=True)

# Handle postinst script
shutil.copy2("postinst", os.path.join(deploy_dir, "DEBIAN", "postinst"))
os.chmod(os.path.join(deploy_dir, "DEBIAN", "postinst"), 0o755)

# Build Debian package
subprocess.run(["dpkg-deb", "--build", deploy_dir], check=True)
logging.info("Debian package created successfully!")
