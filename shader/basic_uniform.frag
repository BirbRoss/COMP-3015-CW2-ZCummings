#version 460

const float PI = 3.14159265358979323846;

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;

layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D brickTex;
layout (binding = 1) uniform sampler2D hayTex;
layout (binding = 2) uniform sampler2D noiseTex;

//Light | Creates a structure, like a script's public variable in unity, that can be called per light
uniform struct LightInfo {
    vec4 lPosition;
    vec3 L;

    //bphong variables
    vec3 La;    //Ambient
} lights[3];

//Material | Ambient light bounces, Diffuse light bounces, Specular light bounces and Specular shininess (like blender roughness)
uniform struct Materialinfo {
    float Rough;
    bool Metal;
    vec3 Colour;
    bool applyBrickHayTex;
    bool applyNoiseTex;
    //mat4 Slice; || Removed due to visual glitchiness

    //bphong variables
    vec3 Ka;    //Ambient
    vec3 Kd;    //Diffuse
    vec3 Ks;    //Specular
    float Shininess;
} Material;

//Fog | Fog's outer radius | Fog's inner radius | Fog's colour
uniform struct FogInfo {
    float maxDist;
    float minDist;
    vec3 fColour;
}Fog;

uniform bool shaderToggle;

//R | Normal distribution function | Calculates what microfacets are aligned on a surface, the higher their alignment with the camera, in this case the half vector, the smoother a surface is.
float ggxDist(float nDotH) {
    float a2 = pow(Material.Rough, 4.0);
    float d = (nDotH * nDotH) * (a2  - 1) + 1;
    return a2 / (PI * d * d);
}

//D | Geometry function | Calculates if any microfacet is large enough to shadow the ones around it
float geomSmith(float dotProd) {
    //Direct lighting
    float k = ((Material.Rough + 1.0) * (Material.Rough + 1.0)) / 8.0;

    float denom = dotProd * (1 - k) + k;
    return 1.0 / denom;
}

//F | Frensel function | Calculates what light is reflected vs refracted based on the camera's angle and will change the reflect/refract if the metalic boolean is set
//Also takes into account the possible textures
vec3 schlickFresnel(float lDotH, vec3 tColour, vec3 nTexColour) {
    //0.04 is the range for dielectrics/metalic materials.
    vec3 f0 = vec3(0.04);

    //If it is metalic it uses the right colour to calculate it.
        if (Material.applyBrickHayTex) {
            f0 = mix(f0, tColour, Material.Metal);
        }
        else if (Material.applyNoiseTex)
        {
            f0 = mix(f0, nTexColour, Material.Metal);
        }
        else {
            f0 = mix(f0, Material.Colour, Material.Metal);
        }

    
    return f0 + (1.0 - f0) * pow(1.0 - lDotH, 5);
}

vec3 PBRModel(int light, vec3 position, vec3 n)
{
    //Extracts the texture, in this case just colour from the bricks and the colour + alpha of the hay
    vec3 brickColour = texture(brickTex, TexCoord).rgb;
    vec4 hayColour = texture(hayTex, TexCoord);
    vec3 brickTexColour = mix(brickColour, hayColour.rgb, hayColour.a);

    //Takes a slice from the noise texture to create the wood texture | Removed due to visual glitches
    //vec4 cyl = Material.Slice * vec4(TexCoord.st, 0.0, 1.0);
    //float dist = length(cyl.xz);

    //We'll perturb the distance to the noise texture to get a value we can plug into t to place a location within the noise texture to a colour value.
    //vec4 noise = texture(noiseTex, TexCoord);
    //dist += noise.b * 2.5;

    //Takes the absolute of distances fraction (the part behind the decimal point e.g. .01) | t is then smoothstepped between 0.2 and 0.5 using itself
    //float t = 1.0 - abs(fract(dist) * 2.0 - 1.0);
    //t = smoothstep(0.2, 0.5, t);

    //The above can be renabled to reactivate the sliced noise map, please disable the perlin noisemap first


    //Uses perlin noise to add some variation ot the barn's texture
    vec4 noise = texture(noiseTex, TexCoord);
    float t = (cos(noise.a * PI) + 1.0) / 2.0;
    vec3 noiseTexColour = mix(vec3(0.3, 0.15, 0.0), vec3(0.5, 0.35, 0.05), t);


    //Grabs the colour for the diffuse, this does mean metalic materials lack diffuse.
    vec3 diffBRDF = vec3(0.0);
    if (!Material.Metal) {
        if (Material.applyBrickHayTex) {
            diffBRDF =  brickTexColour;
        }
        else if (Material.applyNoiseTex)
        {
            diffBRDF = noiseTexColour;
        }
        else {
            diffBRDF = Material.Colour;
        }
    }

    //Probable source of moving lights bug, cause unknown as this should only be reading the positions and not changing them, the position should always remain relative to the vertex/fragment being rendered
    vec3 l = vec3(0.0);
    vec3 lightI = lights[light].L;
    l = lights[light].lPosition.xyz - position;
    float lDist = length(l);
    lightI /= (lDist * lDist);

    //Normalizes the direction of the point lights as well as the vector and half vector.
    l = normalize(l);
    vec3 v = normalize(-position);
    vec3 h = normalize(v + l);

    float nDotH = max(dot(n, h), 0.0);
    float lDotH = dot(l, h);

    //does this need max? no?
    float nDotL = max(dot(n, l), 0.0);
    float nDotV = dot(n, v);

    //0.25 * microfacet alignment * reflect vs refracted light * the microfacet obscuring geography for lighting & view
    //Would add 0.00001 but does not fix black pixels problem
    vec3 specBRDF = 0.25 * ggxDist(nDotH) * schlickFresnel(lDotH, brickTexColour, noiseTexColour) * (geomSmith(nDotL) * geomSmith(nDotV));

    return (diffBRDF + PI * specBRDF) * lightI * nDotL;
}

vec3 bPhongModel(int light, vec3 position, vec3 n)
{
    //Extracts the texture, in this case just colour from the bricks and the colour + alpha of the hay
    vec3 brickColour = texture(brickTex, TexCoord).rgb;
    vec4 hayColour = texture(hayTex, TexCoord);
    vec3 texColour = mix(brickColour, hayColour.rgb, hayColour.a);

     vec4 noise = texture(noiseTex, TexCoord);
    float t = (cos(noise.a * PI) + 1.0) / 2.0;
    vec3 noiseTexColour = mix(vec3(0.3, 0.15, 0.0), vec3(0.5, 0.35, 0.05), t);

    //Calculates diffuse here, it has to be here to establish the variables
    vec3 s = normalize(vec3(lights[light].lPosition.xyz - position)); //find out s vector
    float sDotN = max(dot(s, n), 0.0); //calculate dot product between s & n
    vec3 diffuse;
    vec3 ambient;

    //Calculates ambient lighting based on the material's values and if a texture needs to be applied
    if (Material.applyBrickHayTex) {
        ambient = lights[light].La * texColour;
        diffuse = texColour * sDotN;
    }
    else if (Material.applyNoiseTex) {
        ambient = lights[light].La * noiseTexColour;
        diffuse = noiseTexColour * sDotN;
    }
    else {
        ambient = lights[light].La * Material.Ka;
        diffuse = Material.Kd * sDotN;
    }

    //calculate specular here, takes the view position rather than the pixel's view/reflection which is just more efficient compute wise
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
    {
        vec3 v = normalize(-position.xyz);
        vec3 h = normalize(v + s);
        spec = Material.Ks * pow( max( dot(h,n), 0.0 ),Material.Shininess);
    }
    return ambient + (lights[light].L / 6.0) * (diffuse + spec);
}

void main() {
    
    //Calculates the distances from zero of position z, and fog fact just runs calculation based on the set distances, as well as clamping the factor between 1 & 0 so it doesn't break when mixed later 
    //It is not a fact about fog, I appologise. One has been provided for you in the markdown
    float fogDist = abs(Position.z);
    float fogFact = (Fog.maxDist - fogDist)/(Fog.maxDist - Fog.minDist);
    fogFact = clamp(fogFact, 0.0, 1.0);

    //Initializing colour then calculating it using the shader model, taking in the light's number, the view's position and the normal
    vec3 Colour = vec3(0.0);
    vec3 n = normalize(Normal);
    for(int i = 0; i < 3; i++) {
        if (shaderToggle) {
            Colour += PBRModel(i, Position.xyz, n);
        }
        else
        {
            Colour += bPhongModel(i, Position.xyz, n);
        } 
    }

    //Calculates the correct colour of the pixels based on fog variables and mixes that value with the original blinn-phong calculation, using the fog factor as the ratio
    Colour = mix(Fog.fColour,Colour,fogFact);

    //Gamma correction, adjusts the image brightness to the monitor's gamma, which is 2.2.
    Colour = pow(Colour, vec3(1.0/2.2));

    FragColor = vec4(Colour, 1.0);
}
