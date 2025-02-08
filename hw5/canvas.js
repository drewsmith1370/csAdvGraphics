window.g = {
  cnv: undefined,
};

window.addEventListener("load", () => {
  // Find canvas and gl context
  g.cnv = document.getElementById("cnv");
  var gl = g.cnv.getContext("webgl");
  if (!gl) {
    console.log("gl context not found");
    return;
  }

  // Set viewport and clear colors
  gl.viewport(0, 0, gl.drawingBufferWidth, gl.drawingBufferHeight);
  // Set the clear color to darkish green.
  gl.clearColor(0, 0, 0, 0);
  // Clear the context with the newly set color. This is
  // the function call that actually does the drawing.
  gl.clear(gl.COLOR_BUFFER_BIT);
});
