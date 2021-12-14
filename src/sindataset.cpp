#include "sindataset.h"

#include <cmath>

namespace chart_qt
{

SinDataSet::SinDataSet()
{
    hasErrors = true;

    startTimer(40);
    m_xdata.resize(1e5);
    m_ydata.resize(1e5);

    for (int i = 0; i < 1e5; ++i) {
        const float x = float(i) / 100.;
        m_xdata[i] = x;
        m_ydata[i] = std::sin(m_offset + x);
    }

    m_xPosErrors.resize(1e5, 0.3);
    m_xNegErrors.resize(1e5, 0.3);
    m_yPosErrors.resize(1e5, 0.2);
    m_yNegErrors.resize(1e5, 0.1);
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

std::span<float> SinDataSet::getPositiveErrors(int dimIndex)
{
    return dimIndex == 0 ? m_xPosErrors : m_yPosErrors;
}

std::span<float> SinDataSet::getNegativeErrors(int dimIndex)
{
    return dimIndex == 0 ? m_xNegErrors : m_yNegErrors;
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
