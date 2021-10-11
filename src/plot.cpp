#include "plot.h"
#include "dataset.h"

namespace chart_qt {

void Plot::setDataSet(DataSet *dataset)
{
    if (m_dataset == dataset) {
        return;
    }

    if (m_dataset) {
        disconnect(dataset, &DataSet::dataChanged, this, &Plot::updateNeeded);
    }

    m_dataset = dataset;
    emit dataSetChanged();

    if (dataset) {
        connect(dataset, &DataSet::dataChanged, this, &Plot::updateNeeded);
        connect(dataset, &DataSet::dataChanged, this, [this]() { m_needsUpdate = true; });
    }
}

}
