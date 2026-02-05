#pragma once

#include <QComboBox>
#include <QWheelEvent>

class NoScrollComboBox : public QComboBox {
	Q_OBJECT
  public:
	explicit NoScrollComboBox(QWidget* parent = nullptr);

  protected:
	void wheelEvent(QWheelEvent* event);
};
