#version 330 core
out vec4 FragColor;
in float height;
in vec2 TexCoord;

uniform sampler2D terrainTexture;
uniform sampler2D detailTexture;
uniform float repeatDetailMap;

void main() {

    vec4 baseColor = texture(terrainTexture, TexCoord);
    float detailValue = texture(detailTexture, TexCoord * repeatDetailMap).r;

    // Use the detail map to modulate the base texture
    vec4 blendedColor = baseColor * (1.0 + detailValue - 0.5);

    FragColor = blendedColor;
    
    
}
