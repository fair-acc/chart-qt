#include "xyplot.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_1_Core>
#include <QLineSeries>

#include "dataset.h"

namespace chart_qt {

XYPlot::XYPlot()
      : m_series(new QLineSeries)
{
    connect(this, &Plot::update, this, &XYPlot::updateSeries);
}

void XYPlot::updateSeries()
{
    auto ds = dataSet();
    
    const int dataCount = ds->getDataCount();
    const auto xdata = ds->getValues(0).data();
    const auto ydata = ds->getValues(1).data();

    QList<QPointF> points;
    for (int i = 0; i < dataCount; ++i) {
        points << QPointF(xdata[i], ydata[i]);
    }
    m_series->replace(points);
}

QAbstractSeries *XYPlot::series() const
{
    return m_series;
}

void XYPlot::initialize()
{
    static const char *vs = "\
        attribute float vx;\n\
        attribute float vy;\n\
        uniform mat4 matrix;\n\
        void main()\n\
        {\n\
            gl_Position = matrix * vec4(vx, vy, 0, 1);\n\
        }";

//     static const char *vs = "\
//         #version 410\n\
//         in double vx;\n\
//         in double vy;\n\
//         uniform mat4 matrix;\n\
//         void main()\n\
//         {\n\
//             gl_Position = matrix * vec4(vx, vy, 0, 0);\n\
//         }";

    static const char *fs = "\
        void main()\n\
        {\n\
            gl_FragColor = vec4(1, 0, 0, 1);\n\
        }";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);

    m_program.link();
    m_program.bind();

    qDebug()<<QOpenGLContext::currentContext()->format();

//         m_vao.create();
//     qDebug()<<m_vao.isCreated();
//     m_vao.bind();

    auto vertexLocation = m_program.attributeLocation("vx");
    auto gl = QOpenGLContext::currentContext()->functions();
//     auto gl = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_4_1_Core>();
    gl->glEnableVertexAttribArray(vertexLocation);



    m_buffer.create();
    m_buffer.bind();
    gl->glVertexAttribPointer(vertexLocation, 1, GL_FLOAT, true, 0, 0);
//     gl->glVertexAttribLPointer(vertexLocation, 1, GL_DOUBLE, 0, 0);

    auto vy = m_program.attributeLocation("vy");
    gl->glEnableVertexAttribArray(vy);
qDebug()<<"/init";

}

void XYPlot::updateData(int index, int count)
{
//     qDebug()<<"up";
//     m_vao.bind();
    auto ds = dataSet();
    m_buffer.bind();

    const int dataCount = dataSet()->getDataCount();
    if (m_allocated != dataCount) {
        m_buffer.allocate(dataCount * sizeof(double) * 2);
        m_allocated = dataCount;
    }

    const auto xdata = ds->getValues(0).data();
    const auto ydata = ds->getValues(1).data();
//     qDebug()<<"up1"<<m_buffer.isCreated();

//     auto data = static_cast<double *>(m_buffer.map(QOpenGLBuffer::WriteOnly));
    auto data = static_cast<float *>(m_buffer.map(QOpenGLBuffer::WriteOnly));
    auto ydst = data + dataCount;
//     memcpy(data, xdata, dataCount * sizeof(double));
//     memcpy(ydst, ydata, dataCount * sizeof(double));
    for (int i = 0; i < dataCount; ++i) {
//         const double x = ds->get(0, i);
//         const double y = ds->get(1, i);
//         data[0] = x;
//         data[1] = y;
        data[i] = float(xdata[i]);
        ydst[i] = float(ydata[i]);
    }
// qDebug()<<"up2";

    m_buffer.unmap();

    auto gl = QOpenGLContext::currentContext()->functions();
//     auto gl = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_4_1_Core>();

    auto vy = m_program.attributeLocation("vy");
    gl->glVertexAttribPointer(vy, 1, GL_FLOAT, true, 0, (void *)(dataCount * sizeof(float)));
//     qDebug()<<"/up";
//     gl->glVertexAttribLPointer(vy, 1, GL_DOUBLE, 0, (void *)(dataCount * sizeof(double)));
}

void XYPlot::render()
{
    m_program.bind();
//     m_vao.bind();
    m_buffer.bind();
    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glDrawArrays(GL_LINE_STRIP, 0, dataSet()->getDataCount());
}

void XYPlot::handleResize(int w, int h)
{
    QMatrix4x4 matrix;
    matrix.ortho(QRect{ 0, 0, w, h });
    matrix.translate(0, h / 2);
    matrix.scale(20, 200);
    m_program.setUniformValue("matrix", matrix);
}

}
