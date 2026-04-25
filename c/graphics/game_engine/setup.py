#!/usr/bin/env python
import os
import urllib.request
import zipfile
import shutil
import tarfile
import platform

VENDOR_DIR = ".vendor"

def download_sdl():
    sdl_ver = "3.4.0"
    sdl_path = os.path.join(VENDOR_DIR, f"SDL3-{sdl_ver}")
    if os.path.exists(sdl_path):
        return
    system = platform.system().lower()

    if system == "windows":
        url = f"https://github.com/libsdl-org/SDL/releases/download/release-{sdl_ver}/SDL3-devel-{SDL_VER}-mingw.zip"
        ext = ".zip"
    else:
        url = f"https://github.com/libsdl-org/SDL/releases/download/release-{sdl_ver}/SDL3-{SDL_VER}.tar.gz"
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
    if os.path.exists(volk_dir):
        return
    volk_h = "https://raw.githubusercontent.com/zeux/volk/master/volk.h"
    volk_c = "https://raw.githubusercontent.com/zeux/volk/master/volk.c"
    os.makedirs(volk_dir, exist_ok=True)
    print("Downloading volk...")
    urllib.request.urlretrieve(volk_h, os.path.join(volk_dir, "volk.h"))
    urllib.request.urlretrieve(volk_c, os.path.join(volk_dir, "volk.c"))

def download_vulkan_headers():
    vk_version = "1.4.341"
    vk_headers_url = f"https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v{vk_version}.zip"
    vulkan_headers_dir = os.path.join(VENDOR_DIR, "Vulkan-Headers")
    if os.path.exists(vulkan_headers_dir):
        return
    print("Downloading Vulkan headers...")
    zip_path = os.path.join(VENDOR_DIR, "vk_headers.zip")
    req = urllib.request.Request(vk_headers_url, headers={'User-Agent': 'Mozilla/5.0'})
    with urllib.request.urlopen(req) as response, open(zip_path, 'wb') as out_file:
        shutil.copyfileobj(response, out_file)
    print("Extracting Vulkan headers...")
    with zipfile.ZipFile(zip_path, 'r') as zip_ref:
        zip_ref.extractall(VENDOR_DIR)
    old_name = os.path.join(VENDOR_DIR, f"Vulkan-Headers-{vk_version}")
    if os.path.exists(old_name):
        os.rename(old_name, vulkan_headers_dir)
    os.remove(zip_path)
    print("Vulkan headers ready at .vendor/Vulkan-Headers")

def download_stb_image():
    # todo: wget https://github.com/nothings/stb/tree/master
    stb_dir = os.path.join(VENDOR_DIR, "stb")
    if os.path.exists(stb_dir):
        return
    stb_image = "https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h"
    os.makedirs(stb_dir, exist_ok=True)
    print("Downloading stb...")
    urllib.request.urlretrieve(stb_image, os.path.join(stb_dir, "stb_image.h"))

def download_cgltf():
    cgltf_url = "https://raw.githubusercontent.com/jkuhlmann/cgltf/refs/heads/master/cgltf.h"
    print("Downloading cgltf...")
    urllib.request.urlretrieve(cgltf_url, os.path.join(VENDOR_DIR, "cgltf.h"))

def download_bc7enc():
    bc7enc_dir = os.path.join(VENDOR_DIR, "bc7enc")
    if os.path.exists(bc7enc_dir):
        return
    os.makedirs(bc7enc_dir, exist_ok=True)
    bc7enc_c_url = "https://raw.githubusercontent.com/richgel999/bc7enc/refs/heads/master/bc7enc.c"
    bc7enc_h_url = "https://raw.githubusercontent.com/richgel999/bc7enc/refs/heads/master/bc7enc.h"
    print("Downloading bc7enc...")
    urllib.request.urlretrieve(bc7enc_c_url, os.path.join(bc7enc_dir, "bc7enc.c"))
    urllib.request.urlretrieve(bc7enc_h_url, os.path.join(bc7enc_dir, "bc7enc.h"))

if __name__ == "__main__":
    os.makedirs(VENDOR_DIR, exist_ok=True)
    download_sdl()
    download_volk()
    download_vulkan_headers()
    download_stb_image()
    download_cgltf()
    download_bc7enc()
