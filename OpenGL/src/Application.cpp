#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

// includes for parsing the shader files
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak()
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

struct ShaderProgramSource
{
    std::string vertexSource;
    std::string fragmentSource;
};

static ShaderProgramSource ParseShader(const std::string filePath)
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

// The function compiles the shader and returns its object id
static unsigned int CompileShader(unsigned int type, const std::string& source)
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
        char* message = (char*) alloca(length * sizeof(char)); // Dynamic allocation on heap memory
        glGetShaderInfoLog(id, length, &length, message);  // Write error to log
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);  // delete failed shader
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
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
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "): " << function 
            << " " << file << ": " << line << std::endl;
        return false;
    }
    return true;
}

// Input processing
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // After pressing the Escape key, window will close
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(2);  // sets the swap interval for current OpenGL context. Used to smoothly animate color of rectangle

    GLenum err = glewInit();  // glewInit() must be called after creating valid OpenGL rendering context as above
    if (err != GLEW_OK)
        std::cout << "Error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] = {
        -0.5f,  -0.5f, // 0
         0.5f,  -0.5f, // 1
         0.5f,   0.5f, // 2    
        -0.5f,   0.5f  // 3
    };

    // Index buffer
    unsigned int indices[] = {
        0, 1, 2,  // Indices of positions to create first triangle
        2, 3, 0   // Indices of positions to create second triangle
    };

    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    // Generating and binding index buffer
    unsigned int ibo;  // ibo = index buffer object
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

    std::cout << "...VERTEX SHADER CODE..." << std::endl;
    std::cout << source.vertexSource << std::endl;
    std::cout << "...FRAGMENT SHADER CODE..." << std::endl;
    std::cout << source.fragmentSource << std::endl;

    unsigned int shader = CreateShader(source.vertexSource, source.fragmentSource);

    // Use the program for rendering
    GLCall(glUseProgram(shader));

    GLCall(int location = glGetUniformLocation(shader, "u_Color"));  // retrieve the location
    ASSERT(location != -1);  // if uniform variable not found then location would be -1
    GLCall(glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f));

    /* NOTE: glUniform4f() must be called before glDrawArray() call */

    float r = 0.0f;
    float increment = 0.05f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // Process the input
        processInput(window);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;

        r += increment;

        /* Swap front and back buffers */
        glfwSwapBuffers(window); // To make rendered frame visible

        /* Poll for and process events */
        glfwPollEvents();
    }

    // Cleanup
    glDeleteProgram(shader);

    glfwTerminate(); // To properly clean/delete all of GLFW's resources that were allocated
    return 0;
}