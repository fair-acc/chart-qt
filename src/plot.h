#ifndef PLOT_H
#define PLOT_H

#include <QObject>

class QAbstractSeries;

namespace chart_qt {

class DataSet;

class Plot : public QObject
{
    Q_OBJECT
public:

    virtual void initialize() = 0;
    virtual void updateData(int index, int count) = 0;
    virtual void render() = 0;
    virtual void handleResize(int width, int height);

    virtual QAbstractSeries *series() const = 0;

    void setDataSet(DataSet *dataset);
    inline DataSet *dataSet() const { return m_dataset; }

signals:
    void dataSetChanged();
    void update();

private:
    DataSet *m_dataset = nullptr;
};

}

#endif
