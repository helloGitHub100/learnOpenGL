#version 330 core
out vec4 FragColor;

in float lineTexCoords;
uniform vec2 sphereTexCoords;

uniform sampler2D roughnessMap;
uniform int SAMPLE_COUNT = 1024;

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
// efficient VanDerCorpus calculation.
float RadicalInverse_VdC(uint bits) 
{
     bits = (bits << 16u) | (bits >> 16u);
     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
     return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}
// ----------------------------------------------------------------------------
vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
	float a = roughness*roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
	vec3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);
	
	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}
// ----------------------------------------------------------------------------
void main()
{		
    float xSegment = sphereTexCoords.x;
    float ySegment = sphereTexCoords.y;
    float xPos = cos(xSegment * 2.0f * PI) * sin(ySegment * PI);
    float yPos = cos(ySegment * PI);
    float zPos = sin(xSegment * 2.0f * PI) * sin(ySegment * PI);
    vec3 WorldPos = vec3(xPos, yPos, zPos);
    vec3 N = normalize(WorldPos);
    
    // make the simplifying assumption that V equals R equals the normal 
    vec3 R = N;
    vec3 V = R;
    float roughness = texture(roughnessMap, sphereTexCoords).r;
    // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
    uint i = uint(clamp(lineTexCoords * SAMPLE_COUNT, 0, SAMPLE_COUNT));
    vec2 Xi = Hammersley(i, uint(SAMPLE_COUNT));
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);
    vec3 L  = normalize(2.0 * dot(V, H) * H - V);

    FragColor = vec4(L, 1.0);
}
