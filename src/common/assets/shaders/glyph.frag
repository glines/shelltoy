#version 430

flat in int aaAtlasIndex;
flat in ivec2 aaAtlasPos;
flat in ivec2 aaAtlasDim;
flat in ivec2 sdfAtlasPos;
flat in ivec2 sdfAtlasDim;

in vec2 glyphOffset;

in vec2 texCoord;

uniform sampler2D aaAtlas[4], sdfAtlas[4];

out vec4 color;

void main(void) {
  /* TODO: We need to compensate atlas pos, size, glyph offset, etc. */

  color = texture(aaAtlas[aaAtlasIndex], texCoord);
}
