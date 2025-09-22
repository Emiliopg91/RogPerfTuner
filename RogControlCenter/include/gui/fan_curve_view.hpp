#pragma once

#include <QtCharts>

class FanCurveView : public QChartView {
	Q_OBJECT
  public:
	explicit FanCurveView(QLineSeries* series, QWidget* parent = nullptr);

	int selectedIndex = -1;
	QLineSeries* series;
	QPointF hoverPos;
	bool hasHover = false;

  protected:
  protected:
	void paintEvent(QPaintEvent* event) override;

	void showEvent(QShowEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;

	void mouseReleaseEvent(QMouseEvent*) override;

	int findClosestPoint(const QPointF& pos);
};
