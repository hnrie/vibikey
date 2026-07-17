"""
CatKey - interface translation via gettext.

Uses standard .po/.mo catalogs in locales/<lang>/LC_MESSAGES/catkey.mo
(editable with Poedit). English is the source language: msgid strings are the
English text, so English needs no catalog (msgid is returned as-is).

Usage:
    from .i18n import set_language, _
    set_language("vi")
    label.setText(_("Apply"))
"""

import gettext
from pathlib import Path

LANG_EN = "en"
LANG_VI = "vi"

_LOCALE_DIR = Path(__file__).resolve().parent.parent / "locales"
_DOMAIN = "catkey"

_current = LANG_EN
_translation = gettext.NullTranslations()


def set_language(lang: str) -> None:
    """Load the catalog for `lang`. Falls back to source (English) msgids."""
    global _current, _translation
    _current = lang if lang in (LANG_EN, LANG_VI) else LANG_EN
    if _current == LANG_EN:
        _translation = gettext.NullTranslations()
        return
    try:
        _translation = gettext.translation(
            _DOMAIN, localedir=str(_LOCALE_DIR), languages=[_current],
        )
    except (OSError, FileNotFoundError):
        _translation = gettext.NullTranslations()


def current_language() -> str:
    return _current


def _(message: str) -> str:
    """Translate `message` (an English source string / msgid)."""
    return _translation.gettext(message)
