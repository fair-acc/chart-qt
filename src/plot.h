#ifndef PLOT_H
#define PLOT_H

#include <QObject>

class QSGNode;

namespace chart_qt {

class DataSet;

class Plot : public QObject
{
    Q_OBJECT
public:

    virtual QSGNode *sgNode() = 0;
    virtual void update(double w, double h) = 0;

    void setDataSet(DataSet *dataset);
    inline DataSet *dataSet() const { return m_dataset; }

    bool m_needsUpdate = false;

signals:
    void dataSetChanged();
    void updateNeeded();

private:
    DataSet *m_dataset = nullptr;
};

}

#endif
