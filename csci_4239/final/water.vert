//
//  Water position
//  

uniform float tick;
uniform sampler2D choppy_water;
uniform sampler2D smooth_water;

attribute vec2 particle_numbers;

varying float textureX;
varying float textureY;
varying float waveheight;

void main(void)
{
   textureX = particle_numbers.x;
   textureY = particle_numbers.y;
   vec4 watermap = texture2D(choppy_water, particle_numbers);
   float x = (watermap.x * 1.0) + (particle_numbers.x * 7.0) + (tick * 3.0);
   float y = (watermap.y * 2.0) + (particle_numbers.y * 7.0) + (tick * 3.0);
   waveheight = ((cos(x) + 1.0) + (sin(y) + 1.0)) / 30.0;
   vec4 position = vec4(particle_numbers.x - 0.5, waveheight, particle_numbers.y - 0.5, gl_Vertex.w);

   gl_Position = gl_ModelViewProjectionMatrix*position;
}
