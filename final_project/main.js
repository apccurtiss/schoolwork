var gl,canvas;
//  Modified from ex08
function webGLStart()
{
   var cubeTexture;
   //  See README for source
   function initTextures() {
     cubeTexture = gl.createTexture();
     cubeImage = new Image();
     cubeImage.src = "earth_physical.bmp";
     cubeImage.onload = function() { handleTextureLoaded(cubeImage, cubeTexture); Display() }
   }
   
   //  See README for source
   function handleTextureLoaded(image, texture) {
     gl.bindTexture(gl.TEXTURE_2D, texture);
     gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, image);
     gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
     gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
     gl.generateMipmap(gl.TEXTURE_2D);
     gl.bindTexture(gl.TEXTURE_2D, null);
   }
   
   //  Set canvas
   canvas = document.getElementById("canvas");
   //  Select canvas size
   var size = Math.min(window.innerWidth,window.innerHeight)-10;
   canvas.width  = size;
   canvas.height = size;
   
   //  Start WebGL
   if (!window.WebGLRenderingContext)
   {
      alert("Your browser does not support WebGL. See http://get.webgl.org");
      return;
   }
   try
   {
      gl = canvas.getContext("experimental-webgl");
   }
   catch(e)
   {}
   if (!gl)
   {
      alert("Can't get WebGL");
      return;
   }

   //  Set viewport to entire canvas
   gl.viewport(0,0,size,size);

   //  Load Shader
   var prog = CompileShaderProg(gl,"shader-vs","shader-fs");

   //  Set program
   gl.useProgram(prog);

   //  Set projection
   var ProjectionMatrix = new CanvasMatrix4();
   ProjectionMatrix.ortho(-2.5,+2.5,-2.5,+2.5,-2.5,+2.5);
   
   //  Binds arrays set in sphere.js
   var sNormals = gl.createBuffer();
   gl.bindBuffer(gl.ARRAY_BUFFER,sNormals);
   gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(sphereNormals),gl.STATIC_DRAW);
   
   var sTexCoord = gl.createBuffer();
   gl.bindBuffer(gl.ARRAY_BUFFER,sTexCoord);
   gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(sphereTexCoord),gl.STATIC_DRAW);
   
   var sVerts = gl.createBuffer();
   gl.bindBuffer(gl.ARRAY_BUFFER,sVerts);
   gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(sphereVerts),gl.STATIC_DRAW);
   
   var sColors = gl.createBuffer();
   gl.bindBuffer(gl.ARRAY_BUFFER,sColors);
   gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(sphereColors),gl.STATIC_DRAW);
   
   var sElementBuffer = gl.createBuffer();
   gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,sElementBuffer);
   gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(indexData),gl.STATIC_DRAW);

   //  Set state to draw scene
   gl.enable(gl.DEPTH_TEST);
   gl.clearColor(0.8,0.8,0.8,1);
   //  Mouse control variables
   var x0 = y0 = move  = 0;
   //  Rotation angles
   var th = ph = -150;
   var phLight = 0.0;
   var hLight = 1.0;
   //  Draw scene the first time
   
   //  For animation of the light
   var startTime = new Date();
   
   initTextures();

   //
   //  Display the scene
   //  Modified from ex08
   //
   function Display()
   {
      //  Move the light
      var time = (new Date().getTime() - startTime.getTime()) / 500.0;
      phLight = time;

      gl.activeTexture(gl.TEXTURE0);
      gl.bindTexture(gl.TEXTURE_2D, cubeTexture);
      gl.uniform1i(gl.getUniformLocation(prog, "uSampler"), 0);
      //  Clear the screen and Z buffer
      gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);

      // Compute modelview matrix
      var ModelviewMatrix = new CanvasMatrix4();
      ModelviewMatrix.makeIdentity();
      ModelviewMatrix.rotate(ph,0,1,0);
      ModelviewMatrix.rotate(th,1,0,0);
      
      //  Compute normal matrix
      var NormalMatrix = ModelviewMatrix;
      NormalMatrix.invert();
      NormalMatrix.transpose();

      // Set shader
      gl.useProgram(prog);

      //  Set projection and modelview matrixes
      gl.uniformMatrix4fv(gl.getUniformLocation(prog,"ProjectionMatrix") , false , new Float32Array(ProjectionMatrix.getAsArray()));
      gl.uniformMatrix4fv(gl.getUniformLocation(prog,"ModelviewMatrix")  , false , new Float32Array(ModelviewMatrix.getAsArray()));
      gl.uniformMatrix4fv(gl.getUniformLocation(prog,"NormalMatrix")     , false , new Float32Array(NormalMatrix.getAsArray()));
//float Position[] = {(float)(3*Cos(zh)),z0,(float)(3*Sin(zh)),1.0};

      //  Set up 3D vertex array
      gl.bindBuffer(gl.ARRAY_BUFFER,sVerts);
      var XYZ = gl.getAttribLocation(prog,"XYZ");
      gl.enableVertexAttribArray(XYZ);
      gl.vertexAttribPointer(XYZ,3,gl.FLOAT,false,0,0);

      //  Set up 3D color array
      gl.bindBuffer(gl.ARRAY_BUFFER,sColors);
      var RGB = gl.getAttribLocation(prog,"RGB");
      gl.enableVertexAttribArray(RGB);
      gl.vertexAttribPointer(RGB,3,gl.FLOAT,false,0,0);
      
      //  Set up 3D normal array
      gl.bindBuffer(gl.ARRAY_BUFFER,sNormals);
      var NORM = gl.getAttribLocation(prog,"NORM");
      gl.enableVertexAttribArray(NORM);
      gl.vertexAttribPointer(NORM,3,gl.FLOAT,false,0,0);
      
      //  Set up texture coordinates array
      gl.bindBuffer(gl.ARRAY_BUFFER,sTexCoord);
      var TEX = gl.getAttribLocation(prog,"TEX");
      gl.enableVertexAttribArray(TEX);
      gl.vertexAttribPointer(TEX,2,gl.FLOAT,false,0,0);
      
      //  Set up element array
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,sElementBuffer);
  
      //  Set up light motion
      gl.uniform4f(gl.getUniformLocation(prog,"LightPos"), 3*Math.cos(phLight), hLight, 3*Math.sin(phLight), 1.0);
      gl.uniform4f(gl.getUniformLocation(prog,"ambient"), 0.5,0.5,0.5,1.0);
      gl.uniform4f(gl.getUniformLocation(prog,"diffuse"), 0.9,0.9,0.9,1.0);
      gl.uniform4f(gl.getUniformLocation(prog,"specular"), 0.9,0.9,0.9,1.0);
      gl.uniform1f(gl.getUniformLocation(prog,"shininess"), 30.0);
      
  
      //  Draw all vertexes;
      gl.drawElements(gl.TRIANGLES, indexData.length, gl.UNSIGNED_SHORT,0);

      //  Disable vertex arrays
      gl.disableVertexAttribArray(XYZ);
      gl.disableVertexAttribArray(RGB);
      gl.disableVertexAttribArray(NORM);
      gl.disableVertexAttribArray(TEX);

      //  Flush
      gl.flush ();
   }

   //
   //  Resize canvas
   //
   canvas.resize = function ()
   {
      var size = Math.min(window.innerWidth, window.innerHeight)-10;
      canvas.width  = size;
      canvas.height = size;
      gl.viewport(0,0,size,size);
   }

   //
   //  Mouse button pressed
   //
   canvas.onmousedown = function (ev)
   {
      move  = 1;
      x0 = ev.clientX;
      y0 = ev.clientY;
   }

   //
   //  Mouse button released
   //
   canvas.onmouseup = function (ev)
   {
      move  = 0;
   }

   //
   //  Mouse movement
   //
   canvas.onmousemove = function (ev)
   {
      if (move==0) return;
      //  Update angles - reverses if sphere is upside-down
      ph -= Math.cos(Math.PI * th / 180) * (ev.clientX-x0) / 3;
      th += (ev.clientY-y0) / 3;
      //  Store location
      x0 = ev.clientX;
      y0 = ev.clientY;
   }
}

//  Refresh the display
function render() {
   requestAnimationFrame(render);
   Display();
}