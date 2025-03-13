from PyQt5.QtWidgets import QApplication, QDialog, QMainWindow, QComboBox
from PyQt5.QtGui import QCursor, QWheelEvent


class GuiUtils:
    """Gui utility class"""

    @staticmethod
    def center_window_on_current_screen(element: QDialog | QMainWindow):
        """Center window"""
        cursor_position = QCursor.pos()
        screen = QApplication.screenAt(cursor_position)
        if screen is not None:
            screen_geometry = screen.availableGeometry()
            x = int((screen_geometry.width() - element.width()) / 2) + screen_geometry.x()
            y = int((screen_geometry.height() - element.height()) / 2) + screen_geometry.y()
            element.move(x, y)


class NoScrollComboBox(QComboBox):
    """Implementation without wheel event"""

    def wheelEvent(self, event: QWheelEvent):  # pylint: disable=invalid-name
        """Ignore wheel event"""
        event.ignore()
