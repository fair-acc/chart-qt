#include "sindataset.h"

#include <cmath>

namespace chart_qt
{

SinDataSet::SinDataSet()
{
    startTimer(40);
    m_xdata.resize(1e5);
    m_ydata.resize(1e5);
}

SinDataSet::~SinDataSet()
{
}

float SinDataSet::get(int dimIndex, int index) const
{
    return (dimIndex == 0 ? m_xdata : m_ydata)[index];
}

std::span<float> SinDataSet::getValues(int dimIndex)
{
    return dimIndex == 0 ? m_xdata : m_ydata;
}

int SinDataSet::getDataCount() const
{
    return 1e5;
}

void SinDataSet::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e)
    m_offset += 0.1;

    for (int i = 0; i < 1e5; ++i) {
        const float x = float(i) / 100.;
        m_xdata[i] = x;
        m_ydata[i] = std::sin(m_offset + x);
    }

    emit dataChanged(0, getDataCount());
}

}
