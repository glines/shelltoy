#version 330

in vec2 vertPos;  /* Position of our vertex in the cell quad. The vertex
                     position is always one of the four corners of the square
                     defined by points (0.0, 0.0) and (1.0, 1.0). */
in ivec2 cell;  /* The integer position of the cell on the terminal screen.
                   Note that this position is measured from the top left of the
                   screen, as is typical with terminal emulators. */
in vec3 fgColor;  /* The color of the underline. */

uniform ivec2 cellSize;  /* The pixel size of each cell in the terminal. This
                            is used to calculate the position of this vertex on
                            the terminal screen. */
uniform ivec2 viewportSize;  /* The dimensions of the viewport in pixels */
uniform int underlineOffset;  /* The number of pixels by which to offset the
                                 underlines from the bottom of each cell */

flat out vec3 fragFgColor;

void main(void) {
  /* We compute the position of this vertex in screen space, which is expressed
   * in pixel coordinates. The vertPos that comes into the shader is part of a
   * quad defined by the points (0, 0) and (1, 1). This quad is positioned to
   * cover a one-pixel high rectangle so that the underline can be drawn. */
  /* FIXME: Support drawing thicker underlines for high-DPI screens */
  vec2 screenPos =
    vec2(cell.x * cellSize.x,
        viewportSize.y - (1 + cell.y) * cellSize.y + underlineOffset)
    + vertPos * vec2(cellSize.x, 1.0);  /* XXX */
  /* Now we compute the position of this vertex in normalized device
   * coordinates, which range from -1 to +1 */
  vec2 normalizedPos = 2.0 * (screenPos / viewportSize) - vec2(1.0);

  /* Pass the foreground color to the fragment shader */
  fragFgColor = fgColor;

  gl_Position = vec4(normalizedPos, 0.0, 1.0);
}
