"""CLI entrypoint for the D++ interpreter."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

from .api import run_source
from .errors import DppError


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Run D++ source files.")
    parser.add_argument("path", type=Path, help="Path to a .dpp source file")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    try:
        source = args.path.read_text(encoding="utf-8")
        run_source(source)
    except FileNotFoundError:
        print(f"Error: File not found: {args.path}", file=sys.stderr)
        return 1
    except DppError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    return 0
