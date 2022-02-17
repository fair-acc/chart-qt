#include "sindataset.h"

#include <cmath>

namespace ChartQtSample {

SinDataSet::SinDataSet() {
    hasErrors = true;

    startTimer(40);
    _xdata.resize(1e5);
    _ydata.resize(1e5);

    for (int i = 0; i < 1e5; ++i) {
        const float x = float(i) / 100.;
        _xdata[i]     = x;
        _ydata[i]     = std::sin(_offset + x);
    }

    _xPosErrors.resize(1e5, 0.3);
    _xNegErrors.resize(1e5, 0.3);
    _yPosErrors.resize(1e5, 0.2);
    _yNegErrors.resize(1e5, 0.1);
}

SinDataSet::~SinDataSet() {
}

float SinDataSet::get(int dimIndex, int index) const {
    return (dimIndex == 0 ? _xdata : _ydata)[index];
}

std::span<float> SinDataSet::getValues(int dimIndex) {
    return dimIndex == 0 ? _xdata : _ydata;
}

std::span<float> SinDataSet::getPositiveErrors(int dimIndex) {
    return dimIndex == 0 ? _xPosErrors : _yPosErrors;
}

std::span<float> SinDataSet::getNegativeErrors(int dimIndex) {
    return dimIndex == 0 ? _xNegErrors : _yNegErrors;
}

int SinDataSet::getDataCount() const {
    return 1e5;
}

void SinDataSet::timerEvent(QTimerEvent *e) {
    Q_UNUSED(e)
    _offset += 0.1;

    for (int i = 0; i < 1e5; ++i) {
        const float x = float(i) / 100.;
        _xdata[i]     = x;
        _ydata[i]     = std::sin(_offset + x);
    }

    emit dataChanged(0, getDataCount());
}

} // namespace ChartQtSample
