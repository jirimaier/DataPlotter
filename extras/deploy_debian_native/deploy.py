import enum
import logging
import os
import re
import shutil
import subprocess
import sys
import urllib.request


class Target(enum.Enum):
    DEBIAN = 1
    APPIMAGE = 2


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

def create_directories(deploy_dir, target: Target):
    """Creates necessary directories for the Debian package."""
    dirs = [
        os.path.join(deploy_dir, "usr", "bin"),
        os.path.join(deploy_dir, "usr", "share", "doc", "data-plotter"),
        os.path.join(deploy_dir, "usr", "lib"),
        os.path.join(deploy_dir, "usr", "share", "icons", "hicolor", "256x256", "apps"),
        os.path.join(deploy_dir, "usr", "share", "applications"),
    ]
    if target == Target.DEBIAN:
        dirs.append(os.path.join(deploy_dir, "DEBIAN"))
    for dir in dirs:
        os.makedirs(dir, exist_ok=True)

def copy_files(deploy_dir, target: Target):
    """Copies necessary files into the deployment directory."""
    files = {
        "../../build/target/data-plotter": os.path.join(deploy_dir, "usr", "bin", "data-plotter"),
        "../../documentation/license.txt": os.path.join(deploy_dir, "usr", "share", "doc", "data-plotter", "copyright"),
        "icon.png": os.path.join(deploy_dir, "usr", "share", "icons", "hicolor", "256x256", "apps", "data-plotter.png"),
        "data-plotter.desktop": os.path.join(deploy_dir, "usr", "share", "applications", "data-plotter.desktop")
    }
    if target == Target.DEBIAN:
        files.update({
            "../packaging/deb/debian/control": os.path.join(deploy_dir, "DEBIAN", "control"),
            "qt.conf": os.path.join(deploy_dir, "usr", "bin", "qt.conf"),
        })
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
    return shlib_deps

def update_control_file(deploy_dir, version, dependencies):
    """Updates the Debian control file with the correct version and dependencies."""
    file_path = os.path.join(deploy_dir, "DEBIAN", "control")
    with open(file_path, "r") as file:
        content = file.read().replace("${shlibs:Depends}", dependencies).replace("${misc:Depends},","")
        content = "\n".join([line for line in content.splitlines() if line.strip()])
        content = content.rstrip('\n') + '\n' + f"Version: {version}\n"
    os.remove(file_path)
    with open(file_path, "w") as file:
        file.write(content)

def update_desktop_file(deploy_dir, version):
    """Updates the .desktop file with the correct version."""
    file_path = os.path.join(deploy_dir, "usr", "share", "applications", "data-plotter.desktop")
    with open(file_path, "r") as file:
        content = file.read().replace("{version}", version)
    with open(file_path, "w") as file:
        file.write(content)

def set_permissions(deploy_dir, target: Target):
    """Sets correct permissions for executable and DEBIAN directory."""
    subprocess.run(["chmod", "755", os.path.join(deploy_dir, "usr", "bin", "data-plotter")], check=True)
    if target == Target.DEBIAN:
        subprocess.run(["chmod", "-R", "755", os.path.join(deploy_dir, "DEBIAN")], check=True)

def handle_postinst_script(deploy_dir):
    """Copies and sets permissions for postinst script."""
    shutil.copy2("postinst", os.path.join(deploy_dir, "DEBIAN", "postinst"))
    os.chmod(os.path.join(deploy_dir, "DEBIAN", "postinst"), 0o755)

def build_package(deploy_dir):
    """Builds the Debian package."""
    subprocess.run(["dpkg-deb", "--build", deploy_dir], check=True)
    logging.info("Debian package created successfully!")

def add_platform_files(app_folder, platform):
    if os.path.exists(os.path.join(app_folder, "platform.cfg")):
        os.remove(os.path.join(app_folder, "platform.cfg"))
    with open(os.path.join(app_folder, "platform.cfg"), "w") as f:
        f.write(platform)

def update_appimage_recipe(deploy_root, version):
    """Updates the AppImageBuilder recipe file with the correct version."""
    final_recipe = os.path.join(deploy_root, "AppImageBuilder.yml")
    with open('AppImageBuilder.yml', "r") as file:
        content = file.read().replace("{version}", version)
    with open(final_recipe, "w") as file:
        file.write(content)

def download_appimage_tools(deploy_root):
    appimage_builder = os.path.join(deploy_root, "appimage-builder")
    appimagetool = os.path.join(deploy_root, "appimagetool")
    urllib.request.urlretrieve("https://github.com/AppImageCrafters/appimage-builder/releases/download/v1.1.0/"
                               "appimage-builder-1.1.0-x86_64.AppImage", appimage_builder)
    urllib.request.urlretrieve("https://github.com/AppImage/appimagetool/releases/download/continuous/"
                               "appimagetool-x86_64.AppImage", appimagetool)
    subprocess.run(["chmod", "755", appimage_builder], check=True)
    subprocess.run(["chmod", "755", appimagetool], check=True)


def build_appimage(deploy_root, deploy_dir, version):
    """Builds the AppImage bundle."""

    # First, complete the AppDir with external libraries using AppImageBuilder (see the recipe for details).
    subprocess.run([os.path.join(deploy_root, "appimage-builder"),
                    "--skip-appimage",  # this is deferred to a different tool
                    "--appdir", deploy_dir,
                    "--recipe", os.path.join(deploy_root, "AppImageBuilder.yml"),
                    "--build-dir", os.path.join(deploy_root, "appimage-build")], check=True)

    # Then, we use appimagetool directly to package the AppDir into a self-contained AppImage.
    # There are two reasons for using appimagetool directly:
    #  - We can use a newer version of it that uses an AppImage "type2 runtime" / "static runtime".
    #    This allows us to avoid issues with missing libfuse2 package on recent Ubuntu versions.
    #  - The newer tool and runtime support zstd compression, which leads to app launch times much faster than xz.

    # place where external auto-update tools will look for incremental update files
    update_info = "gh-releases-zsync|jirimaier|DataPlotter|latest|DataPlotter-*x86_64.AppImage.zsync"

    appimagetool_env = os.environ.copy()
    appimagetool_env["ARCH"] = "x86_64"
    appimagetool_env["APPIMAGETOOL_APP_NAME"] = "DataPlotter"
    appimagetool_env["VERSION"] = version

    subprocess.run([os.path.join(deploy_root, "appimagetool"), "--updateinformation", update_info, deploy_dir],
        env=appimagetool_env,
        cwd=deploy_root,
        check=True)

    logging.info("AppImage created successfully!")

def main():
    """Main function to execute the build steps."""
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    setup_logging()

    if len(sys.argv) < 2:
        target = Target.DEBIAN
    else:
        target_str = sys.argv[1]
        if target_str == "debian":
            target = Target.DEBIAN
        elif target_str == "appimage":
            target = Target.APPIMAGE
        else:
            raise ValueError(f"Unknown target {target_str}, valid options are 'debian' and 'appimage'")

    version = get_version("../../CMakeLists.txt")
    if not version:
        raise Exception("Could not extract version from CMakeLists file")

    deploy_root = os.path.abspath("deploy")
    deploy_dir = os.path.join(deploy_root, f"data-plotter_{version.replace('.','_')}_amd64")
    if os.path.exists(deploy_root):
        shutil.rmtree(deploy_root)
    
    create_directories(deploy_dir, target)
    copy_files(deploy_dir, target)

    if target == Target.DEBIAN:
        check_tool_installed("dpkg-shlibdeps")
        dependencies = generate_dependencies(deploy_dir)
        update_control_file(deploy_dir, version, dependencies)
        update_desktop_file(deploy_dir, version)
        set_permissions(deploy_dir, target)
        handle_postinst_script(deploy_dir)
        build_package(deploy_dir)
    elif target == Target.APPIMAGE:
        update_appimage_recipe(deploy_root, version)
        download_appimage_tools(deploy_root)
        build_appimage(deploy_root, deploy_dir, version)
    else:
        raise NotImplementedError(f"Target {target} is not yet supported")

if __name__ == "__main__":
    main()
