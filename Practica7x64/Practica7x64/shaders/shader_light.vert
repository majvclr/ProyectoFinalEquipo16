
#version 330

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex;
layout (location = 2) in vec3 norm;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 vColor;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 color;

void main()
{
    gl_Position = projection * view * model * vec4(pos, 1.0);
    
    vColor = vec4(color, 1.0f);
    
    // Usamos las coordenadas UV originales del modelo .obj
    TexCoord = tex; 
    
    // Cálculo de normales para iluminación correcta
    Normal = mat3(transpose(inverse(model))) * norm;
    
    FragPos = (model * vec4(pos, 1.0)).xyz;
}