#pragma once

#include <QApplication>
#include <QComboBox>
#include <QCursor>
#include <QDialog>
#include <QMainWindow>
#include <QScreen>
#include <QWheelEvent>

class GuiUtils {
  public:
	template <typename T>
	static void centerWindowOnCurrentScreen(T* element) {
		// Obtener la posición actual del cursor
		QPoint cursorPos = QCursor::pos();

		// Obtener la pantalla donde está el cursor
		QScreen* screen = QApplication::screenAt(cursorPos);
		if (screen) {
			QRect screenGeometry = screen->availableGeometry();
			int x				 = (screenGeometry.width() - element->width()) / 2 + screenGeometry.x();
			int y				 = (screenGeometry.height() - element->height()) / 2 + screenGeometry.y();
			element->move(x, y);
		}
	}
};

// Subclase de QComboBox que ignora el evento de rueda
class NoScrollComboBox : public QComboBox {
	Q_OBJECT
  public:
	explicit NoScrollComboBox(QWidget* parent = nullptr) : QComboBox(parent) {
	}

  protected:
	void wheelEvent(QWheelEvent* event) override {
		event->ignore();  // Ignorar el evento de rueda
	}
};
