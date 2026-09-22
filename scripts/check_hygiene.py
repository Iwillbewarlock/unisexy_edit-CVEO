#!/usr/bin/env python3
"""Reject files that must never be committed.

Enforces AGENTS.md rule R-7.1, in two passes over what git actually tracks.

  1. Anything tracked that .gitignore already covers. That only happens via
     `git add -f`, or because the file was committed before the ignore rule
     existed -- which is how build logs ended up in this repo until b23a2c2.
  2. Anything matching the patterns below, for the case .gitignore has not
     thought of yet: a backup folder, a stray .env, a database file.

Exit code 0 when the tree is clean, 1 otherwise. Read-only.
"""

from __future__ import annotations

import re
import subprocess
import sys

# (compiled pattern, what it matches, why it must not be committed)
RULES: list[tuple[re.Pattern[str], str, str]] = [
    (re.compile(r"\.(log|dmp|pdb)$", re.I), "log or debug output", "build/run output, not source"),
    (re.compile(r"(^|/)(build|cmake|configure)[_-]?\w*(log|output)\w*\.(txt|log)$", re.I),
     "build or configure log", "build output, not source"),
    (re.compile(r"\.(dll|lib|exe|obj|ilk|exp)$", re.I), "build artefact", "rebuildable from source"),
    (re.compile(r"(^|/)(build|buildae|buildvr|out|bin)/", re.I), "build directory", "rebuildable from source"),
    (re.compile(r"(^|/)vcpkg_installed/", re.I), "vcpkg install tree", "restored by the build"),
    (re.compile(r"(^|/)\.env($|\.)", re.I), "environment file", "may hold secrets"),
    (re.compile(r"\.(db|db-wal|db-shm|sqlite3?)$", re.I), "database file", "runtime data, not source"),
    (re.compile(r"backup[_-]?\d{6,}", re.I), "dated backup folder", "see AGENTS.md 6절: do not revive backup experiments"),
    (re.compile(r"(^|/)CMakeUserPresets\.json$"), "local CMake preset", "machine-specific, gitignored"),
    (re.compile(r"\.(zip|7z|rar)$", re.I), "archive", "binary blob, not source"),
]

# Paths that look like a hit but are legitimate.
ALLOW = (
    re.compile(r"^docs/"),
    re.compile(r"^\.github/"),
)


def main() -> int:
    try:
        out = subprocess.run(
            ["git", "ls-files", "-z"], capture_output=True, text=True, check=True
        ).stdout
    except (subprocess.CalledProcessError, FileNotFoundError) as exc:
        print(f"error: could not list tracked files: {exc}", file=sys.stderr)
        return 1

    tracked = [p for p in out.split("\0") if p]
    findings: list[str] = []

    # Pass 1: tracked although .gitignore covers it.
    try:
        ignored = subprocess.run(
            ["git", "ls-files", "-i", "-c", "--exclude-standard", "-z"],
            capture_output=True, text=True, check=True,
        ).stdout
    except subprocess.CalledProcessError as exc:
        print(f"error: could not list ignored-but-tracked files: {exc}", file=sys.stderr)
        return 1
    seen: set[str] = set()
    for path in (p for p in ignored.split("\0") if p):
        seen.add(path)
        findings.append(f"{path}\n      tracked although .gitignore covers it")

    # Pass 2: patterns .gitignore has not thought of.
    for path in tracked:
        if path in seen or any(a.search(path) for a in ALLOW):
            continue
        for pattern, kind, why in RULES:
            if pattern.search(path):
                findings.append(f"{path}\n      {kind} -- {why}")
                break

    if findings:
        print("Files that must not be committed (AGENTS.md R-7.1):\n", file=sys.stderr)
        for f in findings:
            print(f"  - {f}", file=sys.stderr)
        print(
            "\nRemove each with `git rm --cached <path>` and add a matching\n"
            "line to .gitignore, then commit.",
            file=sys.stderr,
        )
        return 1

    print(f"no forbidden files tracked ({len(tracked)} files checked)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
