#!/usr/bin/env python3

"""
A script to scan through all build artifacts and generate an artifact summary
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
    # ex: artifacts/debug/debug-builds-linux/x86_64/debug/bin/sample_project
    parts = path.parts
    # find debug/release
    if "debug" in parts:
        build_type = "Debug"
    elif "release" in parts:
        build_type = "Release"
    else:
        build_type = "Unknown"

    for part in parts:
        if part.startswith("debug-builds-") or part.startswith("release-builds-"):
            os = part.split("-")[-1]
            break
    else:
        os = "Unknown"

    arch_candidates = [
        "x86_64",
        "amd64",
        "arm",
        "arm64",
    ]
    arch = next((p for p in parts if p in arch_candidates), "Unknown")

    return os, arch, build_type


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
