#version 140

uniform sampler2D texUnitOld;
uniform sampler2D texUnitNew;

in vec2 uv;

out vec4 fragColor;

void main() {
    vec4 colorOld = texture(texUnitOld, uv);
    vec4 colorNew = texture(texUnitNew, uv);

    // discard (almost) identical pixels
    if (distance(colorOld, colorNew) < 0.0001) {
        discard;
    }

    // not discarded -> different
    fragColor = vec4(1.0); 
}
