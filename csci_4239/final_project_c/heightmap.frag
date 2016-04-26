//
//  Terrain color
//  

uniform sampler2D heightmap_texture;   
uniform sampler2D rock_texture;

//  Terrtain
varying vec2 texture_coordinates;
varying vec4 position;
varying vec3 frag_normal;

//  Lighting
varying vec3 light_position;
varying vec3 view_position;
varying vec3 point_position;

const vec4 grass = vec4(0.6,1.0,0.6,1.0);
const vec4 snow = vec4(1.0,1.0,1.0,1.0);

vec4 blinn()
{
   //  N is the object normal
   vec3 N = normalize(frag_normal);
   //  L is the light vector
   vec3 L = normalize(light_position);
   //  V is the view vector
   vec3 V = normalize(view_position);

   //  Emission and ambient color
   vec4 color = vec4(0.3,0.0,0.0,1.0);

   //  Diffuse light is cosine of light and normal vectors
   float Id = dot(L,N);
   if (Id>0.0)
   {
      //  Add diffuse
      color += Id*vec4(0.0,0.5,0.0,1.0);
      //  The half vectors
      vec3 H = normalize(V+L);
      //  Specular is cosine of reflected and view vectors
      float Is = dot(H,N);
      if (Is>0.0) color += pow(Is,32)*vec4(0.5,0.5,0.5,1.0);
   }

   //  Return sum of color components
   return color;
}

void main(void)
{
   vec4 color = mix(grass, snow, position.y * 3.0);
  
   if (norm.y < 0.7) {
      if (position.y > 0.5) {
         color = texture2D(rock_texture, texture_coordinates);
      } else {
         color = mix(texture2D(rock_texture, texture_coordinates), color, (norm.y * 0.7));
      }
   }
  
   if (position.y < 0.05) { 
      color = vec4(0.2,0.6,0.5,0.5);
   }
   
   gl_FragColor = color * blinn();
   
   //vec4(position.y * 2.0, 0.5-position.y, 0.0, 1.0);
}
