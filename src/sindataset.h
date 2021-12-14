#ifndef SINDATASET_H
#define SINDATASET_H

#include "dataset.h"

namespace chart_qt {

class SinDataSet final : public DataSet
{
public:
    SinDataSet();
    ~SinDataSet();

    float get(int dimIndex, int index) const final;
    int getDataCount() const final;
    int getDimension() const final { return 2; }
    std::span<float> getValues(int dimIndex) final;

    std::span<float> getPositiveErrors(int dimIndex) final;
    std::span<float> getNegativeErrors(int dimIndex) final;

protected:
    void timerEvent(QTimerEvent *e) final;

private:
    double m_offset = 0;
    std::vector<float> m_xdata;
    std::vector<float> m_ydata;
    std::vector<float> m_xPosErrors;
    std::vector<float> m_xNegErrors;
    std::vector<float> m_yPosErrors;
    std::vector<float> m_yNegErrors;
};

}

#endif
