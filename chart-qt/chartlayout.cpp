#include "chartlayout.h"

#include <QDebug>

#include "chartitem.h"

namespace chart_qt {

ChartLayout::ChartLayout(QQuickItem *parent)
           : QQuickItem(parent)
{
}

ChartLayout::~ChartLayout() noexcept
{
}

QQmlListProperty<ChartItem> ChartLayout::charts()
{
    QQmlListProperty<ChartItem>::AppendFunction append = [](QQmlListProperty<ChartItem> *list, ChartItem *chart) {
        auto layout = static_cast<ChartLayout *>(list->object);
        layout->m_charts.push_back(chart);
        chart->setParentItem(layout);
        layout->polish();
    };
    QQmlListProperty<ChartItem>::CountFunction count = [](QQmlListProperty<ChartItem> *list) -> qsizetype {
        auto layout = static_cast<ChartLayout *>(list->object);
        return layout->m_charts.size();
    };
    QQmlListProperty<ChartItem>::AtFunction at = [](QQmlListProperty<ChartItem> *list, qsizetype i) -> ChartItem * {
        auto layout = static_cast<ChartLayout *>(list->object);
        return layout->m_charts[i];
    };
    QQmlListProperty<ChartItem>::ClearFunction clear = [](QQmlListProperty<ChartItem> *list) {
        auto layout = static_cast<ChartLayout *>(list->object);
        layout->m_charts.clear();
    };
    return QQmlListProperty(this, this, append, count, at, clear);
}

Qt::Orientation ChartLayout::orientation() const
{
    return m_orientation;
}

void ChartLayout::setOrientation(Qt::Orientation o)
{
    if (m_orientation != o) {
        m_orientation = o;
        emit orientationChanged();

        polish();
    }
}

void ChartLayout::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    polish();
}

void ChartLayout::updatePolish()
{
    if (m_charts.size() == 0) {
        return;
    }

    QMarginsF margins;
    if (m_orientation == Qt::Horizontal) {
        int x = 0;
        int w = width() / m_charts.size();

        int top = 0;
        int bottom = height();
        for (auto c: m_charts) {
            c->setX(x);
            c->setY(0);
            c->setWidth(w);
            c->setHeight(height());

            x += w;

            auto r = c->implicitContentRect();
            top = std::max(top, int(r.top()));
            bottom = std::min(bottom, int(r.bottom()));
        }

        margins = QMarginsF(0, top, 0, height() - bottom);
    } else {
        int y = 0;
        int h = height() / m_charts.size();

        int left = 0;
        int right = width();
        for (auto c: m_charts) {
            c->setX(0);
            c->setY(y);
            c->setWidth(width());
            c->setHeight(h);

            y += h;

            auto r = c->implicitContentRect();
            left = std::max(left, int(r.left()));
            right = std::min(right, int(r.right()));
        }

        margins = QMarginsF(left, 0, width() - right, 0);
    }
    for (auto c: m_charts) {
        c->setMinimumContentMargins(margins);
    }
}

}
