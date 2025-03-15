import os
import shutil
import re
import logging
import subprocess
import urllib.request
import argparse
import zipfile

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


def download_vcredist_installer(deploy_folder):
    """Download VCRedist.exe to the tools directory if it does not already exist."""
    vcredist_url = "https://aka.ms/vs/17/release/vc_redist.x64.exe"
    vcredist_path = os.path.join(deploy_folder, "vc_redist.x64.exe")

    if not os.path.exists(vcredist_path):
        print("Downloading Visual C++ Redistributable...")
        try:
            os.makedirs(deploy_folder, exist_ok=True)
            urllib.request.urlretrieve(vcredist_url, vcredist_path)
            print("Downloaded VCRedist.exe successfully.")
        except Exception as e:
            logging.error(f"Failed to download VCRedist.exe: {e}")
    else:
        print("VCRedist.exe already exists in tools folder.")


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



def run_inno_setup(version, inno_compiler):
    """Run Inno Setup with the extracted version and executable path."""
    inno_setup_script = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "innosetup_script.iss")
    )

    if not os.path.exists(inno_compiler):
        logging.error("Inno Setup compiler not found!")
        return

    try:
        subprocess.run(
            [
                inno_compiler,
                f"/DAppVersion={version}",
                inno_setup_script,
            ],
            check=True,
        )
        print(f"Inno Setup successfully created the installer for version {version}.")
    except subprocess.CalledProcessError as e:
        logging.error(f"Inno Setup failed: {e}")


def create_portable_version(app_folder, vcredist_folder, version):
    """Create a portable version of the application by copying necessary files, including VC runtime DLLs."""

    # List of required VC runtime DLLs
    vc_runtime_dlls = [
        "concrt140.dll",
        "msvcp140.dll",
        "msvcp140_1.dll",
        "msvcp140_2.dll",
        "msvcp140_atomic_wait.dll",
        "msvcp140_codecvt_ids.dll",
        "vccorlib140.dll",
        "vcruntime140.dll",
        "vcruntime140_1.dll",
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
                        f"Permission denied while removing directory {directory} in {app_folder} (but files in are removed), skipping.")

    os.makedirs(app_folder, exist_ok=True)
    shutil.copy(exe_path, dest_path)
    print(f"Copied {exe_path} to {dest_path}")

    return app_folder, dest_path

def copy_openssl_libs(app_folder, openssl_folder):
    """Copy OpenSSL libraries to the application folder."""
    openssl_dlls = ["libcrypto-1_1-x64.dll", "libssl-1_1-x64.dll"]
    for dll in openssl_dlls:
        dll_path = os.path.join(openssl_folder, dll)
        if os.path.exists(dll_path):
            shutil.copy(dll_path, app_folder)
            print(f"Copied {dll} to {app_folder}")
        else:
            raise FileNotFoundError(f"{dll} not found in {openssl_folder}")
        
def remove_vcredist_installer_from_app_folder(app_folder):
    """Remove the VCRedist installer from the application folder."""
    vcredist_path = os.path.join(app_folder, "vc_redist.x64.exe")
    if os.path.exists(vcredist_path):
        os.remove(vcredist_path)
        logging.warning(f"The VC Redist installer is in app directory. Removed {vcredist_path}")
    else:
        print(f"{vcredist_path} is not in {app_folder}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Deploy DataPlotter application for Windows."
    )
    parser.add_argument(
        "--windeployqt",
        default=r"C:\\Qt\\5.15.2\\msvc2019_64\\bin\\windeployqt.exe",
        help="Path to windeployqt.exe",
    )
    parser.add_argument(
        "--inno",
        default=r"C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe",
        help="Path to Inno Setup compiler",
    )
    parser.add_argument(
        "--vcredist",
        default=r"C:\\Windows\\System32",
        help="Path to Visual C++ Redistributable DLLs",
    )
    parser.add_argument(
        "--openssl",
         default=r"C:\\Program Files\\OpenSSL-Win64\\bin",
         help="Path to OpenSSL DLLs"
    )
    parser.add_argument(
        "--executable",
        help="Path to the DataPlotter executable",
        default=r"..\\build\\target\\DataPlotter.exe"
    )

    args = parser.parse_args()

    os.chdir(os.path.dirname(__file__))

    BUILD_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "build"))
    DEPLOY_DIR = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "deploy")
    )

    try:
        app_folder, exe_path = copy_exe(BUILD_DIR, DEPLOY_DIR, CMAKE_FILE)
        deploy_qt_dependencies(app_folder, args.windeployqt)
        copy_openssl_libs(app_folder, args.openssl)
        download_vcredist_installer(DEPLOY_DIR)

        version = get_version(CMAKE_FILE)
        if version:
            remove_vcredist_installer_from_app_folder(app_folder)
            run_inno_setup(version, args.inno)
            create_portable_version(app_folder, args.vcredist, version)
        else:
            print("Error: Version could not be determined.")
    except FileNotFoundError as e:
        print(f"Error: {e}")
