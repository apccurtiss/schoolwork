//
//  Terrain position
//  

attribute vec2 particle_numbers;

uniform sampler2D heightmap_texture;
uniform float terrain_size;
uniform float terrain_scale;
varying float is_water;
varying vec2 texture_coordinates;
varying vec4 position;
varying vec3 norm;
varying vec3 frag_normal;

varying vec3 point_position;
varying vec3 view_position;
varying vec3 light_position;

vec3 triangle_normal(vec3 v1, vec3 v2, vec3 v3) {
   vec3 u = v1 - v2;
   vec3 v = v3 - v2;
   return cross(u, v);
}

vec3 find_position(vec2 coordinates) {
   vec4 heightmap = texture2D(heightmap_texture, coordinates);
   return vec3(terrain_scale*(coordinates.x), heightmap.r, terrain_scale*(coordinates.y));
}

vec3 calculate_normal() {
   vec3 positions[9];

   for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
         float x_offset = float(i - 1) / terrain_size; 
         float y_offset = float(j - 1) / terrain_size; 
         
         positions[i*3 + j] = find_position(particle_numbers + vec2(x_offset, y_offset));
      }
   }
   
   vec3 average = triangle_normal(positions[3],positions[1],positions[0]);
   average += triangle_normal(positions[4],positions[1],positions[3]);
   average += triangle_normal(positions[3],positions[4],positions[1]);
   average += triangle_normal(positions[4],positions[2],positions[1]);
   average += triangle_normal(positions[5],positions[2],positions[4]);
   average += triangle_normal(positions[6],positions[4],positions[3]);
   average += triangle_normal(positions[7],positions[4],positions[6]);
   average += triangle_normal(positions[7],positions[5],positions[4]);
   average += triangle_normal(positions[8],positions[5],positions[7]);
   
   return normalize(average);
   
   
   //vec3 v1 = vec3(find_height;
   //vec3 v2 = position2.xyz;//vec3(1,0,1);
   //vec3 v3 = position3.xyz;//vec3(0,1,0);
   //return normalize(triangle_normal(v1, v2, v3));
   
}

void main(void)
{
   P = vec3(find_position(particle_numbers));
   Light  = vec3(1.0,1.0,1.0) - vec3(position);
   norm = calculate_normal();
   View  = -P;

   position = vec4(P, gl_Vertex.w);
   texture_coordinates = 10.0*vec2(mod(particle_numbers.x, 1.0/10.0),mod(particle_numbers.y, 1.0/10.0));
   gl_Position = gl_ModelViewProjectionMatrix*position;
}