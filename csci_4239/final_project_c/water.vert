//
//  Water position
//  

varying vec4 position;
uniform float tick;
uniform sampler2D water;
uniform sampler2D smooth_water;

attribute vec2 particle_numbers;

void main(void)
{
   float waveheight = texture2D(water, particle_numbers).r;
   //(cos((particle_numbers.y * 7.0) + (tick * 3.0)) + 1.0) * (sin((particle_numbers.x * 7.0) + (tick * 3.0)) + 1.0) / 20.0 + texture2D(choppy_water, particle_numbers).r;
   position = vec4(particle_numbers.x, waveheight, particle_numbers.y, gl_Vertex.w);

   gl_Position = gl_ModelViewProjectionMatrix*position;
}
