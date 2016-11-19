#version 330

flat in int fragAtlasIndex;
in vec2 atlasTexCoord;

uniform sampler2D atlas[1];

out vec4 color;

void main(void) {
  float alpha = texture(atlas[0], atlasTexCoord).r;
  color = vec4(vec3(1.0), alpha);
}
