#include "Shader.h"

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include "Renderer.h"


Shader::Shader(const std::string& filePath) :
	m_FilePath(filePath), m_RendererID(0) 
{
    ShaderProgramSource source = ParseShader(filePath);
    m_RendererID = CreateShader(source.vertexSource, source.fragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_RendererID));
}

ShaderProgramSource Shader::ParseShader(const std::string filePath)
{
    std::ifstream stream(filePath);

    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];  // to store vertex and fragment shader src
    ShaderType shaderType = ShaderType::NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                shaderType = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                shaderType = ShaderType::FRAGMENT;
        }
        else
        {
            ss[int(shaderType)] << line << std::endl;
        }
    }

    return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);  // shader created
    const char* src = source.c_str();  // c_str() returns the pointer, source needs to be non-null value

    // 1. Set Shader source
    glShaderSource(id, 1, &src, nullptr);

    // 2. Compile the shader
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success); // iv = integer vector
    if (success == GL_FALSE)
    {
        // compile error for shader
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char)); // Dynamic allocation on heap memory
        glGetShaderInfoLog(id, length, &length, message);  // Write error to log
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);  // delete failed shader
        return 0;
    }

    return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    /* program = program ID
    *  vs = vertex shader ID
    *  fs = fragment shader ID
    */
    unsigned int program = glCreateProgram(); // program object to link multiple shader later in the pipeline

    // create shader objects
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    // Attach shaders to a program created above
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    // Link the program
    glLinkProgram(program);

    // Validate the program
    glValidateProgram(program);

    // Delete the shader
    // We don't need the shader objects after linking them with the program object, we no longer need them anymore
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::Bind() const
{
    GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform4f(const std::string& uniformName, float v0, float v1, float v2, float v3)
{
    GLCall(glUniform4f(GetUniformLocation(uniformName), v0, v1, v2, v3));
}

int Shader::GetUniformLocation(const std::string& uniformName)
{
    // To prevent unnecessary calculating uniform location each time and only allow to calculate uniform variable location at first time
    if (m_UniformLocationCache.find(uniformName) != m_UniformLocationCache.end())
        return m_UniformLocationCache[uniformName];

    GLCall(int location = glGetUniformLocation(m_RendererID, uniformName.c_str()));  // retrieve the location
    if (location == -1)
        std::cout << "Uniform name: " << uniformName << " doesn't exist!" << "\n";

    m_UniformLocationCache[uniformName] = location;
    return location;
}