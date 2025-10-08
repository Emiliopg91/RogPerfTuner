#include "../../include/gui/fan_curve_editor.hpp"

#include <QPoint>
#include <QtCharts>
#include <unordered_map>

#include "../../include/utils/constants.hpp"
#include "../../include/utils/string_utils.hpp"

CurveEditor::CurveEditor(const std::string& profile, QWidget* parent) : QDialog(parent), profile(profile) {
	setWindowModality(Qt::WindowModal);

	setWindowTitle(QString::fromStdString(translator.translate(
		"edit.fan.profile", {{"prof", StringUtils::toLowerCase(translator.translate("label.profile." + StringUtils::toUpperCase(profile)))}})));
	setFixedSize(550, 550);
	setWindowIcon(QIcon(QString::fromStdString(Constants::ICON_45_FILE)));
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout* layout = new QVBoxLayout(this);
	setLayout(layout);

	// Creamos el TabWidget
	tabs = new QTabWidget(this);
	layout->addWidget(tabs);

	fans = performanceService.getFans();
	std::sort(fans.begin(), fans.end());

	for (const auto& fan : fans) {
		QLineSeries* s = new QLineSeries();
		s->setPointsVisible(true);
		s->setMarkerSize(5.0);

		const auto& curve = performanceService.getFanCurve(fan, profile);
		for (size_t i = 0; i < curve.perc.size(); i++) {
			*s << QPointF(curve.temp[i], curve.perc[i]);
		}

		FanCurveView* chartView = new FanCurveView(s, this);
		chartView->setRenderHint(QPainter::Antialiasing);
		chartView->chart()->addSeries(s);

		auto axisX = new QValueAxis();
		axisX->setRange(20, 100);
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

		tabs->addTab(tabPage, QString::fromStdString(fan));

		seriesList.push_back(s);
		charts.push_back(chartView);
	}
	QPushButton* saveBtn	   = new QPushButton(QString::fromStdString(translator.translate("save")));
	QPushButton* resetBtn	   = new QPushButton(QString::fromStdString(translator.translate("reset")));
	QPushButton* defaultBtn	   = new QPushButton(QString::fromStdString(translator.translate("reset.default")));
	QPushButton* suggestionBtn = new QPushButton(QString::fromStdString(translator.translate("apply.suggestion")));

	QWidget* buttonGroup = new QWidget();
	QHBoxLayout* hLayout = new QHBoxLayout(buttonGroup);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(5);
	hLayout->addWidget(saveBtn);
	hLayout->addWidget(resetBtn);

	saveBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	resetBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	layout->addWidget(buttonGroup);

	buttonGroup = new QWidget();
	hLayout		= new QHBoxLayout(buttonGroup);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(5);

	hLayout->addWidget(suggestionBtn);
	hLayout->addWidget(defaultBtn);
	defaultBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	suggestionBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	layout->addWidget(buttonGroup);

	connect(saveBtn, &QPushButton::clicked, this, [this]() {
		std::unordered_map<std::string, FanCurveData> curves;
		for (size_t idx = 0; idx < fans.size(); idx++) {
			QLineSeries* series = seriesList[idx];
			std::string fan		= fans[idx];

			FanCurveData curve;
			for (int i = 0; i < series->count(); ++i) {
				QPointF p = series->at(i);
				curve.temp.emplace_back(static_cast<int>(p.x()));
				curve.perc.emplace_back(static_cast<int>(p.y()));
			}

			curves[fan] = curve;
		}

		performanceService.saveFanCurves(this->profile, curves);
	});

	connect(resetBtn, &QPushButton::clicked, this, [this]() {
		for (size_t idx = 0; idx < fans.size(); idx++) {
			QLineSeries* series = seriesList[idx];
			std::string fan		= fans[idx];
			FanCurveView* chart = charts[idx];

			auto curve = performanceService.getFanCurve(fan, this->profile);

			for (int i = 0; i < series->count(); ++i) {
				qreal x = curve.temp[i];
				qreal y = static_cast<int>(curve.perc[i]);
				series->replace(i, QPointF(x, y));
			}

			chart->update();
		}
	});

	connect(defaultBtn, &QPushButton::clicked, this, [this]() {
		for (size_t idx = 0; idx < fans.size(); idx++) {
			QLineSeries* series = seriesList[idx];
			std::string fan		= fans[idx];
			FanCurveView* chart = charts[idx];
			auto curve			= performanceService.getDefaultFanCurve(fan, this->profile);

			for (int i = 0; i < series->count(); ++i) {
				qreal x = curve.temp[i];
				qreal y = static_cast<int>(curve.perc[i]);
				series->replace(i, QPointF(x, y));
			}

			chart->update();
		}
	});

	connect(suggestionBtn, &QPushButton::clicked, this, [this]() {
		for (size_t idx = 0; idx < fans.size(); idx++) {
			QLineSeries* series = seriesList[idx];
			std::string fan		= fans[idx];
			FanCurveView* chart = charts[idx];
			auto curve			= performanceService.getDefaultFanCurve(fan, this->profile);

			for (int i = 0; i < series->count(); ++i) {
				qreal x = curve.temp[i];
				qreal y = std::min(static_cast<int>(curve.perc[i] * 1.2), 100);
				series->replace(i, QPointF(x, y));
			}

			chart->update();
		}
	});
}