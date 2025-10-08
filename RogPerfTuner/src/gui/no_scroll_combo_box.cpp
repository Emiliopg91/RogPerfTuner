#include "../../include/gui/no_scroll_combo_box.hpp"

NoScrollComboBox::NoScrollComboBox(QWidget* parent) : QComboBox(parent) {
}
void NoScrollComboBox::wheelEvent(QWheelEvent* event) {
	event->ignore();
}