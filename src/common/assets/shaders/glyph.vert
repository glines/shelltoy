#version 430

in vec2 vertPos;  /* Position of our vertex in the glyph quad. The vertex
                     position is always one of the four corners of the square
                     defined by points (0.0, 0.0) and (1.0, 1.0). */

in int atlasIndex;  /* Index of the atlas texture to use for the glyph
                       associated with this vertex. This allows for the
                       possibility of using more than one sampler if not all
                       glyphs used fit on one texture. */
/* FIXME: Is atlasPos between 0.0 and 1.0, or is it measured in pixels? If the
 * latter, we need to provide the dimensions of the atlas texture as well. Care
 * must be taken since we need to have pixel-perfect textures here. */
in vec2 atlasPos;  /* The position of the glyph in the atlas texture. This
                      position is the bottom-left corner of the glyph in pixel
                      coordinates. */
in vec2 glyphOffset;  /* The offset of the glyph within its cell. As they are
                         stored in the atlas, glyphs do not occupy an entire
                         cell. Instead, this offset must be applied to the
                         glyph from the bottom-left corner of its cell. */
/* FIXME: I'm not sure if the cell is being measured from the bottom left or
 * the top right here. Guess we'll find out. */
in ivec2 cell;  /* The integer position of the cell on the terminal screen of
                   our glyph. */

out vec2 atlasTexCoord;  /* The coordinates at which to access the atlas
                            texture are passed to the fragment shader. */
flat out int fragAtlasIndex;  /* The index of the atlas texture sampler are
                                 also passed to the fragment shader. */

uniform vec2 cellSize;  /* The pixel size of each cell in the terminal. This is
                           used to calculate the position of this vertex on the
                           terminal screen. */
/* FIXME: I don't think we need gridSize for anything right now. It might be
 * useful for effects though. */
uniform ivec2 gridSize;  /* The number of cells that make up the terminal screen. */

void main(void) {
  /* TODO: Transform the vertices in our quad to the glyph position */
  /* NOTE: The quad covers the rectangle defined by (0,0) and (1,1) */
  vec2 pos = cell * cellSize + vertPos * cellSize;

  gl_Position = vec4(pos, 0.0, 1.0);
}
