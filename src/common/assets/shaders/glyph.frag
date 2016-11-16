#version 330

flat in int fragAtlasIndex;
in vec2 atlasTexCoord;

uniform sampler2D atlas[1];

out vec4 color;

void main(void) {
  color = vec4(texture(atlas[0], atlasTexCoord).rr, 1.0, 1.0);
}
