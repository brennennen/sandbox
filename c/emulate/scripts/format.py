#!/usr/bin/env python
##
# Runs clang-format
#

import os
import sys
import subprocess
import shutil

this_script_dir = os.path.dirname(os.path.abspath(__file__))
source_dir = os.path.abspath(os.path.join(this_script_dir, ".."))
extensions = (".c", ".h")

print(f"Formatting '{source_dir}'...")
files_formatted = 0
for root, dirs, files in os.walk(source_dir):
    for file in files:
        if file.endswith(extensions):
            file_path = os.path.join(root, file)
            cmd = ["clang-format", "-i", "-style=file", file_path]
            try:
                subprocess.run(cmd, check=True)
                files_formatted += 1
            except subprocess.CalledProcessError as e:
                print(f"Failed to format {file_path}: {e}")
print(f"Formatted {files_formatted} files.")
