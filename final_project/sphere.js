//  See README for source

var latitudeBands = 30;
var longitudeBands = 30;
var radius = 2;
var sphereVerts = [];
var sphereNormals = [];
var sphereTexCoord = [];
var sphereColors = [];
var count = 0;
//  Creates the verticies of a sphere along with normals, colors, and texture coordinates
for (var latNumber = 0; latNumber <= latitudeBands; latNumber++) {
   var theta = latNumber * Math.PI / latitudeBands;
   var sinTheta = Math.sin(theta);
   var cosTheta = Math.cos(theta);
   for (var longNumber = 0; longNumber <= longitudeBands; longNumber++)
   {
      var phi = longNumber * 2 * Math.PI / longitudeBands;
      var sinPhi = Math.sin(phi);
      var cosPhi = Math.cos(phi);

      var x = cosPhi * sinTheta;
      var y = cosTheta;
      var z = sinPhi * sinTheta;
      var u = 1 - (longNumber / longitudeBands);
      var v = 1 - (latNumber / latitudeBands);

      sphereNormals.push(x);
      sphereNormals.push(y);
      sphereNormals.push(z);
      sphereTexCoord.push(u);
      sphereTexCoord.push(v);
      sphereVerts.push(radius * x);
      sphereVerts.push(radius * y);
      sphereVerts.push(radius * z);
      sphereColors.push(1.0);
      sphereColors.push(1.0);
      sphereColors.push(1.0);
      count++;
   }
}

//  Creates sets of three points for triangles that make up the eventual sphere
var indexData = [];
for (var latNumber = 0; latNumber < latitudeBands; latNumber++) {
   for (var longNumber = 0; longNumber < longitudeBands; longNumber++) {
      var first = (latNumber * (longitudeBands + 1)) + longNumber;
      var second = first + longitudeBands + 1;
      indexData.push(first);
      indexData.push(second);
      indexData.push(first + 1);

      indexData.push(second);
      indexData.push(second + 1);
      indexData.push(first + 1);
   }
}