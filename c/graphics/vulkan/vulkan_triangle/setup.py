#!/usr/bin/env python
import os
import urllib.request
import zipfile
import shutil
import tarfile
import platform

VENDOR_DIR = ".vendor"

SDL_VER = "3.4.0"
VENDOR_DIR = ".vendor"

VOLK_H = "https://raw.githubusercontent.com/zeux/volk/master/volk.h"
VOLK_C = "https://raw.githubusercontent.com/zeux/volk/master/volk.c"

VK_VERSION = "1.4.341"
VK_HEADERS_URL = f"https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v{VK_VERSION}.zip"

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

def download_volk():
    volk_dir = os.path.join(VENDOR_DIR, "volk")
    os.makedirs(volk_dir, exist_ok=True)
    print("Downloading volk...")
    urllib.request.urlretrieve(VOLK_H, os.path.join(volk_dir, "volk.h"))
    urllib.request.urlretrieve(VOLK_C, os.path.join(volk_dir, "volk.c"))
    print("Volk ready.")

def download_vulkan_headers():
    print("Downloading Vulkan headers...")
    zip_path = os.path.join(VENDOR_DIR, "vk_headers.zip")
    req = urllib.request.Request(VK_HEADERS_URL, headers={'User-Agent': 'Mozilla/5.0'})
    with urllib.request.urlopen(req) as response, open(zip_path, 'wb') as out_file:
        shutil.copyfileobj(response, out_file)
    print("Extracting Vulkan headers...")
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(VENDOR_DIR)
    old_name = os.path.join(VENDOR_DIR, f"Vulkan-Headers-{VK_VERSION}")
    new_name = os.path.join(VENDOR_DIR, "Vulkan-Headers")
    if os.path.exists(new_name):
        shutil.rmtree(new_name)
    if os.path.exists(old_name):
        os.rename(old_name, new_name)

    os.remove(zip_path)
    print("Vulkan headers ready at .vendor/Vulkan-Headers")

if __name__ == "__main__":
    os.makedirs(VENDOR_DIR, exist_ok=True)
    download_sdl()
    download_volk()
    download_vulkan_headers()