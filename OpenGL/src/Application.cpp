//#include <GL/glew.h>
#include "Renderer.h"
#include <GLFW/glfw3.h>

#include <iostream>

// includes for parsing the shader files
#include <fstream>
#include <string>
#include <sstream>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBufferLayout.h"

#include "Shader.h"

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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Learning OpenGL", NULL, NULL);
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
    {  // Scope is added to terminate application successfully
        // If scope was not there then glfwTerminate() would delete the context and we would be stuck in while loop of glGetError()
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

        // Vertex Array Object - Need to explicitly define if we are using opengl core profile
        VertexArray va;
        VertexBuffer vb(positions, 4 * 2 * sizeof(float));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        // Note: glVertexAttribPointer() automatically binds the vertex buffer with vertex array

        // Generating and binding index buffer
        IndexBuffer ib(indices, 6);

        Shader shader("res/shaders/Basic.shader");

        // Use the program for rendering
        shader.Bind();

        shader.SetUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);

        /* NOTE: glUniform4f() must be called before glDrawArray() call */

        // Unbind everything
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        float r = 0.0f;
        float increment = 0.05f;

        Renderer renderer;

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            // Process the input
            processInput(window);

            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT);

            shader.Bind();
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

            renderer.Draw(va, ib, shader);

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
    }

    glfwTerminate(); // To properly clean/delete all of GLFW's resources that were allocated
    return 0;
}