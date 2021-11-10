#ifndef WATERFALLPLOT_H
#define WATERFALLPLOT_H

#include <QQuick3DTextureData>

namespace chart_qt {

class WaterfallPlot : public QQuick3DTextureData
{
    Q_OBJECT
    Q_PROPERTY(int offset READ offset NOTIFY offsetChanged)
    Q_PROPERTY(double gradientStart READ gradientStart WRITE setGradientStart NOTIFY gradientStartChanged)
    Q_PROPERTY(double gradientEnd READ gradientEnd WRITE setGradientEnd NOTIFY gradientEndChanged)
public:
    WaterfallPlot();

    int offset() const { return m_offset; }

    double gradientStart() const;
    void setGradientStart(double g);

    double gradientEnd() const;
    void setGradientEnd(double g);

signals:
    void offsetChanged();
    void gradientStartChanged();
    void gradientEndChanged();

private:
    void updateData();
    void updateTex(int start, int end);

    QByteArray m_data;
    QByteArray m_tex;
    int m_numLines = 0;
    double m_gradient[2] = { 0.2, 0.8 };
    int m_offset = 0;
};

}

#endif
