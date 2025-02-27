import os
import shutil
import subprocess
import logging
import re
import urllib.request


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

def setup_logging():
    """Configures logging."""
    logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')

def create_directories(app_dir):
    """Creates necessary directories for the Debian package."""
    dirs = [
        os.path.join(app_dir, "usr", "bin"),
        os.path.join(app_dir, "usr", "share", "doc", "data-plotter"),
        os.path.join(app_dir, "usr", "share", "icons", "hicolor", "256x256", "apps"),
        os.path.join(app_dir, "usr", "share", "applications"),
    ]
    for dir in dirs:
        os.makedirs(dir, exist_ok=True)

def copy_files(app_dir):
    """Copies necessary files into the deployment directory."""
    files = {
        "../build/DataPlotter": os.path.join(app_dir, "usr", "bin", "data-plotter"),
        "../documentation/license.txt": os.path.join(app_dir, "usr", "share", "doc", "data-plotter", "copyright"),
        "icon.png": os.path.join(app_dir, "usr", "share", "icons", "hicolor", "256x256", "apps", "data-plotter.png"),
        "data-plotter.desktop": os.path.join(app_dir, "usr", "share", "applications", "data-plotter.desktop")
    }
    for src, dst in files.items():
        try:
            shutil.copy2(src, dst)
        except FileNotFoundError:
            logging.error(f"Missing file: {src}")
            raise

def update_desktop_file(app_dir, version):
    """Updates the .desktop file with the correct version."""
    file_path = os.path.join(app_dir, "usr", "share", "applications", "data-plotter.desktop")
    with open(file_path, "r") as file:
        content = file.read().replace("{version}", version)
    with open(file_path, "w") as file:
        file.write(content)

def update_recipe(src_dir, output_dir, version):
    """Updates the .desktop file with the correct version."""
    template = os.path.join(src_dir, 'deploy_linux', 'AppImageBuilder.yml')
    final_recipe = os.path.join(output_dir, 'AppImageBuilder.yml')
    with open(template, "r") as file:
        content = file.read().replace("{version}", version)
    with open(final_recipe, "w") as file:
        file.write(content)
    return final_recipe

def set_permissions(app_dir):
    """Sets correct permissions for executable and DEBIAN directory."""
    subprocess.run(["chmod", "755", os.path.join(app_dir, "usr", "bin", "data-plotter")], check=True)

def download_tools(output_dir):
    appimage_builder = os.path.join(output_dir, "appimage-builder")
    appimagetool = os.path.join(output_dir, "appimagetool")
    urllib.request.urlretrieve("https://github.com/AppImageCrafters/appimage-builder/releases/download/v1.1.0/"
                               "appimage-builder-1.1.0-x86_64.AppImage", appimage_builder)
    urllib.request.urlretrieve("https://github.com/AppImage/appimagetool/releases/download/continuous/"
                               "appimagetool-x86_64.AppImage", appimagetool)
    subprocess.run(["chmod", "755", appimage_builder], check=True)
    subprocess.run(["chmod", "755", appimagetool], check=True)
    return appimage_builder, appimagetool

def build_appimage(appimage_builder, appimagetool, recipe, version, output_dir, app_dir):
    """Builds the AppImage bundle."""

    # First, complete the AppDir with external libraries using AppImageBuilder (see the recipe for details).
    subprocess.run([appimage_builder,
                    "--skip-appimage",  # this is deferred to a different tool
                    "--appdir", app_dir,
                    "--recipe", recipe,
                    "--build-dir", os.path.join(output_dir, 'appimage-build')], check=True)

    # Then, we use appimagetool directly to package the AppDir into a self-contained AppImage.
    # There are two reasons for using appimagetool directly:
    #  - We can use a newer version of it that uses an AppImage "type2 runtime" / "static runtime".
    #    This allows us to avoid issues with missing libfuse2 package on recent Ubuntu versions.
    #  - The newer tool and runtime support zstd compression, which leads to app launch times much faster than xz.

    # place where external auto-update tools will look for incremental update files
    update_info = 'gh-releases-zsync|jirimaier|DataPlotter|latest|DataPlotter-*x86_64.AppImage.zsync'

    appimagetool_env = os.environ.copy()
    appimagetool_env['ARCH'] = 'x86_64'
    appimagetool_env['APPIMAGETOOL_APP_NAME'] = 'DataPlotter'
    appimagetool_env['VERSION'] = version

    subprocess.run([appimagetool, "--updateinformation", update_info, app_dir],
                   env=appimagetool_env,
                   cwd=os.path.dirname(app_dir),
                   check=True)

    logging.info("AppImage created successfully!")


def main():
    """Main function to execute the build steps."""
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    setup_logging()

    version = get_version('../CMakeLists.txt')
    if not version:
        raise Exception("Could not extract version from CMakeLists file")

    logging.info(f"Building DataPlotter version {version}")

    src_dir = os.path.abspath('..')
    output_dir = os.path.abspath("deploy")
    app_dir = os.path.join(output_dir, "AppDir")

    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)

    create_directories(app_dir)
    copy_files(app_dir)
    update_desktop_file(app_dir, version)
    set_permissions(app_dir)
    recipe = update_recipe(src_dir, output_dir, version)
    appimage_builder, appimagetool = download_tools(output_dir)
    build_appimage(appimage_builder, appimagetool, recipe, version, output_dir, app_dir)

if __name__ == "__main__":
    main()
