#version 430

in vec2 vertPos;

in vec2 offset;
in ivec2 cell;

flat out atlasIndex;

uniform vec2 cellSize;
uniform ivec2 gridSize;

void main(void) {
  /* TODO: Transform the vertices in our quad to the glyph position */
  /* NOTE: The quad covers the rectangle defined by (0,0) and (1,1) */
  vec2 pos = cellPos * cellSize + vertPos * cellSize;

  gl_Position = vec4(pos, 0.0, 1.0);
}
