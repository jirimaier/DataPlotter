import os
import shutil
import subprocess
import logging
import re

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

def setup_logging():
    """Configures logging."""
    logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

def create_directories(deploy_dir):
    """Creates necessary directories for the Debian package."""
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

def copy_files(deploy_dir, base_dir):
    """Copies necessary files into the deployment directory."""
    files = {
        "control": os.path.join(deploy_dir, "DEBIAN", "control"),
        "qt.conf": os.path.join(deploy_dir, "usr", "bin", "qt.conf"),
        "../../build/target/DataPlotter": os.path.join(deploy_dir, "usr", "bin", "data-plotter"),
        "../../documentation/license.txt": os.path.join(deploy_dir, "usr", "share", "doc", "data-plotter", "copyright"),
        "icon.png": os.path.join(deploy_dir, "usr", "share", "icons", "hicolor", "256x256", "apps", "data-plotter.png"),
        "data-plotter.desktop": os.path.join(deploy_dir, "usr", "share", "applications", "data-plotter.desktop")
    }
    for src, dst in files.items():
        try:
            shutil.copy2(src, dst)
        except FileNotFoundError:
            logging.error(f"Missing file: {src}")
            raise

def generate_dependencies(deploy_dir):
    """Generates package dependencies using dpkg-shlibdeps."""
    prev_dir = os.getcwd()
    os.chdir(deploy_dir)
    os.rename("DEBIAN", "debian")
    shlib_deps_output = subprocess.check_output(["dpkg-shlibdeps", "-O", "usr/bin/data-plotter"])
    shlib_deps = shlib_deps_output.decode().strip().split("=", 1)[1]
    os.rename("debian", "DEBIAN")
    os.chdir(prev_dir)
    
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
    return f"{shlib_deps}, {', '.join(qml_modules)}"

def update_control_file(deploy_dir, version, dependencies):
    """Updates the Debian control file with the correct version and dependencies."""
    file_path = os.path.join(deploy_dir, "DEBIAN", "control")
    with open(file_path, "r") as file:
        content = file.read().replace("{version}", version).replace("{depends}", dependencies)
    with open(file_path, "w") as file:
        file.write(content)

def update_desktop_file(deploy_dir, version):
    """Updates the .desktop file with the correct version."""
    file_path = os.path.join(deploy_dir, "usr", "share", "applications", "data-plotter.desktop")
    with open(file_path, "r") as file:
        content = file.read().replace("{version}", version)
    with open(file_path, "w") as file:
        file.write(content)

def set_permissions(deploy_dir):
    """Sets correct permissions for executable and DEBIAN directory."""
    subprocess.run(["chmod", "755", os.path.join(deploy_dir, "usr", "bin", "data-plotter")], check=True)
    subprocess.run(["chmod", "-R", "755", os.path.join(deploy_dir, "DEBIAN")], check=True)

def handle_postinst_script(deploy_dir):
    """Copies and sets permissions for postinst script."""
    shutil.copy2("postinst", os.path.join(deploy_dir, "DEBIAN", "postinst"))
    os.chmod(os.path.join(deploy_dir, "DEBIAN", "postinst"), 0o755)

def build_package(deploy_dir):
    """Builds the Debian package."""
    subprocess.run(["dpkg-deb", "--build", deploy_dir], check=True)
    logging.info("Debian package created successfully!")

def main():
    """Main function to execute the build steps."""
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    setup_logging()
    
    version = get_version("../../CMakeLists.txt")
    if not version:
        raise Exception("Could not extract version from CMakeLists file")
    
    logging.info(f"Building DataPlotter version {version}")
    deploy_dir = os.path.join("deploy", f"data-plotter_{version.replace('.','_')}_amd64")
    if os.path.exists(deploy_dir):
        shutil.rmtree(deploy_dir)
    
    create_directories(deploy_dir)
    copy_files(deploy_dir)
    check_tool_installed("dpkg-shlibdeps")
    dependencies = generate_dependencies(deploy_dir)
    update_control_file(deploy_dir, version, dependencies)
    update_desktop_file(deploy_dir, version)
    set_permissions(deploy_dir)
    handle_postinst_script(deploy_dir)
    build_package(deploy_dir)

if __name__ == "__main__":
    main()
