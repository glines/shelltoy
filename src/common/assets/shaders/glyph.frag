#version 430

flat in int atlasIndex;
flat in ivec2 atlasPos;
flat in ivec2 atlasDim;

in vec2 glyphOffset;

in vec2 texCoord;

uniform sampler2D aaAtlas[4], sdfAtlas[4];

out vec4 color;

void main(void) {
  /* TODO: We need to compensate atlas pos, size, glyph offset, etc. */

  color = texture(aaAtlas[aaAtlasIndex], texCoord);
}
