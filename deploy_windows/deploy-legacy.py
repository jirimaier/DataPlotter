import os
import shutil
import re
import logging
import subprocess
import urllib.request
import argparse
import zipfile


def find_exe(build_dir):
    """Find DataPlotter.exe in the build directory."""
    exe_path = os.path.join(build_dir, "DataPlotter.exe")
    if os.path.exists(exe_path):
        return exe_path
    else:
        raise FileNotFoundError(f"{exe_path} not found!")


def get_version(file_path):
    """Extracts the version from the CMakeLists.txt file."""
    try:
        with open(file_path, "r") as file:
            content = file.read()
            match = re.search(
                r"project\(\s*\w+\s+VERSION\s+([\d.]+)", content, re.IGNORECASE
            )
            if match:
                return match.group(1)
    except FileNotFoundError:
        raise FileNotFoundError(f"{file_path} not found!")

    logging.error("Could not extract version from CMakeLists.txt")
    return None


def deploy_qt_dependencies(app_folder, windeployqt_path):
    """Run windeployqt to add required Qt dependencies including QML files, OpenSSL, and platform plugins."""

    exe_path = os.path.join(app_folder, "DataPlotter.exe")

    if not os.path.exists(windeployqt_path):
        logging.error(f"windeployqt not found at {windeployqt_path}")
        return

    try:
        subprocess.run(
            [
                windeployqt_path, 
                "--qmldir", "..",  
                "--no-translations",
                "--release",
                exe_path
            ],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        print("Successfully deployed Qt dependencies and QML files.")

    except subprocess.CalledProcessError as e:
        logging.error(f"windeployqt failed: {e.stderr.decode()}")


def create_portable_version(app_folder, vcredist_folder, version):
    """Create a portable version of the application by copying necessary files, including VC runtime DLLs."""

    # List of required VC runtime DLLs (MSVC 2015, 32-bit)
    vc_runtime_dlls = [
        "concrt140.dll",
        "msvcp140.dll",
        "vccorlib140.dll",
        "vcruntime140.dll",
    ]

    # Copy each required DLL from vcredist_folder to the application folder
    for dll in vc_runtime_dlls:
        dll_path = os.path.join(vcredist_folder, dll)
        if os.path.exists(dll_path):
            shutil.copy(dll_path, app_folder)
            print(f"Copied {dll} to {app_folder}")
        else:
            raise FileNotFoundError(f"{dll} not found in {vcredist_folder}")

    # Create a ZIP archive for the portable version, including all subdirectories
    zip_filename = f"deploy\\DataPlotter_{version.replace('.', '_')}_Portable.zip"
    with zipfile.ZipFile(zip_filename, "w", zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(app_folder):
            for file in files:
                file_path = os.path.join(root, file)
                archive_name = os.path.relpath(file_path, app_folder)  # Preserve directory structure
                zipf.write(file_path, archive_name)

    print(f"Portable version created as {zip_filename}")


def copy_exe(build_dir, deploy_dir, cmake_file):
    """Copy the executable to a versioned directory within the deployment directory."""
    exe_path = find_exe(build_dir)
    app_folder = os.path.join(deploy_dir, "DataPlotter")
    dest_path = os.path.join(app_folder, "DataPlotter.exe")

    if os.path.exists(app_folder):
        for root, dirs, files in os.walk(app_folder, topdown=False):
            for file in files:
                os.remove(os.path.join(root, file))
            for directory in dirs:
                try:
                    os.rmdir(os.path.join(root, directory))
                except PermissionError:
                    logging.warning(
                        f"Permission denied while removing directory {directory} in {app_folder} (but files inside are removed), skipping."
                    )

    os.makedirs(app_folder, exist_ok=True)
    shutil.copy(exe_path, dest_path)
    print(f"Copied {exe_path} to {dest_path}")

    return app_folder, dest_path


def copy_openssl_libs(app_folder, openssl_folder):
    """Copy OpenSSL libraries to the application folder."""
    openssl_dlls = ["libcrypto-1_1.dll", "libssl-1_1.dll"]
    for dll in openssl_dlls:
        dll_path = os.path.join(openssl_folder, dll)
        if os.path.exists(dll_path):
            shutil.copy(dll_path, app_folder)
            print(f"Copied {dll} to {app_folder}")
        else:
            raise FileNotFoundError(f"{dll} not found in {openssl_folder}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Deploy DataPlotter application for Windows."
    )
    parser.add_argument(
        "--windeployqt",
        default=r"C:\\Qt\\5.7.2\\msvc2015\\bin\\windeployqt.exe",
        help="Path to windeployqt.exe",
    )
    parser.add_argument(
        "--vcredist",
        default=r"C:\\Windows\\System32",
        help="Path to Visual C++ Redistributable DLLs",
    )
    parser.add_argument(
        "--openssl",
        default=r"C:\\Program Files\\OpenSSL-Win32\\bin",
        help="Path to OpenSSL DLLs"
    )

    args = parser.parse_args()

    os.chdir(os.path.dirname(__file__))

    BUILD_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build"))
    DEPLOY_DIR = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "deploy_windows", "deploy")
    )
    CMAKE_FILE = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "CMakeLists.txt")
    )

    try:
        app_folder, exe_path = copy_exe(BUILD_DIR, DEPLOY_DIR, CMAKE_FILE)
        deploy_qt_dependencies(app_folder, args.windeployqt)
        copy_openssl_libs(app_folder, args.openssl)

        version = get_version(CMAKE_FILE)
        if version:
            create_portable_version(app_folder, args.vcredist, version)
        else:
            print("Error: Version could not be determined.")
    except FileNotFoundError as e:
        print(f"Error: {e}")
