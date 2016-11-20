#version 330

flat in int fragAtlasIndex;
in vec2 atlasTexCoord;
in vec2 glyphMask;

flat in vec3 fragFgColor;
flat in vec4 fragBgColor;

uniform sampler2D atlas[1];

out vec4 color;

void main(void) {
  /* The glyphMask ranges from (0.0, 0.0) to (1.0, 1.0) in the region in which
   * we are to draw the glyph. We use a step function to find this region and
   * mask our glyph alpha. */
  vec2 gm = step(0.0, glyphMask) * (1.0 - step(1.0, glyphMask));
  float alpha = texture(atlas[0], atlasTexCoord).r * gm.x * gm.y;

  /* Blend the foreground onto the background using alpha as a weight */
  color = vec4(
      alpha * fragFgColor + (1.0 - alpha) * fragBgColor.rgb,
      max(alpha, fragBgColor.a));
}
