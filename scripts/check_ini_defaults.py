#!/usr/bin/env python3
"""Check that the plugin's default settings agree in all three places.

Enforces AGENTS.md rule R-7.3. A default lives in three places and all of
them must say the same thing:

  1. the hardcoded initialisers at the top of Settings::Load()
  2. the fallback argument of the matching ini.GetBoolValue(...) call
  3. every shipped Unisexy.ini template

On 2026-08-18 these drifted -- the template said EyesMale=true while the code
defaulted to false -- and because MO2 writes a fresh ini from the code
defaults when the mod folder has none, the template had no effect and no CVEO
eye was converted. This script exists so that cannot happen silently again.

Exit code 0 when everything agrees, 1 otherwise. Read-only.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SETTINGS = ROOT / "src" / "Settings.cpp"
TEMPLATES = [
    ROOT / "Data" / "SKSE" / "Plugins" / "Unisexy.ini",
    ROOT / "Skyrim" / "Data" / "SKSE" / "plugins" / "Unisexy.ini",
]

# ini key -> (C++ HeadPartType enum, which field of GenderSettings)
ENABLED_KEYS = {
    "HairMale": ("kHair", "male"),
    "HairFemale": ("kHair", "female"),
    "ScarsMale": ("kScar", "male"),
    "ScarsFemale": ("kScar", "female"),
    "EyesMale": ("kEyes", "male"),
    "EyesFemale": ("kEyes", "female"),
    "BrowsMale": ("kEyebrows", "male"),
    "BrowsFemale": ("kEyebrows", "female"),
    "FacialHairMale": ("kFacialHair", "male"),
    "FacialHairFemale": ("kFacialHair", "female"),
}

# ini key -> C++ HeadPartType enum, for the [ShowOnlyUnisexy] section
SHOWONLY_KEYS = {
    "ShowOnlyUnisexyHair": "kHair",
    "ShowOnlyUnisexyEyes": "kEyes",
    "ShowOnlyUnisexyScars": "kScar",
    "ShowOnlyUnisexyBrows": "kEyebrows",
    "ShowOnlyUnisexyFacialHair": "kFacialHair",
}

# ini key -> C++ member, for plain [Debug] booleans
SCALAR_KEYS = {
    "VerboseLogging": "_verboseLogging",
    "ShowOnlyUnisexy": "_showOnlyUnisexy",
}

errors: list[str] = []


def fail(msg: str) -> None:
    errors.append(msg)


def parse_bool(text: str) -> bool | None:
    t = text.strip().lower()
    return True if t == "true" else False if t == "false" else None


def read_hardcoded(src: str) -> dict[str, bool]:
    """Defaults assigned before the ini file is read."""
    out: dict[str, bool] = {}

    # _enabledTypes[...::kHair] = { true, true };
    pat = re.compile(
        r"_enabledTypes\[\s*RE::BGSHeadPart::HeadPartType::(\w+)\s*\]\s*=\s*\{\s*"
        r"(true|false)\s*,\s*(true|false)\s*\}"
    )
    for enum, male, female in pat.findall(src):
        out[f"enabled:{enum}:male"] = male == "true"
        out[f"enabled:{enum}:female"] = female == "true"

    # _showOnlyUnisexyTypes[...::kHair] = false;
    pat = re.compile(
        r"_showOnlyUnisexyTypes\[\s*RE::BGSHeadPart::HeadPartType::(\w+)\s*\]\s*=\s*(true|false)\s*;"
    )
    for enum, val in pat.findall(src):
        out[f"showonly:{enum}"] = val == "true"

    # _verboseLogging = false;  /  _showOnlyUnisexy = false;
    for member in SCALAR_KEYS.values():
        m = re.search(rf"{re.escape(member)}\s*=\s*(true|false)\s*;", src)
        if m:
            out[f"scalar:{member}"] = m.group(1) == "true"
    return out


def read_fallbacks(src: str) -> dict[str, bool]:
    """Third argument of each ini.GetBoolValue(section, key, <fallback>, ...)."""
    out: dict[str, bool] = {}
    pat = re.compile(
        r'GetBoolValue\(\s*[^,]+,\s*"([A-Za-z]+)"\s*,\s*(true|false)\s*,'
    )
    for key, val in pat.findall(src):
        out[key] = val == "true"
    return out


def read_template(path: Path) -> dict[str, bool]:
    """Key/value pairs from an ini template, ignoring comments and sections."""
    out: dict[str, bool] = {}
    for lineno, raw in enumerate(path.read_text(encoding="utf-8-sig").splitlines(), 1):
        line = raw.strip()
        if not line or line.startswith((";", "#", "[")):
            continue
        if "=" not in line:
            continue
        key, _, value = line.partition("=")
        parsed = parse_bool(value.split(";")[0])
        if parsed is None:
            fail(f"{path.relative_to(ROOT)}:{lineno}: value is not true/false: {line}")
            continue
        out[key.strip()] = parsed
    return out


def main() -> int:
    if not SETTINGS.is_file():
        print(f"error: {SETTINGS} not found", file=sys.stderr)
        return 1
    src = SETTINGS.read_text(encoding="utf-8")

    hardcoded = read_hardcoded(src)
    fallbacks = read_fallbacks(src)
    templates = {}
    for path in TEMPLATES:
        if not path.is_file():
            fail(f"missing ini template: {path.relative_to(ROOT)}")
            continue
        templates[path] = read_template(path)

    # Build the expected table from the hardcoded initialisers, then check the
    # other two sources against it.
    expected: dict[str, bool] = {}
    for key, (enum, field) in ENABLED_KEYS.items():
        slot = f"enabled:{enum}:{field}"
        if slot not in hardcoded:
            fail(f"Settings.cpp: no hardcoded default found for {enum} ({key})")
            continue
        expected[key] = hardcoded[slot]
    for key, enum in SHOWONLY_KEYS.items():
        slot = f"showonly:{enum}"
        if slot not in hardcoded:
            fail(f"Settings.cpp: no hardcoded _showOnlyUnisexyTypes default for {enum} ({key})")
            continue
        expected[key] = hardcoded[slot]
    for key, member in SCALAR_KEYS.items():
        slot = f"scalar:{member}"
        if slot not in hardcoded:
            fail(f"Settings.cpp: no hardcoded default found for {member} ({key})")
            continue
        expected[key] = hardcoded[slot]

    for key, want in sorted(expected.items()):
        got = fallbacks.get(key)
        if got is None:
            fail(f"Settings.cpp: no GetBoolValue fallback found for key {key!r}")
        elif got != want:
            fail(
                f"{key}: hardcoded default is {str(want).lower()} but the "
                f"GetBoolValue fallback in Settings.cpp is {str(got).lower()}"
            )

        for path, values in templates.items():
            rel = path.relative_to(ROOT)
            if key not in values:
                fail(f"{rel}: key {key!r} is missing")
            elif values[key] != want:
                fail(
                    f"{key}: hardcoded default is {str(want).lower()} but "
                    f"{rel} says {str(values[key]).lower()}"
                )

    # An ini template must not carry keys the code never reads.
    for path, values in templates.items():
        for key in sorted(set(values) - set(expected)):
            fail(f"{path.relative_to(ROOT)}: key {key!r} is not read by Settings.cpp")

    if errors:
        print("Settings defaults disagree (AGENTS.md R-7.3):\n", file=sys.stderr)
        for e in errors:
            print(f"  - {e}", file=sys.stderr)
        print(
            "\nA default lives in three places and all must match:\n"
            "  1. the initialisers at the top of Settings::Load()\n"
            "  2. the third argument of the matching ini.GetBoolValue(...)\n"
            "  3. every shipped Unisexy.ini template\n"
            "See AGENTS.md section 4 for why.",
            file=sys.stderr,
        )
        return 1

    print(f"settings defaults agree across {len(templates) + 2} sources "
          f"({len(expected)} keys checked)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
