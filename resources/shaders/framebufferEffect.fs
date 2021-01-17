#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

const float offset = 1.0 / 300.0;

void main() {
#if 0 // normal rendering
    vec3 col = texture(screenTexture, TexCoords).rgb;
    FragColor = vec4(col, 1.0);
#endif

#if 0 // inversion
    FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
#endif

#if 0
    FragColor = texture(screenTexture, TexCoords);
    float average = (0.2 * FragColor.r + 0.7 * FragColor.b + 0.07 * FragColor.b) / 3.0;
    FragColor = vec4(average, average, average, 1.0);
#endif
    // niz offset-a po kojima se krece od trenutnog fragmenta u svim smerovima
    vec2 offsets[9] = vec2[](
        vec2(-offset, offset),  // top-left
        vec2(0.0f, offset),     // top-center
        vec2(offset, offset),   // top-right
        vec2(-offset, 0.0f),    // bottom-left
        vec2(0.0f, 0.0f),       // center
        vec2(0.0f, offset),     // right
        vec2(-offset, -offset),
        vec2(0.0f, -offset),
        vec2(offset, -offset)
    );

    // kernel po kome se racuna novi piksel gornjim postupkom
    float kernel[9] = float[](
//           0.0, -1.0, 0.0,
//           -1.0, 5.0, -1.0,
//           0.0, -1.0, 0.0
//           0.0, 0.0, 0.0,
//           0.0, 1.0, 0.0,
//           0.0, 0.0, 0.0
        1.0 / 16, 2.0 / 16, 1.0 /16,
        2.0 / 16, 4.0 / 16, 2.0 /16,
        1.0 / 16, 2.0 / 16, 1.0 /16
    );

    // prolazim kroz okolne teksele -> smesta se u niz
    vec3 sampleTex[9];
    for (int i = 0; i < 9; ++i) {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }

    // svaku boju mnozim sa odgovarajucim elementom kernela -> sve saberem -> izlaz za ovaj fragment
    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; ++i) {
        col += sampleTex[i] * kernel[i];
    }
    FragColor = vec4(col, 1.0);
}
