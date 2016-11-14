#version 430

flat in int fragAtlasIndex;
in vec2 atlasTexCoord;

uniform sampler2D atlas[4];

out vec4 color;

void main(void) {
  color = texture(atlas[fragAtlasIndex], atlasTexCoord);
}
