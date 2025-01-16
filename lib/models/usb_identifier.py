class UsbIdentifier:
    """Class for usb device id storage"""

    def __init__(self, id_vendor: str, id_product: str, name: str = None):
        self.id_vendor = id_vendor
        self.id_product = id_product
        self.name = name
