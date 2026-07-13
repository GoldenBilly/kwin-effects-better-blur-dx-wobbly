/*
    Provider-owned Better Wobbly mesh composite.
    The source coordinates refer to Better Blur DX's own final cache.
    SPDX-License-Identifier: GPL-2.0-or-later
*/
uniform sampler2D texUnit;
uniform vec2 textureSize;
uniform float opacity;

varying vec2 uv;

void main(void)
{
    vec2 cacheUv = vec2(uv.x / textureSize.x,
                        1.0 - uv.y / textureSize.y);
    vec2 halfTexel = vec2(0.5) / textureSize;
    cacheUv = clamp(cacheUv, halfTexel, vec2(1.0) - halfTexel);
    gl_FragColor = texture2D(texUnit, cacheUv) * opacity;
}
