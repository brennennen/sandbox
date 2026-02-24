import os
import urllib.request
import zipfile
import tarfile
import platform

SDL_VER = "3.4.0"
VENDOR_DIR = ".vendor"

def download_sdl():
    os.makedirs(VENDOR_DIR, exist_ok=True)
    system = platform.system().lower()

    if system == "windows":
        url = f"https://github.com/libsdl-org/SDL/releases/download/release-{SDL_VER}/SDL3-devel-{SDL_VER}-mingw.zip"
        ext = ".zip"
    else:
        url = f"https://github.com/libsdl-org/SDL/releases/download/release-{SDL_VER}/SDL3-{SDL_VER}.tar.gz"
        ext = ".tar.gz"

    archive_path = os.path.join(VENDOR_DIR, f"sdl3{ext}")
    print(f"Downloading SDL3 from {url}...")
    urllib.request.urlretrieve(url, archive_path)

    if ext == ".zip":
        with zipfile.ZipFile(archive_path, 'r') as zip_ref:
            zip_ref.extractall(VENDOR_DIR)
    else:
        with tarfile.open(archive_path, 'r:gz') as tar_ref:
            tar_ref.extractall(VENDOR_DIR)

    print("SDL3 installed to .vendor/")

if __name__ == "__main__":
    download_sdl()