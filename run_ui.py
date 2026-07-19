#!/usr/bin/env python3
"""
VibiKey - Vietnamese Input Method
Entry point
"""

import sys
import os

# Ensure parent directory is on path so vibikey_ui can be imported
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from vibikey_ui.app import VibiKeyApp


def main():
    app = VibiKeyApp()
    sys.exit(app.run())


if __name__ == "__main__":
    main()
