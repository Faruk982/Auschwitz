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
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4
#define NR_SPOT_LIGHTS 4

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLights[NR_SPOT_LIGHTS];
uniform vec3 objectColor;
uniform float emissionStrength;
uniform float shininess;
uniform bool emissiveOnly;

// Texture uniforms
uniform sampler2D texture1;
uniform bool useTexture;
uniform int textureMode;           // 0 = texture only, 1 = blend
uniform float textureTiling;
uniform float textureBlendFactor;
uniform int textureDisplayMode;    // 0=without texture, 1=with texture, 2=blended texture

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
    vec3 texColor = texture(texture1, TexCoord * textureTiling).rgb;

    if (emissiveOnly) {
        surfaceColor = objectColor;
    } else if (textureDisplayMode == 0) {
        // without texture
        surfaceColor = objectColor;
    } else if (textureDisplayMode == 1) {
        // with texture
        surfaceColor = useTexture ? texColor : objectColor;
    } else {
        // blended texture
        surfaceColor = useTexture ? mix(objectColor, texColor, textureBlendFactor) : objectColor;
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
    
    // 3. Spot lights
    if(enableSpotLight) {
        for(int i = 0; i < NR_SPOT_LIGHTS; i++)
            result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir, surfaceColor);
    }
    
    // Emission
    vec3 emission = surfaceColor * emissionStrength;
    if (emissiveOnly) {
        vec3 emissiveColor = enableSpotLight ? emission : vec3(0.0);
        FragColor = vec4(emissiveColor, 1.0);
        return;
    }
    result += emission;

    FragColor = vec4(result, 1.0);
}

// Directional light
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 surfaceColor)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    
    vec3 ambient  = light.ambient  * surfaceColor;
    vec3 diffuse  = light.diffuse  * diff * surfaceColor;
    vec3 specular = light.specular * spec * vec3(0.3);
    
    vec3 finalColor = vec3(0.0);
    if(enableAmbient) finalColor += ambient;
    if(enableDiffuse) finalColor += diffuse;
    if(enableSpecular) finalColor += specular;
    
    return finalColor;
}

// Point light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 surfaceColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    
    vec3 ambient  = light.ambient  * surfaceColor;
    vec3 diffuse  = light.diffuse  * diff * surfaceColor;
    vec3 specular = light.specular * spec * vec3(0.3);
    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    vec3 finalColor = vec3(0.0);
    if(enableAmbient) finalColor += ambient;
    if(enableDiffuse) finalColor += diffuse;
    if(enableSpecular) finalColor += specular;
    return finalColor;
}

// Spot light
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 surfaceColor)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    float theta = dot(lightDir, normalize(-light.direction));
    float intensity = theta > light.cutOff ? 1.0 : 0.0;

    vec3 ambient  = light.ambient  * surfaceColor;
    vec3 diffuse  = light.diffuse  * diff * surfaceColor;
    vec3 specular = light.specular * spec * vec3(0.3);

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    vec3 finalColor = vec3(0.0);
    if(enableAmbient) finalColor += ambient;
    if(enableDiffuse) finalColor += diffuse;
    if(enableSpecular) finalColor += specular;
    return finalColor;
}
