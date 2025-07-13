#!/usr/bin/env python3

"""
A script to scan through all build artifacts(packages) and generate an artifact summary
table for GitHub Actions step summary.
"""

import sys
import hashlib
import os
from pathlib import Path


def sha256sum(file_path: Path) -> str:
    h = hashlib.sha256()
    with file_path.open("rb") as f:
        while chunk := f.read(8192):
            h.update(chunk)
    return h.hexdigest()


def format_size(size: float) -> str:
    for unit in ["B", "KB", "MB", "GB"]:
        if size < 1024:
            return f"{size:.1f} {unit}"
        size /= 1024
    return f"{size:.1f} TB"


def parse_info(path: Path):
    # ex: ./artifacts/femtolog-1.0.0-release_linux_x86_64.deb
    build_type = "Unknown"
    os_name = "Unknown"
    arch_name = "Unknown"

    filename = os.path.basename(path)

    build_type_candidates = [
        "debug",
        "release",
    ]
    for b in build_type_candidates:
        if b in filename:
            build_type = b
            break

    os_candidates = [
        "linux",
        "windows",
        "darwin",
    ]
    for o in os_candidates:
        if o in filename:
            os_name = o
            break

    arch_candidates = [
        "x86_64",
        "amd64",
        "arm",
        "arm64",
    ]
    for a in arch_candidates:
        if a in filename:
            arch_name = a
            break

    return os_name, arch_name, build_type


def main(artifact_dir: Path):
    print("# Build Artifacts Summary\n")
    print("| OS | Arch | Build Type | File | Size | SHA-256 |")
    print("|:--:|:----:|:----------:|:----:|:----:|:-------:|")

    for path in sorted(artifact_dir.rglob("*")):
        if not path.is_file():
            continue
        platform, arch, build_type = parse_info(path)
        basename = os.path.basename(path)
        size = format_size(path.stat().st_size)
        digest = sha256sum(path)
        print(
            f"| {platform} | {arch} | {build_type} | `{basename}` | {size} | `{digest[:10]}` |"
        )


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: summarize.py <artifact_dir>", file=sys.stderr)
        sys.exit(1)
    main(Path(sys.argv[1]))
