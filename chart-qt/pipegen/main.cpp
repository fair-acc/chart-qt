
#include <unordered_map>

#include <private/qrhi_p.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

static QString type(const QShaderDescription::VariableType &type) {
    switch (type) {
    case QShaderDescription::Unknown:
        qCritical("Unknown type");
        exit(EXIT_FAILURE);
        break;

    case QShaderDescription::Float: return "float";
    case QShaderDescription::Vec2: return "QVector2D";
    case QShaderDescription::Vec3: return "QVector3D";
    case QShaderDescription::Vec4: return "QVector4D";
    case QShaderDescription::Mat4: return "std::array<float, 16>"; // Cannot use QMatrix4x4 here, its size is 68 instead of 64
    case QShaderDescription::Int: return "int32_t";
    case QShaderDescription::Uint: return "uint32_t";
    case QShaderDescription::Bool: return "bool";
    default:
        break;
    }
    qCritical("Unhandled variable type");
    exit(EXIT_FAILURE);
    return {};
}

static QString includeForType(const QShaderDescription::VariableType &type) {
    switch (type) {
    case QShaderDescription::Unknown:
        qCritical("Unknown type");
        exit(EXIT_FAILURE);
        break;

    case QShaderDescription::Float: return {};
    case QShaderDescription::Vec2: return "QVector2D";
    case QShaderDescription::Vec3: return "QVector3D";
    case QShaderDescription::Vec4: return "QVector4D";
    case QShaderDescription::Mat4: return "array";
    case QShaderDescription::Int: return "stdint.h";
    case QShaderDescription::Uint: return "stdint.h";
    case QShaderDescription::Bool: return {};
    default:
        break;
    }
    qCritical("Unhandled variable type");
    exit(EXIT_FAILURE);
    return {};
}

static int typeStride(const QShaderDescription::VariableType &type) {
    switch (type) {
    case QShaderDescription::Unknown:
        qCritical("Unknown type");
        exit(EXIT_FAILURE);
        break;

    case QShaderDescription::Float: return sizeof(float);
    case QShaderDescription::Vec2: return sizeof(float) * 2;
    case QShaderDescription::Vec3: return sizeof(float) * 3;
    case QShaderDescription::Vec4: return sizeof(float) * 4;
    case QShaderDescription::Mat4: return sizeof(float) * 16;
    case QShaderDescription::Int: return 4;
    case QShaderDescription::Uint: return 4;
    case QShaderDescription::Bool: return 1;
    default:
        break;
    }
    qCritical("Unhandled variable type");
    exit(EXIT_FAILURE);
    return 0;
}

static QString typeFormat(const QShaderDescription::VariableType &type) {
    switch (type) {
    case QShaderDescription::Unknown:
        qCritical("Unknown type");
        exit(EXIT_FAILURE);
        break;

    case QShaderDescription::Float: return "Float";
    case QShaderDescription::Vec2: return "Float2";
    case QShaderDescription::Vec3: return "Float3";
    case QShaderDescription::Vec4: return "Float4";
    case QShaderDescription::Mat4: return "Mat4";
    case QShaderDescription::Int: return "Int";
    case QShaderDescription::Uint: return "Uint";
    case QShaderDescription::Bool: return "Bool";
    default:
        break;
    }
    qCritical("Unhandled variable type");
    exit(EXIT_FAILURE);
    return {};
}

QString camelCase(QString name) {
    name[0] = name[0].toUpper();
    return name;
}

struct Shader {
    enum Stage {
        Vertex   = 1,
        Fragment = 2,
    };
    Stage   stage;
    QString source;
};

QString shaderStages(int stages) {
    QString out{};
    bool    nostage = true;
    if (stages & Shader::Stage::Vertex) {
        out += "chart_qt::Pipeline::ShaderStage::Vertex";
        nostage = false;
    }
    if (stages & Shader::Stage::Fragment) {
        if (!nostage) {
            out += " | ";
        }
        out += "chart_qt::Pipeline::ShaderStage::Fragment";
    }
    return out;
}

static const char *disclaimer = "/* WARNING WARNING WARNING WARNING WARNING\n"
                                "   This file was autogenerated. If you modify it your\n"
                                "   changes will get overwritten.\n"
                                "   Modify instead the source pipeline json file or the shaders. */\n\n";

int                main(int argc, char **argv) {
    QCoreApplication   app(argc, argv);

    QCommandLineParser cmdParser;
    cmdParser.addHelpOption();

    QCommandLineOption shadersDirOption("s", "Shaders path", "Path of built shaders", ".");
    cmdParser.addOption(shadersDirOption);

    cmdParser.addPositionalArgument("<input file>", "Source file");

    cmdParser.process(app.arguments());

    QString shadersPath = cmdParser.value(shadersDirOption) + "/";

    if (cmdParser.positionalArguments().size() != 1) {
        cmdParser.showHelp();
        exit(EXIT_FAILURE);
    }

    const auto inputFile = cmdParser.positionalArguments().first();

    QFile      pipelineFile(inputFile);
    if (!pipelineFile.open(QIODevice::ReadOnly)) {
        qCritical("Cannot open '%s' for reading.", qPrintable(inputFile));
        exit(EXIT_FAILURE);
    }
    QJsonParseError err;
    auto            doc = QJsonDocument::fromJson(pipelineFile.readAll(), &err);
    if (err.error != QJsonParseError::NoError) {
        qCritical("Failed to parse '%s': %s", qPrintable(pipelineFile.fileName()), qPrintable(err.errorString()));
        exit(EXIT_FAILURE);
    }
    auto                json = doc.object();

    std::vector<Shader> shaders;
    if (auto it = json.find("vertex"); it != json.end()) {
        shaders.push_back({ Shader::Stage::Vertex, it.value().toString() });
    }
    if (auto it = json.find("fragment"); it != json.end()) {
        shaders.push_back({ Shader::Stage::Fragment, it.value().toString() });
    }

    struct Uniform {
        QShaderDescription::UniformBlock block;
        int                              stages = 0;
    };
    std::unordered_map<int, Uniform> uniforms;

    struct Sampler {
        QShaderDescription::InOutVariable var;
        int                               stages = 0;
    };
    std::unordered_map<int, Sampler>               samplers;

    std::vector<QShaderDescription::InOutVariable> inputs;

    for (auto &s : shaders) {
        QString f = shadersPath + s.source + ".qsb";
        QFile   source(f);
        if (!source.open(QIODevice::ReadOnly)) {
            qFatal("Cannot open %s for reading", qPrintable(f));
        }

        QShader    shader = QShader::fromSerialized(source.readAll());

        const auto us     = shader.description().uniformBlocks();
        for (auto &u : us) {
            auto &uniform = uniforms[u.binding];
            uniform.block = u;
            uniform.stages |= s.stage;
        }

        const auto &imageSamplers = shader.description().combinedImageSamplers();
        for (auto &smpl : imageSamplers) {
            auto &sampler = samplers[smpl.binding];
            sampler.var   = smpl;
            sampler.stages |= s.stage;
        }

        if (s.stage == Shader::Stage::Vertex) {
            const auto in = shader.description().inputVariables();
            for (const auto &i : in) {
                if (inputs.size() < i.location + 1) {
                    inputs.resize(i.location + 1);
                }
                inputs[i.location] = i;
            }
        }
    }

    static const auto nameStr      = QStringLiteral("name");
    static const auto locationsStr = QStringLiteral("locations");

    struct InputBinding {
        QString name;
        QString type;
        int     stride;
        int     offset;
    };
    std::vector<InputBinding> vertexInputs;
    std::vector<uint32_t>     vertexInputOffsets;

    const auto                vinputs = json.value("vertexInputs").toArray();
    for (const auto &in : vinputs) {
        QString    ty;
        int        stride    = 0;
        int        offset    = 0;
        const auto locations = in[QStringLiteral("locations")].toArray();
        for (const auto &l : locations) {
            int  loc = l.toInt();
            auto it  = std::find_if(inputs.begin(), inputs.end(), [&](const auto &i) {
                return i.location == loc;
                            });
            if (it == inputs.end()) {
                qCritical("Cannot find input at location %d", loc);
                exit(EXIT_FAILURE);
            }
            ty += type(it->type);
            stride += typeStride(it->type);
            if (vertexInputOffsets.size() <= loc) {
                vertexInputOffsets.resize(loc + 1);
            }
            vertexInputOffsets[loc] = offset;
            offset += stride;
        }

        vertexInputs.push_back(InputBinding{ in[QStringLiteral("name")].toString(), ty, stride });
    }

    QString className = json.value(QStringLiteral("className")).toString();
    QString name      = className.toLower();

    {
        QFile fileOut(name + ".h");
        if (!fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qFatal("Cannot open %s for writing", qPrintable(fileOut.fileName()));
        }
        QTextStream out(&fileOut);

        out << disclaimer;

        QSet<QString> includes;
        for (auto &u : uniforms) {
            for (auto &member : qAsConst(u.second.block.members)) {
                includes.insert(includeForType(member.type));
            }
            for (auto &in : qAsConst(inputs)) {
                if (auto i = includeForType(in.type); !i.isNull()) {
                    includes.insert(i);
                }
            }
        }

        out << "#ifndef __" << className.toUpper() << "__\n";
        out << "#define __" << className.toUpper() << "__\n\n";

        out << "#include <tuple>\n";
        for (auto i : qAsConst(includes)) {
            out << "#include <" << i << ">\n";
        }
        out << "#include \"renderutils.h\"\n\n";

        out << "class " << className << " {\npublic:\n";

        for (auto &u : uniforms) {
            out << "    struct " << camelCase(u.second.block.blockName) << " final {\n";

            out << "        using Layout = chart_qt::DataLayout<";
            const auto s = u.second.block.members.size();
            for (int i = 0; i < s; ++i) {
                const auto &member = u.second.block.members.at(i);
                if (i > 0) {
                    out << ", ";
                }
                out << type(member.type);
            }
            out << ">;\n";

            for (int i = 0; i < s; ++i) {
                const auto &member = u.second.block.members.at(i);
                out << "        " << type(member.type) << " " << member.name << ";\n";
            }
            out << "    };\n";
        }

        out << "    struct Bindings {\n";
        for (auto &u : uniforms) {
            QString name = camelCase(u.second.block.blockName);
            out << "        chart_qt::BufferRef<" << name << "::Layout> " << u.second.block.structName << ";\n";
        }
        for (auto &s : samplers) {
            out << "        const chart_qt::TextureBase &" << s.second.var.name << ";\n";
        }
        out << "    };\n\n";

        out << "    chart_qt::BindingSet createBindingSet(chart_qt::PlotRenderer *renderer, Bindings bindings);\n\n";

        for (const auto &in : vinputs) {
            const QString name  = in[nameStr].toString();
            const QString cname = camelCase(name);
            out << "    struct " << cname << " {\n";
            const auto locations = in[locationsStr].toArray();
            for (const auto &loc : locations) {
                auto it = std::find_if(inputs.begin(), inputs.end(), [&](const auto &i) {
                    return i.location == loc.toInt();
                               });
                if (it == inputs.end()) {
                    qCritical("Cannot find input at location %d", loc.toInt());
                    exit(EXIT_FAILURE);
                }
                out << "        " << type(it->type) << " " << it->name << ";\n";
            }

            out << "    };\n";
            out << "    void set" << cname << "InputBuffer(const chart_qt::Buffer<" << cname << "> &buffer, int offset = 0);\n";
        }

        out << "\n";

        out << "    void setTopology(chart_qt::Pipeline::Topology topology);\n";

        out << "    bool isCreated() const;\n";
        out << "    void create(chart_qt::PlotRenderer *renderer);\n\n";

        out << "    const chart_qt::Pipeline &pipeline() const { return _pipeline; }\n";

        out << "\nprivate:\n";
        out << "    chart_qt::Pipeline _pipeline;\n";

        out << "};\n\n";
        out << "#endif\n";
    }

    {
        QFile fileOut(name + ".cpp");
        if (!fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            qFatal("Cannot open %s for writing", qPrintable(fileOut.fileName()));
        }
        QTextStream out(&fileOut);

        out << disclaimer;

        out << "#include \"" << name << ".h\"\n\n";

        const int numVBindings = vinputs.count();
        for (int i = 0; i < numVBindings; ++i) {
            const auto   &in    = vinputs.at(i);
            const QString name  = in[nameStr].toString();
            const QString cname = camelCase(name);
            out << "void " << className << "::set" << cname << "InputBuffer(const chart_qt::Buffer<" << cname << "> &buffer, int offset)\n";
            out << "{\n";
            out << "    _pipeline.setVertexInputBuffer(" << i << ", buffer, offset);\n";
            out << "}\n\n";
        }

        out << "bool " << className << "::isCreated() const\n";
        out << "{\n    return _pipeline.isCreated();\n}\n\n";

        out << "void " << className << "::setTopology(chart_qt::Pipeline::Topology topology)\n";
        out << "{\n    _pipeline.setTopology(topology);\n}\n\n";

        out << "void " << className << "::create(chart_qt::PlotRenderer *renderer)\n";
        out << "{\n";
        for (int i = 0; i < shaders.size(); ++i) {
            const auto &s = shaders[i];
            out << "    _pipeline.setShader(chart_qt::Pipeline::ShaderStage::";
            switch (s.stage) {
            case Shader::Stage::Vertex:
                out << "Vertex";
                break;
            case Shader::Stage::Fragment:
                out << "Fragment";
                break;
            }
            out << ", \":/" << s.source << ".qsb\");\n";
        };
        out << "\n";

        for (auto &u : uniforms) {
            out << "    _pipeline.addUniformBufferBinding(" << u.second.block.binding << ", " << shaderStages(u.second.stages) << ");\n";
        }
        for (auto &s : samplers) {
            out << "    _pipeline.addSampledTexture(" << s.second.var.binding << ", " << shaderStages(s.second.stages) << ");\n";
        }
        out << "\n";

        for (int i = 0; i < numVBindings; ++i) {
            const auto &in        = vinputs.at(i);
            const auto  locations = in[locationsStr].toArray();
            int         stride    = vertexInputs[i].stride;
            for (const auto &l : locations) {
                int loc = l.toInt();
                out << "    _pipeline.addVertexInput(" << i << ", " << loc << ", chart_qt::Pipeline::VertexInputFormat::" << typeFormat(inputs[loc].type) << ", " << vertexInputOffsets[loc] << ", " << stride << ");\n";
            }
        }
        out << "\n    _pipeline.create(renderer);\n";

        out << "}\n\n";

        out << "chart_qt::BindingSet " << className << "::createBindingSet(chart_qt::PlotRenderer *renderer, Bindings bindings)\n";
        out << "{\n";
        out << "    auto set = renderer->createBindingSet();\n";
        for (auto &u : uniforms) {
            out << "    set.uniformBuffer(" << u.second.block.binding << ", ";
            out << shaderStages(u.second.stages) << ", bindings." << u.second.block.structName << ".ref);\n";
        }
        for (auto &s : samplers) {
            out << "    set.sampledTexture(" << s.second.var.binding << ", ";
            out << shaderStages(s.second.stages) << ", bindings." << s.second.var.name << ");\n";
        }
        out << "    set.create();\n";
        out << "    return set;\n";
        out << "}\n\n";
    }
}
