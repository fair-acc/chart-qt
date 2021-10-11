#include "xyplot.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include "dataset.h"

namespace chart_qt {


void XYPlot::initialize()
{
    static const char *vs = "\
        attribute vec4 vertex;\n\
        uniform mat4 matrix;\n\
        void main()\n\
        {\n\
            gl_Position = matrix * vertex;\n\
        }";

    static const char *fs = "\
        void main()\n\
        {\n\
            gl_FragColor = vec4(1, 0, 0, 1);\n\
        }";

    m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
    m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);

    m_program.link();
    m_program.bind();

    auto vertexLocation = m_program.attributeLocation("vertex");
    auto gl = QOpenGLContext::currentContext()->functions();
    gl->glEnableVertexAttribArray(vertexLocation);

    m_buffer.create();
    m_buffer.bind();
    gl->glVertexAttribPointer(vertexLocation, 2, GL_FLOAT, true, 0, 0);
}

void XYPlot::updateData(int index, int count)
{
    auto ds = dataSet();
    m_buffer.bind();

    const int dataCount = dataSet()->getDataCount();
    if (m_allocated != dataCount) {
        m_buffer.allocate(dataCount * 8);
        m_allocated = dataCount;
    }

    auto data = static_cast<float *>(m_buffer.map(QOpenGLBuffer::WriteOnly));
    for (int i = 0; i < dataCount; ++i, data += 2) {
        const double x = ds->get(0, i);
        const double y = ds->get(1, i);
        data[0] = x;
        data[1] = y;
    }
    m_buffer.unmap();

    auto gl = QOpenGLContext::currentContext()->functions();
}

void XYPlot::render()
{
    m_program.bind();
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
