/* Input */
varying vec4 v_Color;
varying vec2 v_Texcoord0;
uniform sampler2D u_Texture0;
uniform int u_Time;

void main() {
    gl_FragColor = texture2D(u_Texture0, v_Texcoord0) * v_Color;
}
