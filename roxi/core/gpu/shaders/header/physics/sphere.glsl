struct Sphere {
  vec3 center;
  float radius;
};


bool detect_sphere_distance(Sphere sphere_a, Sphere sphere_b) {

  vec3 difference_vec = sphere_a.center - sphere_b.center;
  float difference_length = dot(difference_vec, difference_vec);
  float radii_sum = sphere_a.radius + sphere_b.radius;
  return difference_length < radii_sum;
}

bool get_sphere_distance(Sphere sphere_a, Sphere sphere_b) {

  vec3 difference_vec = sphere_a.center - sphere_b.center;
  float difference_length = dot(difference_vec, difference_vec);
  float radii_sum = sphere_a.radius + sphere_b.radius;
  return difference_length - radii_sum;
}
