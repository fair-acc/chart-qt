#include "plot.h"
#include "axis.h"
#include "chartitem.h"
#include "dataset.h"

namespace chart_qt {

Plot::Plot() {
}

Plot::~Plot() = default;

void Plot::setDataSet(DataSet *dataset) {
    if (_dataset == dataset) {
        return;
    }

    if (_dataset) {
        disconnect(dataset, &DataSet::dataChanged, this, &Plot::updateNeeded);
    }

    _dataset = dataset;
    emit dataSetChanged();

    if (dataset) {
        connect(dataset, &DataSet::dataChanged, this, &Plot::updateNeeded);
        connect(dataset, &DataSet::dataChanged, this, [this]() { _needsUpdate = true; });
    }
}

DataSet *Plot::dataSet() const {
    return _dataset.data();
}

Axis *Plot::xAxis() const {
    return _xAxis;
}

void Plot::setXAxis(Axis *axis) {
    if (_xAxis == axis) {
        return;
    }

    if (_xAxis) {
        disconnect(_xAxis, &QObject::destroyed, this, &Plot::resetXAxis);
    }
    _xAxis = axis;
    if (_xAxis) {
        connect(_xAxis, &QObject::destroyed, this, &Plot::resetXAxis);
    }

    emit xAxisChanged();
}

void Plot::resetXAxis() {
    setXAxis(nullptr);
}

Axis *Plot::yAxis() const {
    return _yAxis;
}

void Plot::setYAxis(Axis *axis) {
    if (_yAxis == axis) {
        return;
    }

    if (_yAxis) {
        disconnect(_yAxis, &QObject::destroyed, this, &Plot::resetXAxis);
    }
    _yAxis = axis;
    if (_yAxis) {
        connect(_yAxis, &QObject::destroyed, this, &Plot::resetXAxis);
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
