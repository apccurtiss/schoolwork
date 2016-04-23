//
//  Particle engine - derived from Confetti Cannon derived from Orange Book
//  

uniform   sampler2D position_texture;
uniform   sampler2D velocity_texture;
attribute vec2 particle_numbers;   //  Particle number
varying vec4 snow_position;

void main(void)
{
   vec4 position = texture2D(position_texture, particle_numbers);
   //Particle location
   vec4 vertex_position = vec4(position.x - 0.5, position.y, position.z - 0.5, gl_Vertex.w);
   
   snow_position = gl_ModelViewProjectionMatrix*vertex_position;
   //  Let's hope this works...
   gl_Position = snow_position;
   
}
