#ifndef XYPLOT_H
#define XYPLOT_H

#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

#include "plot.h"

namespace chart_qt {

class Node;

class XYPlot : public Plot
{
public:
    QSGNode *sgNode() override;
    void update(double w, double h) override;

private:
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_buffer;
    QOpenGLVertexArrayObject m_vao;
    size_t m_allocated = 0;
    Node *m_node = nullptr;
};

}

#endif
