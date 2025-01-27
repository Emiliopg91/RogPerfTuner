from rcc.communications.client.tcp.openrgb.client.utils import RGBColor


transition = RGBColor(0, 0, 0).transition_to(RGBColor(255, 255, 255), 10)
print(f"{len(transition)} - {transition}")
