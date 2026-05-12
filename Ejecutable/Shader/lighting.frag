#version 330 core

#define MAX_LIGHTS 10

struct Material
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light
{
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform vec3 viewPos;
uniform Material material;

uniform int numLights;
uniform Light lights[MAX_LIGHTS];

uniform sampler2D texture_diffuse;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 texColor = texture(texture_diffuse, TexCoords).rgb;
    
        vec3 result = vec3(0.05) * texColor;

    for(int i = 0; i < numLights; i++)
    {
        // Ambient
        vec3 ambient = lights[i].ambient * material.ambient;

        // Diffuse
        vec3 lightDir = normalize(lights[i].position - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = lights[i].diffuse * diff * material.diffuse;

        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
        vec3 specular = lights[i].specular * spec * material.specular;

        // Atenuación para que no se queme todo blanco
        float distance = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);

        

        result += (ambient + diffuse + specular) * attenuation;    }

    color = vec4(result, 1.0f) * texture(texture_diffuse, TexCoords);
}