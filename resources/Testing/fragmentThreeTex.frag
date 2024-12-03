#version 330 core
out vec4 FragColor;

in vec3 ourColor2;
in vec2 TexCoord;

uniform float mixValue;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	//FragColor = texture(texture1, TexCoord) * vec4(ourColor2, 1.0);
   FragColor = mix(texture(texture1,TexCoord),texture(texture2,vec2(1-TexCoord.x,TexCoord.y)),1);
};

