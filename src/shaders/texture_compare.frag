uniform sampler2D texUnitOld;
uniform sampler2D texUnitNew;

varying vec2 uv;

void main() {
    vec4 colorOld = texture2D(texUnitOld, uv);
    vec4 colorNew = texture2D(texUnitNew, uv);

    // discard (almost) identical pixels
    if (distance(colorOld, colorNew) < 0.0001) {
        discard; 
    }

    // not discarded -> different
    gl_FragColor= vec4(1.0); 
}
