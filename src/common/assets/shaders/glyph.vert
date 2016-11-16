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
in vec2 glyphSize;  /* The dimension of the glyph as it appears in the atlas,
                       including padding. Most glyphs are slightly smaller than
                       the terminal cell. */
in vec2 offset;  /* The offset of the glyph within its cell. As they are stored
                    in the atlas, glyphs do not occupy an entire cell. Instead,
                    this offset must be applied to the glyph from the
                    bottom-left corner of its cell. */
/* FIXME: I'm not sure if the cell is being measured from the bottom left or
 * the top right here. Guess we'll find out. */
in ivec2 cell;  /* The integer position of the cell on the terminal screen of
                   our glyph. */

out vec2 atlasTexCoord;  /* The coordinates at which to access the atlas
                            texture are passed to the fragment shader. */
flat out int fragAtlasIndex;  /* The index of the atlas texture sampler are
                                 also passed to the fragment shader. */

uniform ivec2 cellSize;  /* The pixel size of each cell in the terminal. This
                            is used to calculate the position of this vertex on
                            the terminal screen. */
/* FIXME: I don't think we need gridSize for anything right now. It might be
 * useful for effects though. */
uniform ivec2 gridSize;  /* The number of cells that make up the terminal screen. */
uniform ivec2 viewportSize;  /* The dimensions of the viewport in pixels */
uniform int atlasSize;  /* The dimensions of the atlas texture. Since the atlas
                           texture is always a square, only one value is
                           given. */


/* TODO: Need to pass in an array of integers for the atlas size */

void main(void) {
  /* FIXME: For now, this shader assumes cells start at the bottom left, when
   * in fact they start at the top left */

  /* We compute the position of this vertex in screen space, which is expressed
   * in pixel coordinates */
  /* NOTE: The vertPos that comes into the shader is for a quad defined by the
   * points (0, 0) and (1, 1). */
  vec2 screenPos = cell * cellSize + vertPos * cellSize + offset;
  /* Now we compute the position of this vertex in normalized device
   * coordinates, which range from -1 to +1 */
  vec2 normalizedPos = 2.0 * (screenPos / viewportSize) - vec2(1.0);

  /* Compute the texture coordinates of our glyph in the atlas */
  atlasTexCoord = (atlasPos + vertPos * glyphSize) / ivec2(atlasSize);

  /* Pass some parameters as non-interpolated input to the fragment shader */
  fragAtlasIndex = atlasIndex;

  gl_Position = vec4(normalizedPos, 0.0, 1.0);
}
