// from HelloVulkan
struct CameraUBO
{
	mat4 projection;
	mat4 view;
	vec4 position;
	float cameraNear;
	float cameraFar;
};
