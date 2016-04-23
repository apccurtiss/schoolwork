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
varying vec3 normal;
varying vec3 norm;

vec3 triangle_normal(vec3 v1, vec3 v2, vec3 v3) {
   vec3 u = v1 - v2;
   vec3 v = v3 - v2;
   return cross(u, v);
}

vec3 calculate_normal() {
   vec3 positions[9];
   for (int i = 0; i < 9; i++) {
      positions[i] = vec3(0.0,0.0,0.0);
   }
   for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
         float x_offset = float(i - 1) / terrain_size; 
         float y_offset = float(j - 1) / terrain_size; 
         vec4 heightmap = texture2D(heightmap_texture, particle_numbers + vec2(x_offset, y_offset));
         float x = particle_numbers.x + x_offset;
         float y = heightmap.r*0.5;
         float z = particle_numbers.y + y_offset;
         positions[i*3, j] = vec3(x,y,z);
      }
   }
   
   vec3 average = triangle_normal(positions[0],positions[1],positions[3]);
   //vec3 average = triangle_normal(positions[4],positions[1],positions[3]);
   //vec3 average = triangle_normal(positions[1],positions[2],positions[4]);
   //vec3 average = triangle_normal(positions[5],positions[2],positions[4]);
   //vec3 average = triangle_normal(positions[3],positions[4],positions[6]);
   //vec3 average = triangle_normal(positions[7],positions[4],positions[6]);
   //vec3 average = triangle_normal(positions[4],positions[5],positions[7]);
   //vec3 average = triangle_normal(positions[8],positions[5],positions[7]);
   
   return normalize(average);
   
   /*
   vec3 v1 = position.xyz;//vec3(0,0,0);
   vec3 v2 = position2.xyz;//vec3(1,0,1);
   vec3 v3 = position3.xyz;//vec3(0,1,0);
   return normalize(triangle_normal(v1, v2, v3));
   */
}

void main(void)
{
   vec4 heightmap = texture2D(heightmap_texture, particle_numbers);
   position = vec4(terrain_scale*(particle_numbers.x - 0.5), heightmap.r*0.5, terrain_scale*(particle_numbers.y - 0.5), gl_Vertex.w);
   
   norm = calculate_normal();
   texture_coordinates = vec2(mod(particle_numbers.x, 2.0), mod(particle_numbers.y, 2.0));
   gl_Position = gl_ModelViewProjectionMatrix*position;
}