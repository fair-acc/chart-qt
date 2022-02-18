# Pipegen

Pipegen is a code generation tool that creates C++ pipeline classes by reflecting on a set
of shaders and some additional information contained in a json file:

    {
        "className": "MyPipeline",
        "vertex": "shaders/shader.vert",
        "vertexInputs": [
            {
                "name": "vertexPos",
                "locations": [ 0, 1 ]
            }
        ],
        "fragment": "shaders/shader.frag"
    }

When using this json pipegen will generate a C++ class called 'MyPipeline' that uses the
specified shaders. Additionally the json specifies the pipeline uses one vertex buffer
that feeds data at locations 0 and 1.
