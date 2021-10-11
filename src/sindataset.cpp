#include "sindataset.h"

#include <cmath>

namespace chart_qt
{

SinDataSet::SinDataSet()
{
    startTimer(10);
}

SinDataSet::~SinDataSet()
{
}

double SinDataSet::get(int dimIndex, int index) const
{
    const double x = double(index) / 100.;
    if (dimIndex == 0) {
        return x;
    }
    return std::sin(m_offset + x);
}

int SinDataSet::getDataCount() const
{
    return 1e5;
}

void SinDataSet::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e)
    m_offset += 0.1;

    emit dataChanged(0, getDataCount());
}

}
