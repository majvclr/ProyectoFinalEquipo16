#version 330

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec4 vColor;

out vec4 color;

const int MAX_POINT_LIGHTS = 5;
const int MAX_SPOT_LIGHTS  = 8;

struct Light {
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
};

struct DirectionalLight {
    Light base;
    vec3 direction;
};

struct Material {
    float specularIntensity;
    float shininess;
};

uniform DirectionalLight directionalLight;
uniform sampler2D theTexture;
uniform Material material;
uniform vec3 eyePosition;

vec4 CalcLightByDirection(Light light, vec3 direction)
{
    vec4 ambientcolor = vec4(light.color, 1.0f) * light.ambientIntensity;
    
    float diffuseFactor = max(dot(normalize(Normal), normalize(direction)), 0.0f);
    vec4 diffusecolor = vec4(light.color * light.diffuseIntensity * diffuseFactor, 1.0f);
    
    vec4 specularcolor = vec4(0, 0, 0, 0);
    if(diffuseFactor > 0.0f)
    {
        vec3 fragToEye = normalize(eyePosition - FragPos);
        vec3 reflectedVertex = normalize(reflect(direction, normalize(Normal)));
        float specularFactor = dot(fragToEye, reflectedVertex);
        if(specularFactor > 0.0f)
        {
            specularFactor = pow(specularFactor, material.shininess);
            specularcolor = vec4(light.color * material.specularIntensity * specularFactor, 1.0f);
        }
    }
    return (ambientcolor + diffusecolor + specularcolor);
}

void main()
{
    vec4 tex = texture(theTexture, TexCoord);
    
    if (tex.a < 0.1)
        discard;

    vec4 finalcolor = CalcLightByDirection(directionalLight.base, directionalLight.direction);
    
    // Multiplicación final para aplicar luz y color base a la textura
    color = tex * vColor * finalcolor;
}