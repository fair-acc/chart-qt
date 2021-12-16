#ifndef PLOT_H
#define PLOT_H

#include <QObject>
#include <QQmlParserStatus>

class QSGNode;
class QQuickWindow;

namespace chart_qt {

class DataSet;
class Axis;

class Plot : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(Axis *xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    Q_PROPERTY(Axis *yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
public:

    virtual QSGNode *sgNode() = 0;
    virtual void update(QQuickWindow *window, double w, double h, bool paused) = 0;

    void setDataSet(DataSet *dataset);
    inline DataSet *dataSet() const { return m_dataset; }

    Axis *xAxis() const;
    void setXAxis(Axis *axis);

    Axis *yAxis() const;
    void setYAxis(Axis *axis);

    bool m_needsUpdate = false;

    void classBegin() override;
    void componentComplete() override;

signals:
    void dataSetChanged();
    void updateNeeded();
    void xAxisChanged();
    void yAxisChanged();

private:
    void resetXAxis();
    void resetYAxis();

    DataSet *m_dataset = nullptr;
    Axis *m_xAxis = nullptr;
    Axis *m_yAxis = nullptr;
};

}

#endif
