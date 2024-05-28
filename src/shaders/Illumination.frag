layout(binding = 2) uniform Ubo { 
	float time; 
	float x; 
	float y; 
	float z; 
} ubo;


layout(location = 0) in vec3 normal;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 outColor;

void main()
{
    // float diffuse = max(dot(normal, vec3(ubo.x, ubo.y, ubo.z)), 0.0);
    // vec3 resultColor = color.xyz * diffuse;
    // outColor = vec4(resultColor, 1.0);

    // Calculate the distance from the fragment to the center of the screen
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 screenSize = vec2(800.0, 600.0); // Adjust according to your screen resolution
    vec2 center = screenSize * 0.5;
    float distanceToCenter = distance(fragCoord, center);

    // Calculate the strength of the glow based on the distance from the center of the screen
    float glowStrength = smoothstep(800.0, 600.0, distanceToCenter); // Adjust parameters as needed

    // Yellow glowing color
    vec3 glowingColor = vec3(10.0, 10.0, 0.0); // Yellow

    // Apply glow effect by blending the glowing color with the background color
    vec3 finalColor = mix(glowingColor, vec3(0.0), glowStrength*10); // Blend with black background
    outColor = vec4(finalColor, 1.0); // Apply object's color
}