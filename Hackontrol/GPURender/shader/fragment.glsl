#version 330 core

layout(location = 0) out vec4 color;

uniform float inputTime;
uniform vec2 inputSize;

// https://iquilezles.org/articles/palettes/
vec3 palette(float time) {
	vec3 a = vec3(0.5, 0.5, 0.5);
	vec3 b = vec3(0.5, 0.5, 0.5);
	vec3 c = vec3(1.0, 1.0, 1.0);
	vec3 d = vec3(0.263, 0.416, 0.557);
	return a + b * cos(6.28318 * (c * time + d));
}

// https://www.shadertoy.com/view/mtyGWy
void main() {
	vec2 uv = (gl_FragCoord.xy * 2.0 - inputSize) / inputSize.y;
	vec2 uvCopy = uv;
    vec3 finalColor = vec3(0.0, 0.0, 0.0);
    
    for(float i = 0.0; i < 3.0; i++) {
        uv = fract(uv * 1.5) - 0.5;
        float colorDistance = length(uv) * exp(-length(uvCopy));
        vec3 color = palette(length(uvCopy) + i * 0.4 + inputTime * 0.4);
        colorDistance = sin(colorDistance * 8.0 + inputTime) / 8.0;
        colorDistance = abs(colorDistance);
		colorDistance = pow(0.01 / colorDistance, 1.2);
        finalColor += color * colorDistance;
    }
    
	color = vec4(finalColor, 1.0);
}
