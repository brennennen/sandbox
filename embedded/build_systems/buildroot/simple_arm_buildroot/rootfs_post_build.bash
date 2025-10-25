#!/usr/bin/env bash

# Environment variables:
# HOST_DIR, STAGING_DIR, TARGET_DIR, BUILD_DIR, BINARIES_DIR, BASE_DIR

# Breaks bit-for-bit build reproducibility, use a git tag/version id "x.y.z" for a real product.
echo $(date) > "${TARGET_DIR}/etc/build-date"
