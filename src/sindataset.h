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

    static SinDataSet *instance();

protected:
    void timerEvent(QTimerEvent *e) final;

private:
    double m_offset = 0;
    std::vector<float> m_xdata;
    std::vector<float> m_ydata;
};

}

#endif
