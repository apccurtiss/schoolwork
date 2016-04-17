//
//  Compile a shader
//  Taken verbatum from ex08
//
function CompileShader(gl,id)
{
   //  Get shader by id
   var src = document.getElementById(id);
   //  Create shader based on type setting
   var shader;
   if (src.type == "x-shader/x-fragment")
      shader = gl.createShader(gl.FRAGMENT_SHADER);
   else if (src.type == "x-shader/x-vertex")
      shader = gl.createShader(gl.VERTEX_SHADER);
   else
      return null;
   //  Read source into str
   var str = "";
   var k = src.firstChild;
   while (k)
   {
      if (k.nodeType == 3) str += k.textContent;
      k = k.nextSibling;
   }
   gl.shaderSource(shader, str);
   //  Compile the shader
   gl.compileShader(shader);
   //  Check for errors
   if (gl.getShaderParameter(shader, gl.COMPILE_STATUS) == 0)
      alert(gl.getShaderInfoLog(shader));
   //  Return shader
   return shader;
}

//
//  Compile shader program
//  Taken verbatum from ex08
//
function CompileShaderProg(gl,vert,frag)
{
   //  Compile the program
   var prog  = gl.createProgram();
   gl.attachShader(prog , CompileShader(gl,vert));
   gl.attachShader(prog , CompileShader(gl,frag));
   gl.linkProgram(prog);
   //  Check for errors
   if (gl.getProgramParameter(prog, gl.LINK_STATUS) == 0)
      alert(gl.getProgramInfoLog(prog));
   //  Return program
   return prog;
}