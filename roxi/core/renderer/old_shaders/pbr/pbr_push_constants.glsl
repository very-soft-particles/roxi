struct PBRPushConstant
{
	float light_intensity;
	float base_reflectivity;
	float max_reflection_lod;
	float light_falloff; // Small --> slower falloff, Big --> faster falloff
	float albedo_multipler; // Show albedo color if the scene is too dark, default value should be zero
};
