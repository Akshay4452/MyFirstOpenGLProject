#shader vertex
#version 330 core

layout(location = 0) in vec4 aPos;  // position variable has attribute 0
layout(location = 1) in vec4 aColor;  // color variable has attribute 1

out vec4 ourColor;  // output a color to fragment shader

void main()
{
	gl_Position = aPos;
	ourColor = aColor; // set ourColor to the input we got from the vertex data
}

#shader fragment
#version 330 core

out vec4 FragColor;
in vec4 ourColor;

void main()
{
	FragColor = ourColor;
}