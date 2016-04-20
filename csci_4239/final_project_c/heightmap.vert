//
//  Terrain position
//  

uniform sampler2D heightmap_texture;
varying vec4 position;

attribute vec2 particle_numbers;

void main(void)
{
   vec4 heightmap = texture2D(heightmap_texture, particle_numbers);

   position = vec4(particle_numbers.x, heightmap.r*0.5, particle_numbers.y, gl_Vertex.w);

   gl_Position = gl_ModelViewProjectionMatrix*position;
}
