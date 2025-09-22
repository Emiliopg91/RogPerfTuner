#include "../../include/gui/fan_curve_editor.hpp"

#include <QPoint>
#include <QtCharts>

#include "../../include/utils/constants.hpp"
#include "../../include/utils/string_utils.hpp"

CurveEditor::CurveEditor(const std::string& fan, const std::string& profile, QWidget* parent) : QDialog(parent), fan(fan), profile(profile) {
	setWindowModality(Qt::WindowModal);

	setWindowTitle(QString::fromStdString(translator.translate(
		"edit.fan.profile",
		{{"fan", fan}, {"prof", StringUtils::toLowerCase(translator.translate("label.profile." + StringUtils::toUpperCase(profile)))}})));
	setFixedSize(500, 400);
	setWindowIcon(QIcon(QString::fromStdString(Constants::ICON_45_FILE)));
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	QVBoxLayout* layout = new QVBoxLayout(this);
	setLayout(layout);

	series = new QLineSeries();
	series->setPointsVisible(true);
	series->setMarkerSize(5.0);

	const auto& curve = performanceService.getFanCurve(fan, profile);
	for (size_t i = 0; i < curve.perc.size(); i++) {
		*series << QPointF(curve.temp[i], curve.perc[i]);
	}

	chart = new FanCurveView(series, this);
	chart->setRenderHint(QPainter::Antialiasing);
	chart->chart()->addSeries(series);

	auto axisX = new QValueAxis();
	axisX->setRange(0, 100);
	axisX->setLabelFormat("%dc");
	chart->chart()->addAxis(axisX, Qt::AlignBottom);
	series->attachAxis(axisX);

	auto axisY = new QValueAxis();
	axisY->setRange(0, 100);
	axisY->setLabelFormat("%d%%");
	chart->chart()->addAxis(axisY, Qt::AlignLeft);
	series->attachAxis(axisY);

	layout->addWidget(chart);

	QPushButton* saveBtn	= new QPushButton(QString::fromStdString(translator.translate("save")));
	QPushButton* resetBtn	= new QPushButton(QString::fromStdString(translator.translate("reset")));
	QPushButton* defaultBtn = new QPushButton(QString::fromStdString(translator.translate("reset.default")));

	QWidget* buttonGroup = new QWidget();
	QHBoxLayout* hLayout = new QHBoxLayout(buttonGroup);
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(5);
	hLayout->addWidget(saveBtn);
	hLayout->addWidget(resetBtn);
	hLayout->addWidget(defaultBtn);

	saveBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	resetBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	defaultBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	layout->addWidget(buttonGroup);

	connect(saveBtn, &QPushButton::clicked, this, [this]() {
		FanCurveData curve;
		for (int i = 0; i < series->count(); ++i) {
			QPointF p = series->at(i);
			curve.temp.emplace_back(static_cast<int>(p.x()));
			curve.perc.emplace_back(static_cast<int>(p.y()));
		}
		performanceService.saveFanCurve(this->fan, this->profile, curve);
		this->close();
	});

	connect(resetBtn, &QPushButton::clicked, this, [this]() {
		auto curve = performanceService.getFanCurve(this->fan, this->profile);
		;

		for (int i = 0; i < series->count(); ++i) {
			qreal x = curve.temp[i];
			qreal y = static_cast<int>(curve.perc[i]);
			series->replace(i, QPointF(x, y));
		}

		chart->update();
	});

	connect(defaultBtn, &QPushButton::clicked, this, [this]() {
		auto curve = performanceService.getDefaultFanCurve(this->fan, this->profile);

		for (int i = 0; i < series->count(); ++i) {
			qreal x = curve.temp[i];
			qreal y = static_cast<int>(curve.perc[i]);
			series->replace(i, QPointF(x, y));
		}

		chart->update();
	});
}