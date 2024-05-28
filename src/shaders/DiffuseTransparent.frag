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
  const float diffuse = clamp(dot(normal, -vec3(ubo.x, ubo.y, ubo.z)), 0.0, 1.0);

  const vec3 ambient = vec3(0.07, 0.05, 0.1);

  outColor = vec4(ambient + color.xyz * diffuse, color.w);
}