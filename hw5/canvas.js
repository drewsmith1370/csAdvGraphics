import { CreateShader, CreateUBO, CreateVao } from "./initgl.js";
import { vertSource, fragSource } from "./shaders.js";
import { mat4 } from "./mat4.js";

// Window
var cnv = document.getElementById("cnv");
var gl = cnv.getContext("webgl2");
var prevTime;
var deltaTime;

// Matrices
const viewMat = new mat4();
const modelMat = new mat4();
const projMat = new mat4();

// UBO
const sharedUboBindingPoint = 0;
// mat4 uView
// mat4 uProj
// vec3 uLightDir
// float uTime

// Objects
const fish = {
  vao: undefined,
  shader: undefined,
  ubo: undefined,
};

// Fish
const vertices = new Float32Array([-1, -1, 0, 0, 1, 0, 1, -1, 0]);
const indices = new Uint32Array([0, 1, 2]);
const vertAttribs = [
  {
    name: "aPos",
    size: 3,
    type: gl.FLOAT,
    normalized: false,
    stride: 12,
    offset: 0,
  },
];

// Initialize after window is loaded
window.addEventListener("load", () => {
  // Make sure gl context is present
  if (!gl) {
    console.log("gl context not found");
    return;
  }

  // Create shaders and objects
  fish.shader = CreateShader(gl, vertSource, fragSource);
  fish.vao = CreateVao(gl, vertices, indices, fish.shader, vertAttribs);

  // Create the shared UBO
  CreateUBO(gl, 48, [fish.shader], "SharedUniformBlock", sharedUboBindingPoint);

  // Set viewport and clear colors
  gl.viewport(
    0,
    0,
    gl.drawingBufferWidth * devicePixelRatio,
    gl.drawingBufferHeight * devicePixelRatio
  );
  // Set the clear color
  gl.clearColor(0, 0.8, 1, 1);

  // Begin the main loop
  requestAnimationFrame(mainLoop);
});

function display() {
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  projMat.ortho();

  // Draw fish
  gl.useProgram(fish.shader);
  gl.bindVertexArray(fish.vao);
  gl.drawElements(gl.TRIANGLES, 3, gl.UNSIGNED_INT, 0);

  gl.flush();
}

function updateTime(currentTime) {
  deltaTime = currentTime - prevTime;
  prevTime = currentTime;
}

function mainLoop(currentTime) {
  updateTime();
  display();
  requestAnimationFrame(mainLoop);
}
