//  See README for source
var size = 10;
var poly_count = 30;
var ground_element_buffer;
var ground_vert_buffer;
var ground_tex_coord_buffer;
var ground_normal_buffer;

function init_ground(gl) {
    var ground_verts = [];
    var ground_normals = [];
    var ground_tex_coords = [];
    var ground_colors = [];

    var count = 0;
    //  Creates the verticies of a sphere along with normals, colors, and texture coordinates
    for (var x = -poly_count/2; x <= poly_count/2; x++) {
        var dx = x * size / poly_count;
        var xtex = (x + poly_count/2) / poly_count;
        for (var z = -poly_count/2; z <= poly_count/2; z++) {
            var dz = z * size / poly_count;
            var ztex = (z + poly_count/2) / poly_count;
            
            ground_verts.push(dx);
            ground_verts.push(0.0);
            ground_verts.push(dz);
            ground_tex_coords.push(xtex);
            ground_tex_coords.push(ztex);
            ground_normals.push(0.0);
            ground_normals.push(1.0);
            ground_normals.push(0.0);
            count++;
        }
    }

    //  Creates sets of three points for triangles that make up the eventual sphere
    var ground_vert_index_data = [];
    for (var x = -poly_count/2; x <= poly_count/2; x++) {
        for (var z = -poly_count/2; z <= poly_count/2; z++) {
          var first = (x * poly_count) + z;
          var second = first + poly_count;
          ground_vert_index_data.push(first);
          ground_vert_index_data.push(second);
          ground_vert_index_data.push(second + 1);

          ground_vert_index_data.push(first);
          ground_vert_index_data.push(first + 1);
          ground_vert_index_data.push(second + 1);
       }
    }

    ground_normal_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,ground_normal_buffer);
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(ground_normals),gl.STATIC_DRAW);

    ground_tex_coord_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,ground_tex_coord_buffer);
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(ground_tex_coords),gl.STATIC_DRAW);

    ground_vert_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,ground_vert_buffer);
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(ground_verts),gl.STATIC_DRAW);
    
    ground_element_buffer = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,ground_element_buffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(ground_vert_index_data),gl.STATIC_DRAW);
    console.log(ground_element_buffer);
}

function draw_ground(gl, prog) {
      //console.log(ground_element_buffer);
      //  Set up 3D vertex array
      gl.bindBuffer(gl.ARRAY_BUFFER,ground_vert_buffer);
      var XYZ = gl.getAttribLocation(prog,"XYZ");
      gl.enableVertexAttribArray(XYZ);
      gl.vertexAttribPointer(XYZ,3,gl.FLOAT,false,0,0);

      //  Set up 3D color array
      gl.bindBuffer(gl.ARRAY_BUFFER,ground_vert_buffer);
      var RGB = gl.getAttribLocation(prog,"RGB");
      gl.enableVertexAttribArray(RGB);
      gl.vertexAttribPointer(RGB,3,gl.FLOAT,false,0,0);
      
      //  Set up 3D normal array
      gl.bindBuffer(gl.ARRAY_BUFFER,ground_normal_buffer);
      var NORM = gl.getAttribLocation(prog,"NORM");
      gl.enableVertexAttribArray(NORM);
      gl.vertexAttribPointer(NORM,3,gl.FLOAT,false,0,0);
      
      //  Set up texture coordinates array
      gl.bindBuffer(gl.ARRAY_BUFFER,ground_tex_coord_buffer);
      var TEX = gl.getAttribLocation(prog,"TEX");
      gl.enableVertexAttribArray(TEX);
      gl.vertexAttribPointer(TEX,2,gl.FLOAT,false,0,0);
      
      //  Set up element array
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,ground_element_buffer);
      
      //  Draw all vertexes;
      //console.log([1,1,1]);
      gl.drawElements(gl.TRIANGLES, ground_element_buffer.length, gl.UNSIGNED_SHORT,0);

      //  Disable vertex arrays
      gl.disableVertexAttribArray(XYZ);
      gl.disableVertexAttribArray(RGB);
      gl.disableVertexAttribArray(NORM);
      gl.disableVertexAttribArray(TEX);
      
      return;
}