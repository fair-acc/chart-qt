#include "plot.h"
#include "axis.h"
#include "chartitem.h"
#include "dataset.h"

namespace chart_qt {

Plot::Plot() {
}

Plot::~Plot() = default;

void Plot::setDataSet(DataSet *dataset) {
    if (m_dataset == dataset) {
        return;
    }

    if (m_dataset) {
        disconnect(dataset, &DataSet::dataChanged, this, &Plot::updateNeeded);
    }

    m_dataset = dataset;
    emit dataSetChanged();

    if (dataset) {
        connect(dataset, &DataSet::dataChanged, this, &Plot::updateNeeded);
        connect(dataset, &DataSet::dataChanged, this, [this]() { m_needsUpdate = true; });
    }
}

DataSet *Plot::dataSet() const {
    return m_dataset.data();
}

Axis *Plot::xAxis() const {
    return m_xAxis;
}

void Plot::setXAxis(Axis *axis) {
    if (m_xAxis == axis) {
        return;
    }

    if (m_xAxis) {
        disconnect(m_xAxis, &QObject::destroyed, this, &Plot::resetXAxis);
    }
    m_xAxis = axis;
    if (m_xAxis) {
        connect(m_xAxis, &QObject::destroyed, this, &Plot::resetXAxis);
    }

    emit xAxisChanged();
}

void Plot::resetXAxis() {
    setXAxis(nullptr);
}

Axis *Plot::yAxis() const {
    return m_yAxis;
}

void Plot::setYAxis(Axis *axis) {
    if (m_yAxis == axis) {
        return;
    }

    if (m_yAxis) {
        disconnect(m_yAxis, &QObject::destroyed, this, &Plot::resetXAxis);
    }
    m_yAxis = axis;
    if (m_yAxis) {
        connect(m_yAxis, &QObject::destroyed, this, &Plot::resetXAxis);
    }

    emit yAxisChanged();
}

void Plot::resetYAxis() {
    setYAxis(nullptr);
}

void Plot::classBegin() {
}

void Plot::componentComplete() {
    if (auto c = qobject_cast<ChartItem *>(parent())) {
        c->addPlot(this);
    }
}

} // namespace chart_qt
