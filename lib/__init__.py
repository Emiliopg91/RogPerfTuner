__version__ = "2.0.3"
__app_name__ = "RogControlCenter"


def is_newer(version):
    """Compare current version with parameter"""

    def parse_version(version):
        return tuple(map(int, version.split(".")))

    v1 = parse_version(__version__)
    v2 = parse_version(version)

    return v1 < v2
