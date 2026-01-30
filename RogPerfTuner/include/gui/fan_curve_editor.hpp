#pragma once

#include <QDialog>
#include <QtCharts>
#include <QtWidgets>

#include "gui/fan_curve_view.hpp"
#include "services/performance_service.hpp"
#include "translator/translator.hpp"

class CurveEditor : public QDialog {
	Q_OBJECT
  public:
	CurveEditor(const std::string& profile, QWidget* parent = nullptr);

  private:
	QTabWidget* tabs;
	std::vector<QLineSeries*> seriesList;
	std::vector<FanCurveView*> charts;
	std::vector<std::string> fans;
	std::string profile;

	Translator& translator				   = Translator::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
};
