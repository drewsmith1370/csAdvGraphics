import { CreateShader, CreateUBO, CreateVao, SetUBO } from "./initgl.js";
import { vertSource, fragSource, bgVertSrc, bgFragSrc } from "./shaders.js";
import { mat4, normalize } from "./mat4.js";

// Constants
const FISH_SPEED = 1e-3;

// Window
var cnv = document.getElementById("cnv");
var gl = cnv.getContext("webgl2");
var prevTime = 0;
var deltaTime = 0;
var asp = window.innerHeight / window.innerWidth;
var dim = 2;

// Matrices
const viewMat = new mat4();
const modelMat = new mat4();
const projMat = new mat4();

// UBO
var uboBindingPoint = 0;
var sharedUbo;
var lightDir = new Float32Array([-1, 1, 0]);

// Objects
const fish = {
  vao: undefined,
  shader: undefined,
  ubo: undefined,
  position: [1, 0, -0.8],
  direction: normalize(-1, 1, 0),
};
const bg = {
  vao: undefined,
  shader: undefined,
  ubo: undefined,
};
console.log(fish.position);
var texture;

// Fish
const vertices = new Float32Array([-1, -1, 0, 0, 1, 0, 1, -1, 0]);
const indices = new Uint32Array([0, 1, 2]);
// prettier-ignore
const fishVert = new Float32Array([
  -1.0, 0.0, 0.0 , -1, 0, 0 ,
   0.0, 0.5, 0.0 ,  0, 1, 0 ,
   0.0, 0.0, 0.5 ,  0, 0, 1 ,
   0.0,-0.5, 0.0 ,  0,-1, 0 ,
   0.0, 0.0,-0.5 ,  0, 0,-1 ,
   0.5, 0.0, 0.0 ,  1, 0, 0 ,
  -0.8, 0.0, 0.0 ,  0, 0, 1 ,
  -1.5, 0.5, 0.0 ,  0, 0, 1 ,
  -1.5,-0.5, 0.0 ,  0, 0, 1 ,
]);
// prettier-ignore
const fishInd = new Uint32Array([0,1,2 , 0,2,3 , 0,3,4 , 0,4,1 , 1,5,2 , 2,5,3 , 3,5,4 , 4,5,1 , 6,7,8]);
const vertNrmAttribs = [
  {
    name: "aPos",
    size: 3,
    type: gl.FLOAT,
    normalized: false,
    stride: 24,
    offset: 0,
  },
  {
    name: "aNrm",
    size: 3,
    type: gl.FLOAT,
    normalized: false,
    stride: 24,
    offset: 12,
  },
];
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
// Background
const bgVert = new Float32Array([-1, -1, 1, -1, 1, 1, 1, 1, 1, 1, -1, 1]);
const bgInd = new Uint32Array([0, 1, 2, 2, 3, 0]);

// Initialize after window is loaded
window.addEventListener("load", () => {
  // Make sure gl context is present
  if (!gl) {
    console.log("gl context not found");
    return;
  }

  // Create shaders and objects
  fish.shader = CreateShader(gl, vertSource, fragSource);
  fish.vao = CreateVao(gl, fishVert, fishInd, fish.shader, vertNrmAttribs);
  fish.ubo = CreateUBO(
    gl,
    64,
    [fish.shader],
    "SpecificUniformBlock",
    uboBindingPoint++
  );
  bg.shader = CreateShader(gl, bgVertSrc, bgFragSrc);
  bg.vao = CreateVao(gl, bgVert, bgInd, bg.shader, vertAttribs);
  // bg.ubo = CreateUBO();

  // Create the shared UBO
  sharedUbo = CreateUBO(
    gl,
    144,
    [fish.shader],
    "SharedUniformBlock",
    uboBindingPoint++
  );

  texture = gl.createTexture(gl.TEXTURE_2D);
  // createWindowTexture();

  gl.enable(gl.DEPTH_TEST);
  // Set viewport and clear colors
  cnv.width = window.innerWidth;
  cnv.height = window.innerHeight;
  gl.viewport(0, 0, window.innerWidth, window.innerHeight);
  asp = innerWidth / innerHeight;
  // Set the clear color
  gl.clearColor(0, 0, 0, 0);

  reshape();

  // Begin the main loop
  requestAnimationFrame(mainLoop);
});

function createBufferData(view, proj, ldir, time) {
  let data = new Float32Array(36);
  let m = view.getMat();
  for (var i = 0; i < 16; i++) {
    data[i] = m[i];
  }
  m = proj.getMat();
  for (var i = 0; i < 16; i++) {
    data[i + 16] = m[i];
  }
  m = new Float32Array(ldir);
  for (var i = 0; i < 3; i++) {
    data[i + 32] = m[i];
  }
  data[35] = time;
  return data;
}

function drawFish(fish, time, deltaTime) {
  // Update direction
  let dir = fish.direction;

  dir[0] = -2 * Math.sin(time / 5000);
  dir[1] = 0.5 * Math.cos(time / 1000);
  dir[2] = 0.1 * Math.cos(time / 50);

  dir = normalize(...dir);
  fish.direction = dir;
  if (dir[0] !== dir[0]) throw new Error("NaN Direction");

  // Move fish
  let pos = fish.position;
  pos[0] = 2 * Math.cos(time / 5000);
  pos[1] = 0.5 * Math.sin(time / 1000);
  pos[2] = -0.5 * Math.sin(time / 5000) - 0.8;
  fish.position = pos;

  // Set model matrix
  var model = new mat4();
  model.translate(fish.position[0], fish.position[1], fish.position[2]);
  model.rotateTowards(...fish.direction, 0, 1, 0);
  model.scale(0.3, 0.3, 0.3);
  SetUBO(gl, fish.ubo, model.getMat());

  // Draw fish
  gl.useProgram(fish.shader);
  gl.bindVertexArray(fish.vao);
  gl.drawElements(gl.TRIANGLES, fishInd.length, gl.UNSIGNED_INT, 0);
}

function display(currentTime) {
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  viewMat.identity();
  projMat.identity();

  viewMat.lookAt(0, 0, -3, 0, 0, 0, 0, 1, 0);
  // projMat.ortho(-2, 2, -2, 2, -2, 2);
  // projMat.perspective(59, asp, dim / 16, dim * 16);
  projMat.perspective(59, asp, dim / 16, dim * 16);

  // Set shared uniform
  let data = createBufferData(viewMat, projMat, lightDir, currentTime);
  // console.log(data);
  SetUBO(gl, sharedUbo, data);

  // Draw the fish
  drawFish(fish, currentTime, deltaTime);

  // Draw bg
  gl.useProgram(bg.shader);
  gl.bindVertexArray(bg.vao);
  gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_INT, 0);
  gl.flush();
}

function updateTime(currentTime) {
  deltaTime = currentTime - prevTime;
  prevTime = currentTime;
}

function mainLoop(currentTime) {
  updateTime(currentTime);
  display(currentTime);
  requestAnimationFrame(mainLoop);
}

//
//  Resize canvas
//
function reshape() {
  cnv.width = window.innerWidth;
  cnv.height = window.innerHeight;
  gl.viewport(0, 0, window.innerWidth, window.innerHeight);
  asp = innerWidth / innerHeight;

  // Update page content bounds
  var element = document.getElementById("page-contents");
  var rect = element.getBoundingClientRect();

  // Extract dimension info
  rect = {
    left: rect.left,
    right: rect.right,
    top: rect.top,
    bottom: rect.bottom,
  };

  // Convert to canvas coordinates
  var l = (2 * rect.left) / cnv.width - 1;
  var r = (2 * rect.right) / cnv.width - 1;
  var t = -(2 * rect.top) / cnv.height + 1;
  var b = -(2 * rect.bottom) / cnv.height + 1;

  // Update the vao
  bgVert[0] = l;
  bgVert[3] = l;
  bgVert[6] = r;
  bgVert[9] = r;
  bgVert[4] = t;
  bgVert[7] = t;
  bgVert[1] = b;
  bgVert[10] = b;

  // if the instance is already created update the buffer
  if (bg.vao) {
    gl.bindVertexArray(bg.vao);
    gl.bufferSubData(gl.ARRAY_BUFFER, 0, bgVert);
    gl.bindVertexArray(null);
  }
}
window.addEventListener("resize", reshape);
