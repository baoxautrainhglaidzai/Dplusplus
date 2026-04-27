"""Error types for the D++ language toolchain."""


class DppError(Exception):
    """Base class for D++ errors."""


class LexerError(DppError):
    """Raised when tokenization fails."""


class ParserError(DppError):
    """Raised when parsing fails."""


class DppRuntimeError(DppError):
    """Raised when program execution fails."""
