#version 330

flat in vec4 fragBgColor;

out vec4 color;

void main(void) {
  color = vec4(fragBgColor);
}
