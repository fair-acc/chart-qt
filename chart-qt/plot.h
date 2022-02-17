#ifndef PLOT_H
#define PLOT_H

#include <QObject>
#include <QPointer>
#include <QQmlParserStatus>

class QSGNode;
class QQuickWindow;

namespace chart_qt {

class DataSet;
class Axis;
class PlotRenderer;

class Plot : public QObject, public QQmlParserStatus {
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(Axis *xAxis READ xAxis WRITE setXAxis NOTIFY xAxisChanged)
    Q_PROPERTY(Axis *yAxis READ yAxis WRITE setYAxis NOTIFY yAxisChanged)
    Q_PROPERTY(DataSet *dataSet READ dataSet WRITE setDataSet NOTIFY dataSetChanged)
public:
    Plot();
    ~Plot();

    virtual void          update(QQuickWindow *window, const QRect &chartRect, double devicePixelRatio, bool paused) = 0;
    virtual PlotRenderer *renderer()                                                                                 = 0;

    void                  setDataSet(DataSet *dataset);
    DataSet              *dataSet() const;

    Axis                 *xAxis() const;
    void                  setXAxis(Axis *axis);

    Axis                 *yAxis() const;
    void                  setYAxis(Axis *axis);

    bool                  m_needsUpdate = false;

    void                  classBegin() override;
    void                  componentComplete() override;

signals:
    void dataSetChanged();
    void updateNeeded();
    void xAxisChanged();
    void yAxisChanged();

private:
    void              resetXAxis();
    void              resetYAxis();

    QPointer<DataSet> m_dataset;
    QPointer<Axis>    m_xAxis;
    QPointer<Axis>    m_yAxis;
};

} // namespace chart_qt

#endif
