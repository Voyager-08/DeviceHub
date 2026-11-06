#include "GaugeCard.h"

#include <QPainter>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <utility>

namespace devicehub {

// 简易环形仪表:颜色全部取自调色板
class GaugeWidget : public QWidget
{
public:
    explicit GaugeWidget(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(120, 120);
    }

    void setRange(double minimum, double maximum)
    {
        m_minimum = minimum;
        m_maximum = maximum;
        if (m_maximum <= m_minimum)
            m_maximum = m_minimum + 1.0;
        update();
    }

    void setUnit(const QString &unit)
    {
        m_unit = unit;
        update();
    }

    void setValue(double value)
    {
        m_value = value;
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

        const int side = std::min(width(), height()) - 16;
        const QRectF rect((width() - side) / 2.0, (height() - side) / 2.0,
                          side, side);

        const QPalette pal = palette();
        const QRectF arcRect = rect.adjusted(12, 12, -12, -12);

        QPen trackPen(pal.color(QPalette::Mid), 12);
        trackPen.setCapStyle(Qt::RoundCap);
        painter.setPen(trackPen);
        painter.drawArc(arcRect, 225 * 16, -270 * 16);

        double ratio = 0.0;
        if (m_valid) {
            ratio = (m_value - m_minimum) / (m_maximum - m_minimum);
            ratio = std::clamp(ratio, 0.0, 1.0);
        }

        if (ratio > 0.0) {
            QPen valuePen(pal.color(QPalette::Highlight), 12);
            valuePen.setCapStyle(Qt::RoundCap);
            painter.setPen(valuePen);
            painter.drawArc(arcRect, 225 * 16,
                            static_cast<int>(-270 * 16 * ratio));
        }

        QString text;
        if (m_valid) {
            if (std::abs(m_value - std::round(m_value)) < 0.05)
                text = QString::number(std::llround(m_value));
            else
                text = QString::number(m_value, 'f', 1);
            if (!m_unit.isEmpty())
                text += m_unit;
        } else {
            text = QStringLiteral("--");
        }

        QFont font = painter.font();
        font.setBold(true);
        font.setPointSize(font.pointSize() + 6);
        painter.setFont(font);
        painter.setPen(pal.color(QPalette::Text));
        painter.drawText(rect, Qt::AlignCenter, text);
    }

private:
    double m_minimum = 0.0;
    double m_maximum = 100.0;
    double m_value = 0.0;
    QString m_unit;
    bool m_valid = false;
};

GaugeCard::GaugeCard(DashboardItemSpec spec, QWidget *parent)
    : DashboardCard(std::move(spec), parent)
{
    m_gauge = new GaugeWidget(this);
    m_gauge->setRange(this->spec().minValue, this->spec().maxValue);
    m_gauge->setUnit(this->spec().unit);
    contentLayout()->addWidget(m_gauge, 1);
}

void GaugeCard::setValue(const QVariant &value)
{
    bool ok = false;
    const double number = value.toDouble(&ok);
    if (!ok) {
        m_gauge->setInvalid();
        return;
    }
    m_gauge->setValue(number);
}

} // namespace devicehub
