#version 330 core
out vec4 FragColor;
in float height;
in vec2 TexCoord;

uniform sampler2D terrainTexture;

void main() {
    // Normalize height to a 0-1 range (adjust these values based on your terrain scale)
    float normalizedHeight = (height + 5.0) / 10.0;

    // Map height to a grayscale color
    vec3 grayColor = vec3(normalizedHeight);

    FragColor = texture(terrainTexture, TexCoord);
}
