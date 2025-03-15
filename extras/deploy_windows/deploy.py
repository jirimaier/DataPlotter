import os
import shutil
import re
import logging
import subprocess
import urllib.request
import argparse
import zipfile
import pefile
import glob
import create_msix

def get_exe_version(file_path):
    pe = pefile.PE(file_path)
    for file_info in pe.FileInfo:
        for entry in file_info:
            if entry.Key.decode() == "StringFileInfo":
                for sub_entry in entry.StringTable:
                    for key, value in sub_entry.entries.items():
                        if key.decode() == "FileVersion":
                            return value.decode()


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
                "--qmldir",
                "..",
                "--no-translations",
                "--release",
                exe_path,
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


def add_msvc_runtime_libs(app_folder, vcredist_folder):
    """Copy the required Visual C++ runtime DLLs to the application folder."""

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


def add_documentation(app_folder, doc_folder):
    """Copy the documentation folder to the application folder."""
    # copy all pdf and txt files from the documentation folder to the app folder
    for file in os.listdir(doc_folder):
        if file.endswith(".pdf") or file.endswith(".txt"):
            shutil.copy(
                os.path.join(doc_folder, file), os.path.join(app_folder, "documenation")
            )


def delete_folder_contents(folder):
    """Delete all files and folders within a directory."""
    for root, dirs, files in os.walk(folder, topdown=False):
        for file in files:
            os.remove(os.path.join(root, file))
        for directory in dirs:
            try:
                os.rmdir(os.path.join(root, directory))
            except PermissionError:
                logging.warning(
                    f"Permission denied while removing directory {directory} in {folder} (but files in are removed), skipping."
                )


def copy_openssl_libs(app_folder, openssl_folder):
    """Copy OpenSSL libraries to the application folder, searching recursively."""
    openssl_dlls = ["libcrypto*.dll", "libssl*.dll"]

    for dll_pattern in openssl_dlls:
        found_files = glob.glob(
            os.path.join(openssl_folder, "**", dll_pattern), recursive=True
        )

        if found_files:
            for dll_path in found_files:
                shutil.copy(dll_path, app_folder)
                print(f"Copied {os.path.basename(dll_path)} to {app_folder}")
        else:
            raise FileNotFoundError(
                f"No files matching {dll_pattern} found in {openssl_folder}"
            )


def remove_vcredist_installer_from_app_folder(app_folder):
    """Remove the VCRedist installer from the application folder."""
    vcredist_path = os.path.join(app_folder, "vc_redist.x64.exe")
    if os.path.exists(vcredist_path):
        os.remove(vcredist_path)
        logging.warning(
            f"The VC Redist installer is in app directory. Removed {vcredist_path}"
        )
    else:
        print(f"{vcredist_path} is not in {app_folder}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Deploy DataPlotter application for Windows."
    )
    parser.add_argument("--config", default="IPVMS", help="Configuration flags: I=Installer, P=Portable, V=VCRedist libs in portable, M=MSIX, S=OpenSSL")
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
        default=r"C:\\Qt\\Tools\\OpenSSLv3\\Win_x64",
        help="Path to folder where to search for OpenSSL DLLs",
    )
    parser.add_argument(
        "--msix_tool",
        default=r"C:\\Program Files (x86)\\Windows Kits\\10\bin\\10.0.26100.0\\x64\\makeappx.exe",
        help="Path to MakeAppx.exe",
    )

    args = parser.parse_args()

    os.chdir(os.path.dirname(__file__))
    base_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    deploy_dir = os.path.join(os.path.dirname(__file__), "deploy")
    app_folder = os.path.join(deploy_dir, "DataPlotter")

    delete_folder_contents(deploy_dir)
    os.makedirs(deploy_dir, exist_ok=True)
    os.makedirs(app_folder, exist_ok=True)

    shutil.copy2(
        os.path.join(base_dir, "build", "target", "DataPlotter.exe"), app_folder
    )
    deploy_qt_dependencies(app_folder, args.windeployqt)

    if "S" in args.config.upper():
        copy_openssl_libs(app_folder, args.openssl)

    version = get_exe_version(os.path.join(app_folder, "DataPlotter.exe"))
    if not version:
        raise Exception("Could not extract version from DataPlotter.exe")

    if "I" in args.config.upper():
        download_vcredist_installer(deploy_dir)
        remove_vcredist_installer_from_app_folder(app_folder)
        run_inno_setup(version, args.inno)

    if "P" in args.config:
        # make a copy of the app folder for the portable version
        portable_folder = os.path.join(
            deploy_dir, f"DataPlotter_{version.replace('.','_')}_Portable"
        )
        shutil.copytree(app_folder, portable_folder)
        if "V" in args.config.upper():
            add_msvc_runtime_libs(portable_folder, args.vcredist)
        add_documentation(portable_folder, os.path.join(base_dir, "documentation"))

    if "M" in args.config:
        # make a copy of the app folder for the msix version
        msix_folder = os.path.join(
            deploy_dir, f"DataPlotter_{version.replace('.','_')}_MSIX"
        )
        shutil.copytree(app_folder, msix_folder)
        add_msvc_runtime_libs(msix_folder, args.vcredist)
        create_msix.package_application(
            msix_folder,
            args.msix_tool,
            os.path.join(base_dir, "icons", "icon.ico"),
            deploy_dir,
            version,
        )
