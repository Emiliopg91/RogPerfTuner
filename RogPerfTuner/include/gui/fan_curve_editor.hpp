#pragma once

#include <QDialog>
#include <QtCharts>
#include <QtWidgets>
#include <unordered_map>

#include "framework/translator/translator.hpp"
#include "gui/fan_curve_view.hpp"
#include "services/performance_service.hpp"

class CurveEditor : public QDialog {
	Q_OBJECT
  public:
	CurveEditor(const std::string& profile, QWidget* parent = nullptr);

  private:
	QTabWidget* profTabs;
	std::unordered_map<std::string, std::vector<QLineSeries*>> seriesList;
	std::unordered_map<std::string, std::vector<FanCurveView*>> charts;
	std::vector<std::string> fans;
	std::string profile;

	Translator& translator				   = Translator::getInstance();
	PerformanceService& performanceService = PerformanceService::getInstance();
};
