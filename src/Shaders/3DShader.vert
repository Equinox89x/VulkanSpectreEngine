#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push
{
	mat4 transform;
	mat4 projectionTransform;
	mat4 objectTransform;
	vec3 color;
} push;

const vec3 lightDirection = normalize(vec3(0.5, 0.5, 1.0));
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

void main()
{
	gl_Position = push.transform * vec4(position, 1.0);

	float brightness = max(dot(normalize(position), lightDirection), 0.0);
    vec3 lighting = lightColor * brightness;
	fragColor = color * lighting;
}