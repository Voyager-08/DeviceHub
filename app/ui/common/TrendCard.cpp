#include "TrendCard.h"

#include <QPainter>
#include <QPolygonF>
#include <QVBoxLayout>
#include <QVector>

#include <algorithm>
#include <utility>

namespace devicehub {

class TrendWidget : public QWidget
{
public:
    explicit TrendWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(110);
    }

    void setRange(double minimum, double maximum)
    {
        m_minimum = minimum;
        m_maximum = maximum;
        if (m_maximum <= m_minimum)
            m_maximum = m_minimum + 1.0;
        update();
    }

    void addValue(double value)
    {
        m_points.append(value);
        while (m_points.size() > kMaxPoints)
            m_points.removeFirst();
        m_valid = true;
        update();
    }

    void setInvalid()
    {
        m_valid = false;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        const QPalette pal = palette();
        const QRectF plot = rect().adjusted(6, 8, -6, -8);

        // 背景网格
        QPen gridPen(pal.color(QPalette::Mid), 1);
        painter.setPen(gridPen);
        for (int i = 1; i < 4; ++i) {
            const qreal y = plot.top() + plot.height() * i / 4.0;
            painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        }

        if (!m_valid || m_points.isEmpty()) {
            painter.setPen(pal.color(QPalette::Text));
            painter.drawText(plot, Qt::AlignCenter, QStringLiteral("--"));
            return;
        }

        painter.setPen(QPen(pal.color(QPalette::Highlight), 2));

        QPolygonF polygon;
        const int count = m_points.size();
        const double range = m_maximum - m_minimum;

        for (int i = 0; i < count; ++i) {
            const double ratio =
                std::clamp((m_points.at(i) - m_minimum) / range, 0.0, 1.0);
            const qreal x = count == 1
                ? plot.center().x()
                : plot.left() + plot.width() * i / (count - 1);
            const qreal y = plot.bottom() - plot.height() * ratio;
            polygon.append(QPointF(x, y));
        }

        if (polygon.size() == 1) {
            painter.setBrush(pal.color(QPalette::Highlight));
            painter.drawEllipse(polygon.first(), 3, 3);
        } else {
            painter.setBrush(Qt::NoBrush);
            painter.drawPolyline(polygon);
        }
    }

private:
    static constexpr int kMaxPoints = 60;
    QVector<double> m_points;
    double m_minimum = 0.0;
    double m_maximum = 100.0;
    bool m_valid = false;
};

TrendCard::TrendCard(DashboardItemSpec spec, QWidget *parent)
    : DashboardCard(std::move(spec), parent)
{
    m_trend = new TrendWidget(this);
    m_trend->setRange(this->spec().minValue, this->spec().maxValue);
    contentLayout()->addWidget(m_trend, 1);
}

void TrendCard::setValue(const QVariant &value)
{
    bool ok = false;
    const double number = value.toDouble(&ok);
    if (!ok) {
        m_trend->setInvalid();
        return;
    }
    m_trend->addValue(number);
}

} // namespace devicehub
