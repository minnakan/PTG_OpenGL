#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec3 ViewPos;

in vec2 TexCoords;
  
uniform vec3 lightColor;
uniform vec3 lightPos;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 
  
uniform Material material;

struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;  

void main()
{
 
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    

    //Diffuse lighting calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse,TexCoords));

    //Specular lighting calculations
    vec3 viewDir = normalize(ViewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir,norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular =  light.specular* spec * texture(material.specular,TexCoords).rgb; 

    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}