import os
import subprocess
import pefile
from PIL import Image, ImageOps
import logging

def _convert_ico_to_png(ico_path, output_folder):
    sizes = [44, 50, 71, 150, 310]
    os.makedirs(output_folder, exist_ok=True)

    with Image.open(ico_path) as img:
        for size in sizes:
            png_path = os.path.join(output_folder, f"DATAPLOTTER-Square{size}x{size}Logo.png")
            img_resized = img.resize((size, size), Image.LANCZOS)
            img_resized.save(png_path, "PNG")
            logging.info(f"Generated {png_path}")

        wide_size = (310, 150)
        wide_png_path = os.path.join(output_folder, "DATAPLOTTER-Wide310x150Logo.png")
        wide_img = ImageOps.contain(img, wide_size, Image.LANCZOS)
        wide_canvas = Image.new("RGBA", wide_size, (255, 255, 255, 0))
        paste_x = (wide_size[0] - wide_img.width) // 2
        paste_y = (wide_size[1] - wide_img.height) // 2
        wide_canvas.paste(wide_img, (paste_x, paste_y))
        wide_canvas.save(wide_png_path, "PNG")
        logging.info(f"Generated properly scaled {wide_png_path}")

def _generate_manifest(app_folder, version, template_path="AppxManifest.xml"):
    with open(template_path, "r", encoding="utf-8") as manifest_file:
        manifest_content = manifest_file.read().replace("{version}", version)
    manifest_path = os.path.join(app_folder, "AppxManifest.xml")
    with open(manifest_path, "w", encoding="utf-8") as manifest_file:
        manifest_file.write(manifest_content)
    logging.info(f"Generated AppxManifest.xml at {manifest_path}")

def _create_msix_package(app_folder, output_folder, msix_tool, version):
    package_path = os.path.join(output_folder, f"JiMaier.DataPlotter_{version}_x64__8987y09q3m1zg.msix")
    os.makedirs(output_folder, exist_ok=True)
    try:
        subprocess.run(
            [msix_tool, "pack", "/d", app_folder, "/p", package_path, "/o"],
            check=True
        )
        logging.info(f"Successfully created MSIX package: {package_path}")
        return package_path
    except subprocess.CalledProcessError as e:
        logging.error(f"Error creating MSIX package: {e}")
        return None

def package_application(app_folder, msix_tool, icon_file, deploy_folder, version):
    while version.count(".") < 3:
        version += ".0"
    
    _convert_ico_to_png(icon_file, os.path.join(app_folder, "Assets"))
    _generate_manifest(app_folder, version)
    return _create_msix_package(app_folder, deploy_folder, msix_tool, version)
