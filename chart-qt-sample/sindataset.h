#ifndef SINDATASET_H
#define SINDATASET_H

#include <dataset.h>

namespace ChartQtSample {

class SinDataSet : public chart_qt::DataSet {
public:
    SinDataSet();
    ~SinDataSet();

    float            get(int dimIndex, int index) const final;
    int              getDataCount() const final;
    int              getDimension() const final { return 2; }
    std::span<float> getValues(int dimIndex) final;
    std::span<chart_qt::DataSet::Limit> dataLimits() final;

    std::span<float> getPositiveErrors(int dimIndex) final;
    std::span<float> getNegativeErrors(int dimIndex) final;

protected:
    void timerEvent(QTimerEvent *e) final;

private:
    double             _offset = 0;
    std::vector<float> _xdata;
    std::vector<float> _ydata;
    std::vector<float> _xPosErrors;
    std::vector<float> _xNegErrors;
    std::vector<float> _yPosErrors;
    std::vector<float> _yNegErrors;
    chart_qt::DataSet::Limit _limits[2];
};

class SinDataSet2D : public chart_qt::DataSet {
public:
    SinDataSet2D();
    ~SinDataSet2D();

    float            get(int dimIndex, int index) const final;
    int              getDataCount() const final;
    int              getDimension() const final { return 3; }
    std::span<float> getValues(int dimIndex) final;
    std::span<chart_qt::DataSet::Limit> dataLimits() final;

protected:
    void timerEvent(QTimerEvent *e) final;

private:
    double             _offset = 0;
    std::vector<float> _xdata;
    std::vector<float> _ydata;
    std::vector<float> _zdata;
    chart_qt::DataSet::Limit _limits[3];
};

} // namespace ChartQtSample

#endif
