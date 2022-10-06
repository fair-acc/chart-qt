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

std::span<chart_qt::DataSet::Limit> SinDataSet::dataLimits()
{
    return _limits;
}

void SinDataSet::timerEvent(QTimerEvent *e) {
    Q_UNUSED(e)
    _offset += 0.1;

    // for (int i = 0; i < 1e5; ++i) {
    //     const float x = float(i) / 100.;
    //     _xdata[i]     = x;
    //     _ydata[i]     = std::sin(_offset + x);
    // }

    emit dataChanged(0, getDataCount());
}

constexpr int SIZE = 2000*2000;

SinDataSet2D::SinDataSet2D() {
    hasErrors         = true;

    startTimer(40);

    _xdata.resize(SIZE);
    _ydata.resize(SIZE);
    _zdata.resize(SIZE);

    _limits[0] = { 0, 2000 };
    _limits[1] = { 0, 2000 };
    _limits[2] = { -1, 1 };

    for (int i = 0; i < 2000; ++i) {
        const float x = float(i);
        for (int j = 0; j < 2000; ++j) {
            int idx = i * 2000 + j;
            _xdata[idx]     = x;

            const float y = float(j) ;
            _ydata[idx] = y;
            _zdata[idx]     = std::sin((_offset + x + y) / 100.f);
        }
    }
}

SinDataSet2D::~SinDataSet2D() {
}

float SinDataSet2D::get(int dimIndex, int index) const {
    return (dimIndex == 0 ? _xdata : dimIndex == 1 ? _ydata : _zdata)[index];
}

std::span<float> SinDataSet2D::getValues(int dimIndex) {
    return dimIndex == 0 ? _xdata : (dimIndex == 1 ? _ydata : _zdata);
}

int SinDataSet2D::getDataCount() const {
    return SIZE;
}

std::span<chart_qt::DataSet::Limit> SinDataSet2D::dataLimits()
{
    return _limits;
}

void SinDataSet2D::timerEvent(QTimerEvent *e) {
    _offset += 0.1;

    // for (int i = 0; i < 1e5; ++i) {
    //     const float x = float(i) / 100.;
    //     _xdata[i]     = x;
    //     _ydata[i]     = std::sin(_offset + x);
    // }

    dataChanged(0, getDataCount());
}

} // namespace ChartQtSample
