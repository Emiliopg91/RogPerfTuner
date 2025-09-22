#pragma once

#include <QDialog>
#include <QtCharts>
#include <QtWidgets>

#include "../services/performance_service.hpp"
#include "../translator/translator.hpp"
#include "fan_curve_view.hpp"

class CurveEditor : public QDialog {
	Q_OBJECT
  public:
	CurveEditor(const std::string& fan, const std::string& profile, QWidget* parent = nullptr);

  private:
	FanCurveView* chart;
	QLineSeries* series;
	std::string fan;
	std::string profile;

	Translator& translator				   = Translator::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();

	int findClosestPoint(const QPointF& pos);
};
