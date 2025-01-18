# pylint: disable=E0611,E0401
from openrgb.utils import RGBColor


class OpenRGBUtils:
    """OpenRGB Utils"""

    @staticmethod
    def dim(color: RGBColor, factor: float) -> RGBColor:
        """Get dimmed color"""
        return RGBColor(
            red=round(color.red * factor),
            green=round(color.green * factor),
            blue=round(color.blue * factor),
        )

    @staticmethod
    def to_hex(color: RGBColor) -> str:
        """Get hex representation of color"""
        return f"#{color.red:02x}{color.green:02x}{color.blue:02x}".upper()

    @staticmethod
    def from_hsv(h, s, v) -> RGBColor:
        """Calculate RGBColor from hsv"""
        # Normalize hue
        h = 0 if h is None or not isinstance(h, (int, float)) else h % 360
        # Clamp saturation
        s = 0 if s is None or not isinstance(s, (int, float)) else max(0, min(s, 1))
        # Clamp value
        v = 0 if v is None or not isinstance(v, (int, float)) else max(0, min(v, 1))

        r, g, b = 0, 0, 0

        if s != 0:
            h /= 60  # Sector of the circle
            i = int(h)  # Integer part of h
            f = h - i  # Fractional part of h
            p = v * (1 - s)
            q = v * (1 - s * f)
            t = v * (1 - s * (1 - f))

            if i % 6 == 0:
                r, g, b = v, t, p
            elif i % 6 == 1:
                r, g, b = q, v, p
            elif i % 6 == 2:
                r, g, b = p, v, t
            elif i % 6 == 3:
                r, g, b = p, q, v
            elif i % 6 == 4:
                r, g, b = t, p, v
            elif i % 6 == 5:
                r, g, b = v, p, q
        else:
            r = g = b = v  # Shades of gray

        # Convert to 0-255 range
        return RGBColor(r * 255, g * 255, b * 255)
