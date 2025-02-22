import os
import shutil
import subprocess
import argparse
from PIL import Image, ImageOps
import logging
import re

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

def convert_ico_to_png(ico_path, output_folder):
    """Converts an ICO file into multiple PNG sizes required for MSIX icons."""
    sizes = [44, 50, 71, 150, 310]  # Standard sizes
    os.makedirs(output_folder, exist_ok=True)

    with Image.open(ico_path) as img:
        for size in sizes:
            png_path = os.path.join(output_folder, f"DATAPLOTTER-Square{size}x{size}Logo.png")
            img_resized = img.resize((size, size), Image.LANCZOS)
            img_resized.save(png_path, "PNG")
            print(f"Generated {png_path}")

        # Handle wide tile (310x150) properly by adding padding
        wide_size = (310, 150)
        wide_png_path = os.path.join(output_folder, "DATAPLOTTER-Wide310x150Logo.png")

        # Preserve aspect ratio while fitting within 310x150
        wide_img = ImageOps.contain(img, wide_size, Image.LANCZOS)
        wide_canvas = Image.new("RGBA", wide_size, (255, 255, 255, 0))  # Transparent background
        paste_x = (wide_size[0] - wide_img.width) // 2  # Center horizontally
        paste_y = (wide_size[1] - wide_img.height) // 2  # Center vertically
        wide_canvas.paste(wide_img, (paste_x, paste_y))

        wide_canvas.save(wide_png_path, "PNG")
        print(f"Generated properly scaled {wide_png_path}")


def generate_manifest(app_folder, version):
    """Generates an AppxManifest.xml file for MSIX packaging."""

    with open("AppxManifest.xml", "r", encoding="utf-8") as manifest_file:
        manifest_content = manifest_file.read()
        manifest_content = manifest_content.replace("{version}", version)
    
    manifest_path = os.path.join(app_folder, "AppxManifest.xml")
    with open(manifest_path, "w", encoding="utf-8") as manifest_file:
        manifest_file.write(manifest_content)
    print(f"Generated AppxManifest.xml at {manifest_path}")

def create_msix_package(app_folder, output_folder, msix_tool, version):
    """Creates an MSIX package using MakeAppx and SignTool."""
    
    package_path = os.path.join(output_folder, f"JiMaier.DataPlotter_{version}_x64__8987y09q3m1zg.msix")
    
    os.makedirs(output_folder, exist_ok=True)
    
    # Create the MSIX package
    try:
        subprocess.run(
            [msix_tool, "pack", 
             "/d", app_folder, 
             "/p", package_path,
             "/o"  # Force overwrite without prompting
             ],
            check=True
        )
        print(f"Successfully created MSIX package: {package_path}")
    except subprocess.CalledProcessError as e:
        print(f"Error creating MSIX package: {e}")
        return

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Create an MSIX package for DataPlotter.")
    parser.add_argument("--msix_tool", default=r"C:\\Program Files (x86)\\Windows Kits\\10\bin\\10.0.26100.0\\x64\\makeappx.exe", help="Path to MakeAppx.exe")
    app_folder = "deploy/DataPlotter"
    deploy_folder = "deploy"
    
    args = parser.parse_args()

    os.chdir(os.path.dirname(__file__))
    
    CMAKE_FILE = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "CMakeLists.txt")
    )
    version = get_version(CMAKE_FILE)

    while version.count(".") < 3:
        version += ".0"  
    
    convert_ico_to_png("..\\icons\\icon.ico", os.path.join(app_folder, "Assets"))
    generate_manifest(app_folder, version)

    create_msix_package(
        app_folder=app_folder,
        output_folder=deploy_folder, 
        msix_tool=args.msix_tool,
        version=version
    )
