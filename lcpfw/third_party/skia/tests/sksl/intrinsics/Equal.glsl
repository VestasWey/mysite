
out vec4 sk_FragColor;
in vec4 a;
in vec4 b;
void main() {
    sk_FragColor.x = float(equal(a, b).x ? 1 : 0);
}
