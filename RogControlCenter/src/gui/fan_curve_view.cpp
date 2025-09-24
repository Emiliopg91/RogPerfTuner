#include "../../include/gui/fan_curve_view.hpp"

#include <qlineseries.h>
#include <qnamespace.h>

#include <QtCharts>

FanCurveView::FanCurveView(QLineSeries* series, QWidget* parent) : QChartView(new QChart(), parent), series(series) {
	setRenderHint(QPainter::Antialiasing);
	chart()->legend()->hide();
}

void FanCurveView::paintEvent(QPaintEvent* event) {
	QChartView::paintEvent(event);

	if (!series) {
		return;
	}

	QPainter painter(viewport());
	painter.setRenderHint(QPainter::Antialiasing);
	QFont font = painter.font();
	font.setPointSize(8);
	painter.setFont(font);

	for (int i = 0; i < series->count(); ++i) {
		QPointF p = series->at(i);

		bool drawLabel = (selectedIndex >= 0 && i == selectedIndex) || (hasHover && p == hoverPos);
		if (!drawLabel) {
			continue;
		}

		QPointF pos	  = chart()->mapToPosition(p);
		QString label = QString("%1°C, %2%").arg(int(p.x())).arg(int(p.y()));

		QRectF textRect = painter.fontMetrics().boundingRect(label);
		textRect.moveTo(pos + QPointF(-textRect.width(), -textRect.height() - 5));

		QColor semiTransparentWhite(255, 255, 255, 128);  // fondo semi-transparente
		painter.fillRect(textRect.adjusted(-2, -1, 2, 1), semiTransparentWhite);

		painter.setPen(Qt::black);
		painter.drawText(textRect.topLeft() + QPointF(0, textRect.height() - 2), label);
	}
}

void FanCurveView::showEvent(QShowEvent* event) {
	QPalette pal = QApplication::palette();
	QColor bg	 = pal.window().color();
	QColor text	 = pal.windowText().color();

	bool dark = (bg.lightness() < 128);

	chart()->setBackgroundBrush(bg);
	chart()->setBackgroundPen(Qt::NoPen);

	for (auto* axis : chart()->axes()) {
		QPen axisPen(dark ? Qt::white : Qt::black);
		axis->setLinePen(axisPen);
		axis->setLabelsColor(dark ? Qt::white : Qt::black);
		axis->setTitleBrush(dark ? Qt::white : Qt::black);
	}

	QPen pen = series->pen();
	pen.setColor(dark ? Qt::red : Qt::black);
	series->setPen(pen);

	QChartView::showEvent(event);
}
void FanCurveView::mouseMoveEvent(QMouseEvent* event) {
	QPointF pos = chart()->mapToValue(event->pos(), series);

	// Actualizar hover
	int hoverIndex = findClosestPoint(pos);
	if (hoverIndex >= 0) {
		hoverPos = series->at(hoverIndex);
		hasHover = true;
	} else {
		hasHover = false;
	}

	// Movimiento de drag
	if (selectedIndex >= 0) {
		QPointF pos = chart()->mapToValue(event->pos(), series);

		// ----- Limitar en X -----
		qreal minX = 0.0;
		qreal maxX = 100.0;

		if (selectedIndex == 0) {
			maxX = series->at(1).x();
		} else if (selectedIndex == series->count() - 1) {
			minX = series->at(series->count() - 2).x();
		} else {
			minX = series->at(selectedIndex - 1).x();
			maxX = series->at(selectedIndex + 1).x();
		}

		qreal x = std::clamp(pos.x(), minX, maxX);
		x		= std::round(x);  // 🔹 solo enteros

		// ----- Limitar en Y -----
		qreal minY = 0.0;
		qreal maxY = 100.0;

		if (selectedIndex == 0) {
			maxY = series->at(1).y();
		} else if (selectedIndex == series->count() - 1) {
			minY = series->at(series->count() - 2).y();
		} else {
			minY = series->at(selectedIndex - 1).y();
			maxY = series->at(selectedIndex + 1).y();
		}

		qreal y = std::clamp(pos.y(), minY, maxY);
		y		= std::round(y);  // 🔹 solo enteros

		series->replace(selectedIndex, QPointF(x, y));
		chart()->update();
	}

	chart()->update();
}

// --- mousePressEvent / mouseReleaseEvent ---
void FanCurveView::mousePressEvent(QMouseEvent* event) {
	QPointF pos	  = chart()->mapToValue(event->pos(), series);
	selectedIndex = findClosestPoint(pos);
	chart()->update();
}

void FanCurveView::mouseReleaseEvent(QMouseEvent*) {
	selectedIndex = -1;
	chart()->update();
}

int FanCurveView::findClosestPoint(const QPointF& pos) {
	int idx		   = -1;
	qreal bestDist = 1e9;
	for (int i = 0; i < series->count(); ++i) {
		QPointF p = series->at(i);
		qreal d	  = std::hypot(p.x() - pos.x(), p.y() - pos.y());
		if (d < bestDist) {
			bestDist = d;
			idx		 = i;
		}
	}
	return (bestDist < 2.0) ? idx : -1;
}