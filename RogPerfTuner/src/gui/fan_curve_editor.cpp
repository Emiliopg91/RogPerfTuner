#include "gui/fan_curve_editor.hpp"

#include <QPoint>
#include <QtCharts>
#include <unordered_map>

#include "framework/utils/enum_utils.hpp"
#include "framework/utils/string_utils.hpp"
#include "models/performance/performance_profile.hpp"
#include "services/performance_service.hpp"
#include "utils/constants.hpp"

CurveEditor::CurveEditor(const std::string& profile, QWidget* parent) : QDialog(parent), profile(StringUtils::toUpperCase(profile)) {
	setWindowModality(Qt::WindowModal);

	setWindowTitle(translator.translate("edit.fan.profile").c_str());
	setFixedSize(550, 550);
	setWindowIcon(QIcon(Constants::ASSET_ICON_45_FILE.c_str()));
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout* layout = new QVBoxLayout(this);
	setLayout(layout);

	fans = performanceService.getFans();
	std::sort(fans.begin(), fans.end());

	profTabs = new QTabWidget(this);
	layout->addWidget(profTabs);
	auto i = 0;
	for (auto prof : values<PerformanceProfile>()) {
		if (prof == PerformanceProfile::SMART) {
			continue;
		}
		QWidget* tabPage	   = new QWidget();
		QVBoxLayout* tabLayout = new QVBoxLayout(tabPage);

		QTabWidget* fanTabs = new QTabWidget(this);
		tabLayout->addWidget(fanTabs);

		seriesList[toString(prof)] = {};
		charts[toString(prof)]	   = {};
		for (const auto& fan : fans) {
			QLineSeries* s = new QLineSeries();
			s->setPointsVisible(true);
			s->setMarkerSize(5.0);

			const auto& curve = performanceService.getFanCurve(fan, toString(prof));
			for (size_t i = 0; i < curve.perc.size(); i++) {
				*s << QPointF(curve.temp[i], curve.perc[i]);
			}

			FanCurveView* chartView = new FanCurveView(s, this);
			chartView->setRenderHint(QPainter::Antialiasing);
			chartView->chart()->addSeries(s);

			auto axisX = new QValueAxis();
			axisX->setRange(0, 100);
			axisX->setLabelFormat("%dc");
			chartView->chart()->addAxis(axisX, Qt::AlignBottom);
			s->attachAxis(axisX);

			auto axisY = new QValueAxis();
			axisY->setRange(0, 100);
			axisY->setLabelFormat("%d%%");
			chartView->chart()->addAxis(axisY, Qt::AlignLeft);
			s->attachAxis(axisY);

			QWidget* tabPage	   = new QWidget();
			QVBoxLayout* tabLayout = new QVBoxLayout(tabPage);
			tabLayout->addWidget(chartView);

			fanTabs->addTab(tabPage, fan.c_str());

			seriesList[toString(prof)].push_back(s);
			charts[toString(prof)].push_back(chartView);
		}

		profTabs->addTab(tabPage, (translator.translate("label.profile." + toName(prof)).c_str()));
		if (this->profile == toString(prof)) {
			profTabs->setCurrentIndex(i);
		}
		i++;
	}

	QPushButton* saveBtn = new QPushButton(translator.translate("save").c_str());
	saveBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QPushButton* resetBtn = new QPushButton(translator.translate("reset").c_str());
	resetBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QPushButton* defaultBtn = new QPushButton(translator.translate("reset.default").c_str());
	defaultBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	QWidget* buttonGroup = new QWidget();
	QHBoxLayout* hLayout = new QHBoxLayout(buttonGroup);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(5);
	hLayout->addWidget(resetBtn);
	hLayout->addWidget(defaultBtn);

	layout->addWidget(buttonGroup);

	buttonGroup = new QWidget();
	hLayout		= new QHBoxLayout(buttonGroup);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(5);

	hLayout->addWidget(saveBtn);
	layout->addWidget(buttonGroup);

	connect(saveBtn, &QPushButton::clicked, this, [this]() {
		for (const auto& [profile, seriesEntry] : seriesList) {
			std::unordered_map<std::string, FanCurveData> curves;
			for (size_t idx = 0; idx < fans.size(); idx++) {
				QLineSeries* series = seriesEntry[idx];
				std::string fan		= fans[idx];

				FanCurveData curve;
				for (int i = 0; i < series->count(); ++i) {
					QPointF p = series->at(i);
					curve.temp.emplace_back(static_cast<int>(p.x()));
					curve.perc.emplace_back(static_cast<int>(p.y()));
				}

				curves[fan] = curve;
			}

			performanceService.saveFanCurves(profile, curves);
		}
		performanceService.restoreFanCurves();
	});

	connect(resetBtn, &QPushButton::clicked, this, [this]() {
		for (const auto& [profile, seriesEntry] : seriesList) {
			for (size_t idx = 0; idx < fans.size(); idx++) {
				QLineSeries* series = seriesEntry[idx];
				std::string fan		= fans[idx];
				FanCurveView* chart = charts[profile][idx];

				auto curve = performanceService.getFanCurve(fan, profile);

				for (int i = 0; i < series->count(); ++i) {
					qreal x = curve.temp[i];
					qreal y = static_cast<int>(curve.perc[i]);
					series->replace(i, QPointF(x, y));
				}

				chart->update();
			}
		}
	});

	connect(defaultBtn, &QPushButton::clicked, this, [this]() {
		for (const auto& [profile, seriesEntry] : seriesList) {
			for (size_t idx = 0; idx < fans.size(); idx++) {
				QLineSeries* series = seriesEntry[idx];
				std::string fan		= fans[idx];
				FanCurveView* chart = charts[profile][idx];
				auto curve			= performanceService.getDefaultFanCurve(fan, profile);

				for (int i = 0; i < series->count(); ++i) {
					qreal x = curve.temp[i];
					qreal y = static_cast<int>(curve.perc[i]);
					series->replace(i, QPointF(x, y));
				}

				chart->update();
			}
		}
	});
}