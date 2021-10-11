#ifndef SINDATASET_H
#define SINDATASET_H

#include "dataset.h"

namespace chart_qt {

class SinDataSet final : public DataSet
{
public:
    SinDataSet();
    ~SinDataSet();

    double get(int dimIndex, int index) const final;
    int getDataCount() const final;
    int getDimension() const final { return 2; }

protected:
    void timerEvent(QTimerEvent *e) final;

private:
    double m_offset = 0;
};

}

#endif
