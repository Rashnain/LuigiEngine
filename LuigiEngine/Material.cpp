struct Material {
	float ambiant;
	float diffuse;
	float specular;
	int alpha;

	Material() = default;

	explicit Material(float ambiant, float diffuse, float specular, int alpha)
	{
		this->ambiant = ambiant;
		this->diffuse = diffuse;
		this->specular = specular;
		this->alpha = alpha;
	}
};
