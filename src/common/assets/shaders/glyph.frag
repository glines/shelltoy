#version 330

in vec2 atlasTexCoord;
flat in vec3 fragFgColor;

uniform sampler2D atlas;

out vec4 color;

void main(void) {
  /* Alpha defines the shape of the glyph */
  float alpha = texture(atlas, atlasTexCoord).r;

  color = vec4(fragFgColor, alpha);
}
