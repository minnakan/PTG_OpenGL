#version 330 core
out vec4 FragColor;
in vec3 ourColor2;


void main()
{
   FragColor = vec4(ourColor2,1.0);
};

