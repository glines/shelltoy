#version 330

flat in int fragAtlasIndex;
in vec2 atlasTexCoord;

flat in vec3 fragFgColor;
flat in vec4 fragBgColor;

uniform sampler2D atlas[1];

out vec4 color;

void main(void) {
  float alpha = texture(atlas[0], atlasTexCoord).r;

  /* Blend the foreground onto the background using alpha as a weight */
  color = vec4(
      alpha * fragFgColor + (1.0 - alpha) * fragBgColor.rgb,
      max(alpha, fragBgColor.a));
}
