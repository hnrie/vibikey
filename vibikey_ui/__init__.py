"""VibiKey - Vietnamese Input Method UI"""

try:
    from .app import VibiKeyApp
except Exception:  # PySide6 / libGL may be unavailable in headless/test envs
    VibiKeyApp = None  # type: ignore

__all__ = ["VibiKeyApp"]
