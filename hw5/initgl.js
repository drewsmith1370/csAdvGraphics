export function CreateShader(gl, vert, frag) {
  // Create the shader objects
  let vertShader = gl.createShader(gl.VERTEX_SHADER);
  let fragShader = gl.createShader(gl.FRAGMENT_SHADER);
  // Specify the sources
  gl.shaderSource(vertShader, vert);
  gl.shaderSource(fragShader, frag);
  // Compile the shaders
  gl.compileShader(vertShader);
  gl.compileShader(fragShader);

  // Check compile status
  if (!gl.getShaderParameter(vertShader, gl.COMPILE_STATUS)) {
    console.log(
      "Vertex Shader Compilation Error: " + gl.getShaderInfoLog(vertShader)
    );
    return;
  }
  if (!gl.getShaderParameter(fragShader, gl.COMPILE_STATUS)) {
    console.log(
      "Fragment Shader Compilation Error: " + gl.getShaderInfoLog(fragShader)
    );
    return;
  }

  // Create shader program
  let shdrProg = gl.createProgram();
  // Attach shaders
  gl.attachShader(shdrProg, vertShader);
  gl.attachShader(shdrProg, fragShader);
  // Link
  gl.linkProgram(shdrProg);

  // Check link status
  if (!gl.getProgramParameter(shdrProg, gl.LINK_STATUS)) {
    console.log("Program Linking Error: " + gl.getProgramInfoLog(shdrProg));
    return;
  }

  // Return
  return shdrProg;
}

/*
 *  Create a VAO from vertices and indices.
 *
 *  For attribs: [{
 *  .name: string;
 *  .size: GLint;
 *  .type: GLenum;
 *  .normalized: GLboolean;
 *  .stride: GLsizei;
 *  .offset: GLintptr;
 * }]
 */
export function CreateVao(gl, vrts, idxs, shdr, attribs) {
  // Create VAO
  const vao = gl.createVertexArray();
  gl.bindVertexArray(vao);

  // Vertex data
  gl.bindBuffer(gl.ARRAY_BUFFER, gl.createBuffer());
  gl.bufferData(gl.ARRAY_BUFFER, vrts, gl.STATIC_DRAW);
  // Index data
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, gl.createBuffer());
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, idxs, gl.STATIC_DRAW);

  // Vertex attributes
  for (let attr of attribs) {
    const loc = gl.getAttribLocation(shdr, attr.name);
    if (loc !== -1) {
      gl.vertexAttribPointer(
        loc,
        attr.size,
        attr.type,
        attr.false,
        attr.stride,
        attr.offset
      );
      gl.enableVertexAttribArray(loc);
    } else {
      console.log(`Attribute "${attr.name}" not found in shader "${shdr}".`);
    }
  }

  // Clean and return
  gl.bindVertexArray(null);
  return vao;
}

//
// Create a UBO object
//
export function CreateUBO(gl, size, shaders, name, binding) {
  // Create buffer
  const buffer = gl.createBuffer();
  gl.bindBuffer(gl.UNIFORM_BUFFER, buffer);

  // Allocate memory
  gl.bufferData(gl.UNIFORM_BUFFER, size, gl.DYNAMIC_DRAW);

  // Assign binding point to each shader
  for (const shader of shaders) {
    let uniformBlockIndex = gl.getUniformBlockIndex(shader, name);
    gl.uniformBlockBinding(shader, uniformBlockIndex, binding);
  }

  // Bind buffer to binding point
  gl.bindBufferBase(gl.UNIFORM_BUFFER, binding, buffer);
  return buffer;
}

//
// Set data for uniform buffer
//
export function SetUBO(gl, ubo, data) {
  gl.bindBuffer(gl.UNIFORM_BUFFER, ubo);
  gl.bufferSubData(gl.UNIFORM_BUFFER, 0, data);
}

export {};
