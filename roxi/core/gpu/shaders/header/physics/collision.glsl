struct Collision {
  vec3 contact_normal;
  float restitution;

  uint obj_a;
  uint obj_b;
  uint _pad0;
  uint _pad1;
};
