#include "xyplot.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGGeometryNode>
#include <QSGGeometry>
#include <QSGFlatColorMaterial>
#include <QSGRenderNode>
#include <QOpenGLExtraFunctions>

#ifndef WASM
#include <QOpenGLVersionFunctionsFactory>
#include <QOpenGLFunctions_4_1_Core>
#endif // WASM

#include "dataset.h"

namespace chart_qt {

class Node : public QSGRenderNode
{
public:
    Node()
#ifdef WASM
        : m_doubleSupported(false)
#else
        : m_doubleSupported(QOpenGLContext::currentContext()->format().majorVersion() >= 4 &&
                            QOpenGLContext::currentContext()->format().minorVersion() >= 1)
#endif // WASM
    {
        static const char *vs_float = "\
            attribute float vx;\n\
            attribute float vy;\n\
            uniform mat4 matrix;\n\
            void main()\n\
            {\n\
                gl_Position = matrix * vec4(vx, vy, 0, 1);\n\
            }";

        static const char *vs_double = "\
            #version 410\n\
            in double vx;\n\
            in double vy;\n\
            uniform mat4 matrix;\n\
            void main()\n\
            {\n\
                gl_Position = matrix * vec4(vx, vy, 0, 1);\n\
            }";

        static const char *fs = "\
            void main()\n\
            {\n\
                gl_FragColor = vec4(1, 0, 0, 1);\n\
            }";

        m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, m_doubleSupported ? vs_double : vs_float);
        m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fs);

        m_program.link();
        m_program.bind();

        qDebug()<<QOpenGLContext::currentContext()->format();

        m_vxLocation = m_program.attributeLocation("vx");
        m_vyLocation = m_program.attributeLocation("vy");

        m_buffer.create();
        m_vao.create();
    }

    QSGRenderNode::RenderingFlags flags() const
    {
        return QSGRenderNode::BoundedRectRendering;
    }

    void updateData(DataSet *ds)
    {
        m_vao.bind();
        m_buffer.bind();

        const int dataCount = ds->getDataCount();
        const int dataSize = dataCount * (m_doubleSupported ? sizeof(double) : sizeof(float)) * 2;
        if (m_allocated != dataCount) {
            m_buffer.allocate(dataSize);
            m_allocated = dataCount;

            if (m_vao.isCreated()) {
                setAttribs();
            }
        }

        const auto xdata = ds->getValues(0).data();
        const auto ydata = ds->getValues(1).data();

        auto gl = QOpenGLContext::currentContext()->extraFunctions();

        // can't use QOpenGLBuffer::map directly, because on WASM, only glMapBufferRange is available and it must be
        // called with GL_MAP_INVALIDATE_BUFFER_BIT
        auto data = gl->glMapBufferRange(GL_ARRAY_BUFFER,  0, dataSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        if (m_doubleSupported) {
            auto xdst = static_cast<double *>(data);
            auto ydst = xdst + dataCount;

            memcpy(xdst, xdata, dataCount * sizeof(double));
            memcpy(ydst, ydata, dataCount * sizeof(double));
        } else {
            auto xdst = static_cast<float *>(data);
            auto ydst = xdst + dataCount;

            for (int i = 0; i < dataCount; ++i) {
                xdst[i] = float(xdata[i]);
                ydst[i] = float(ydata[i]);
            }
        }
        gl->glUnmapBuffer(GL_ARRAY_BUFFER);

                        m_vao.release();
    }

    void setAttribs()
    {
        auto gl = QOpenGLContext::currentContext()->functions();
        gl->glEnableVertexAttribArray(m_vxLocation);
        gl->glEnableVertexAttribArray(m_vyLocation);

        if (m_doubleSupported) {
#ifndef WASM
            auto gl4 = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_4_1_Core>();
            gl4->glVertexAttribLPointer(m_vxLocation, 1, GL_DOUBLE, 0, 0);
            gl4->glVertexAttribLPointer(m_vyLocation, 1, GL_DOUBLE, 0, (void *)(m_allocated * sizeof(double)));
#endif
        } else {
            gl->glVertexAttribPointer(m_vxLocation, 1, GL_FLOAT, true, 0, 0);
            gl->glVertexAttribPointer(m_vyLocation, 1, GL_FLOAT, true, 0, (void *)(m_allocated * sizeof(float)));
        }
    }

    void prepare() final
    {
        // matrix() returns a dangling pointer in render(), so copy it here for later.
        // See https://bugreports.qt.io/browse/QTBUG-97589
        m_matrix = *matrix();
    }

    void render(const QSGRenderNode::RenderState *state) final
    {
        auto gl = QOpenGLContext::currentContext()->functions();

        if (m_vao.isCreated()) {
            m_vao.bind();
        } else {
            m_buffer.bind();
            setAttribs();
        }

        m_program.bind();
        gl->glDisable(GL_BLEND);

        QMatrix4x4 m = m_matrix;
        m.translate(0, m_rect.height() / 2);
        m.scale(20, 200);
        m = (*state->projectionMatrix()) * m;

        m_program.setUniformValue("matrix", m);

        gl->glDrawArrays(GL_LINE_STRIP, 0, m_allocated);

        m_vao.release();
    }

    void setRect(const QRectF &r)
    {
        m_rect = r;
    }

    const bool m_doubleSupported;
    int m_vxLocation;
    int m_vyLocation;
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_buffer;
    QOpenGLVertexArrayObject m_vao;
    size_t m_allocated = 0;
    QRectF m_rect;
    QMatrix4x4 m_matrix;
};

QSGNode *XYPlot::sgNode()
{
    if (!m_node) {
        m_node = new Node;
    }
    return m_node;
}

void XYPlot::update(double w, double h)
{
    m_node->setRect({ 0, 0, w, h });
    if (m_needsUpdate) {
        m_needsUpdate = false;

        m_node->updateData(dataSet());
        m_node->markDirty(QSGNode::DirtyGeometry);
    }
}

}
