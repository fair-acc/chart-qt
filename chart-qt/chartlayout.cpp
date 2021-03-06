#include "chartlayout.h"

#include <QDebug>

#include "chartitem.h"

namespace chart_qt {

ChartLayout::ChartLayout(QQuickItem *parent)
    : QQuickItem(parent) {
}

ChartLayout::~ChartLayout() noexcept {
}

QQmlListProperty<ChartItem> ChartLayout::charts() {
    QQmlListProperty<ChartItem>::AppendFunction append = [](QQmlListProperty<ChartItem> *list, ChartItem *chart) {
        auto layout = static_cast<ChartLayout *>(list->object);
        layout->_charts.push_back(chart);
        chart->setParentItem(layout);
        layout->polish();

        connect(chart, &ChartItem::implicitContentRectChanged, layout, &QQuickItem::polish);
    };
    QQmlListProperty<ChartItem>::CountFunction count = [](QQmlListProperty<ChartItem> *list) -> qsizetype {
        auto layout = static_cast<ChartLayout *>(list->object);
        return layout->_charts.size();
    };
    QQmlListProperty<ChartItem>::AtFunction at = [](QQmlListProperty<ChartItem> *list, qsizetype i) -> ChartItem * {
        auto layout = static_cast<ChartLayout *>(list->object);
        return layout->_charts[i];
    };
    QQmlListProperty<ChartItem>::ClearFunction clear = [](QQmlListProperty<ChartItem> *list) {
        auto layout = static_cast<ChartLayout *>(list->object);
        layout->_charts.clear();
    };
    return QQmlListProperty(this, this, append, count, at, clear);
}

Qt::Orientation ChartLayout::orientation() const {
    return _orientation;
}

void ChartLayout::setOrientation(Qt::Orientation o) {
    if (_orientation != o) {
        _orientation = o;
        emit orientationChanged();

        polish();
    }
}

void ChartLayout::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) {
    QQuickItem::geometryChange(newGeometry, oldGeometry);
    polish();
}

void ChartLayout::updatePolish() {
    if (_charts.size() == 0) {
        return;
    }

    QMarginsF margins;
    if (_orientation == Qt::Horizontal) {
        int x      = 0;
        int w      = width() / _charts.size();

        int top    = 0;
        int bottom = height();
        for (auto c : _charts) {
            c->setX(x);
            c->setY(0);
            c->setWidth(w);
            c->setHeight(height());

            x += w;

            auto r = c->implicitContentRect();
            top    = std::max(top, int(r.top()));
            bottom = std::min(bottom, int(r.bottom()));
        }

        margins = QMarginsF(0, top, 0, height() - bottom);
    } else {
        int y     = 0;
        int h     = height() / _charts.size();

        int left  = 0;
        int right = width();
        for (auto c : _charts) {
            c->setX(0);
            c->setY(y);
            c->setWidth(width());
            c->setHeight(h);

            y += h;

            auto r = c->implicitContentRect();
            left   = std::max(left, int(r.left()));
            right  = std::min(right, int(r.right()));
        }

        margins = QMarginsF(left, 0, width() - right, 0);
    }
    for (auto c : _charts) {
        c->setMinimumContentMargins(margins);
    }
}

} // namespace chart_qt
