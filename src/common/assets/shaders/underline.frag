#version 330

flat in vec3 fragFgColor;

out vec4 color;

void main(void) {
  color = vec4(fragFgColor, 1.0);  /* XXX */
}
