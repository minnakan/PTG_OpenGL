#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;


out vec3 Normal;
out vec3 FragPos;
out vec3 ViewPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 viewPos;

void main()
{   
    FragPos = vec3(model * vec4(aPos,1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    ViewPos = viewPos;
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}