//
//  Terrain position
//  

attribute vec2 particle_numbers;

uniform sampler2D heightmap_texture;
uniform float terrain_size;
uniform float terrain_scale;

varying vec4 position;
varying vec3 normal;
varying vec3 norm;

vec3 triangle_normal(vec3 v1, vec3 v2, vec3 v3) {
   vec3 u = v1 - v2;
   vec3 v = v3 - v2;
   return cross(u, v);
}

vec3 calculate_normal() {
   vec4 positions[9];
   
   vec4 heightmap = texture2D(heightmap_texture, particle_numbers);
   position = vec4(terrain_scale*(particle_numbers.x - 0.5), heightmap.r*0.5, terrain_scale*(particle_numbers.y - 0.5), gl_Vertex.w);
   
   vec4 heightmap2 = texture2D(heightmap_texture, particle_numbers + vec2(-(1.0 / terrain_size), 0.0));
   vec4 position2 = vec4(particle_numbers.x - 0.5 - (1.0 / terrain_size), heightmap2.r*0.5, particle_numbers.y - 0.5, gl_Vertex.w);
   
   vec4 heightmap3 = texture2D(heightmap_texture, particle_numbers + vec2(-(1.0 / terrain_size), -(1.0 / terrain_size)));
   vec4 position3 = vec4(particle_numbers.x - 0.5 - (1.0 / terrain_size), heightmap3.r*0.5, particle_numbers.y - 0.5 - (1.0 / terrain_size), gl_Vertex.w);
   
   vec3 v1 = position.xyz;//vec3(0,0,0);
   vec3 v2 = position2.xyz;//vec3(1,0,1);
   vec3 v3 = position3.xyz;//vec3(0,1,0);
   return normalize(triangle_normal(v1, v2, v3));
}

void main(void)
{
   vec4 heightmap = texture2D(heightmap_texture, particle_numbers);
   position = vec4(terrain_scale*(particle_numbers.x - 0.5), heightmap.r*0.5, terrain_scale*(particle_numbers.y - 0.5), gl_Vertex.w);
   
   norm = calculate_normal();
   
   gl_Position = gl_ModelViewProjectionMatrix*position;

}