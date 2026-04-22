#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoord;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 3

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform vec3 objectColor;
uniform float emissionStrength;

// Texture uniforms
uniform sampler2D texture1;
uniform bool useTexture;
uniform int textureMode;           // 0 = texture only, 1 = blend (percentage-based interpolation)
uniform float textureTiling;       // How many times to repeat the texture
uniform float textureBlendFactor;  // 0.0 = 100% objectColor, 1.0 = 100% texture (for mode 1)

// Toggles
uniform bool enableDirLight;
uniform bool enablePointLight;
uniform bool enableSpotLight;
uniform bool enableAmbient;
uniform bool enableDiffuse;
uniform bool enableSpecular;

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 surfaceColor);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 surfaceColor);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 surfaceColor);

void main()
{
    // Determine the surface color based on texture mode
    vec3 surfaceColor;
    if (useTexture) {
        vec3 texColor = texture(texture1, TexCoord * textureTiling).rgb;
        if (textureMode == 0) {
            // Mode 0: Texture only (100% texture)
            surfaceColor = texColor;
        } else {
            // Mode 1: Linear interpolation (percentage-based blending)
            // mix(a, b, t) = (1-t)*a + t*b
            // textureBlendFactor = 0.0 -> 100% objectColor, 0% texture
            // textureBlendFactor = 0.5 -> 50% objectColor, 50% texture
            // textureBlendFactor = 1.0 -> 0% objectColor, 100% texture
            surfaceColor = mix(objectColor, texColor, textureBlendFactor);
        }
    } else {
        surfaceColor = objectColor;
    }

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0);

    // 1. Directional lighting
    if(enableDirLight)
        result += CalcDirLight(dirLight, norm, viewDir, surfaceColor);
    
    // 2. Point lights
    if(enablePointLight) {
        for(int i = 0; i < NR_POINT_LIGHTS; i++)
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, surfaceColor);
    }
    
    // 3. Spot light
    if(enableSpotLight)
        result += CalcSpotLight(spotLight, norm, FragPos, viewDir, surfaceColor);    
    
    // Emission (for the bulb objects)
    vec3 emission = surfaceColor * emissionStrength;
    result += emission;

    FragColor = vec4(result, 1.0);
}

// Calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 surfaceColor)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    vec3 ambient  = light.ambient  * surfaceColor;
    vec3 diffuse  = light.diffuse  * diff * surfaceColor;
    vec3 specular = light.specular * spec * surfaceColor;
    
    vec3 finalColor = vec3(0.0);
    if(enableAmbient) finalColor += ambient;
    if(enableDiffuse) finalColor += diffuse;
    if(enableSpecular) finalColor += specular;
    
    return finalColor;
}

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 surfaceColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    vec3 ambient  = light.ambient  * surfaceColor;
    vec3 diffuse  = light.diffuse  * diff * surfaceColor;
    vec3 specular = light.specular * spec * surfaceColor;
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    vec3 finalColor = vec3(0.0);
    if(enableAmbient) finalColor += ambient;
    if(enableDiffuse) finalColor += diffuse;
    if(enableSpecular) finalColor += specular;
    return finalColor;
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 surfaceColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    vec3 ambient  = light.ambient  * surfaceColor;
    vec3 diffuse  = light.diffuse  * diff * surfaceColor;
    vec3 specular = light.specular * spec * surfaceColor;
    
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    
    vec3 finalColor = vec3(0.0);
    if(enableAmbient) finalColor += ambient;
    if(enableDiffuse) finalColor += diffuse;
    if(enableSpecular) finalColor += specular;
    return finalColor;
}
