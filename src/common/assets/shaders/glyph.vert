#version 430

in vec2 glyphOffset;
in ivec2 cellPos;

in vec2 vertPos;

uniform vec2 cellSize;
uniform ivec2 gridSize;

void main(void) {
  /* TODO: Transform the vertices in our quad to the glyph position */
  /* NOTE: The quad covers the rectangle defined by (0,0) and (1,1) */
  vec2 pos = cellPos * cellSize + vertPos * cellSize;

  gl_Position = vec4(pos, 0.0, 1.0);
}
