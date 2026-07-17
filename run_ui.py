#!/usr/bin/env python3
"""
CatKey - Vietnamese Input Method
Entry point
"""

import sys
import os

# Ensure parent directory is on path so catkey_ui can be imported
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from catkey_ui.app import CatKeyApp


def main():
    app = CatKeyApp()
    sys.exit(app.run())


if __name__ == "__main__":
    main()
