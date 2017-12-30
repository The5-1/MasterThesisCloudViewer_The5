// This file is part of Surface Splatting.
//
// Copyright (C) 2010, 2015 by Sebastian Lipponer.
// 
// Surface Splatting is free software: you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Surface Splatting is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Surface Splatting. If not, see <http://www.gnu.org/licenses/>.

#version 330

#define MULTISAMPLING  0
#define SMOOTH         0



uniform mat4 projection_matrix_inv;
uniform vec4 viewport;

uniform float material_shininess;



uniform sampler2D color_texture;
uniform sampler2D normal_texture;
uniform sampler2D depth_texture;

#if SMOOTH
    vec3 lighting(vec3 normal_eye, vec3 v_eye, vec3 color, float shininess)
{
    const vec3 light_eye = vec3(0.0, 0.0, 1.0);

    float dif = max(dot(light_eye, normal_eye), 0.0);
    vec3 refl_eye = reflect(light_eye, normal_eye);

    vec3 view_eye = normalize(v_eye);
    float spe = pow(clamp(dot(refl_eye, view_eye), 0.0, 1.0),
            shininess);
    float rim = pow(1.0 + dot(normal_eye, view_eye), 3.0);

    vec3 res = 0.15 * color;
    res += 0.6 * dif * color;
    res += 0.1 * spe * vec3(1.0);
    res += 0.1 * rim * vec3(1.0);

    return res;
}
#endif


    in vec2 texture_uv;


#define FRAG_COLOR 0
layout(location = FRAG_COLOR) out vec4 frag_color;

void main()
{
    vec4 res = vec4(0.0);
#if MULTISAMPLING
    ivec2 itexture_uv = ivec2(textureSize(color_texture) * texture_uv);

    for (int i = 0; i < 4; ++i)
#endif
    {
    #if MULTISAMPLING
        vec4 pixel = texelFetch(color_texture, itexture_uv, i);

        #if SMOOTH
        vec3 normal = normalize(
            texelFetch(normal_texture, ivec2(itexture_uv), i).xyz
            );
        float depth = texelFetch(depth_texture, ivec2(itexture_uv), i).r;
        #endif
    #else
        vec4 pixel = texture(color_texture, texture_uv);

        #if SMOOTH
        vec3 normal = normalize(texture(normal_texture, texture_uv).xyz);
        float depth = texture(depth_texture, texture_uv).r;
        #endif
    #endif

        if (pixel.a > 0.0)
        {
            #if SMOOTH
            vec4 p_ndc = vec4(
                2.0 * (gl_FragCoord.xy - viewport.xy) / (viewport.zw) - 1.0,
                (2.0 * depth - gl_DepthRange.near - gl_DepthRange.far)
                / gl_DepthRange.diff, 1.0
                );

            vec4 v_eye = projection_matrix_inv * p_ndc;
            v_eye = v_eye / v_eye.w;

            res += vec4(lighting(normal, v_eye.xyz,
                pixel.rgb / pixel.a, material_shininess), 1.0);
            #else
                res += vec4(pixel.rgb / pixel.a, 1.0f);
            #endif
        }
        else
        {
            res += vec4(1.0);
        }
    }

    #if MULTISAMPLING
        frag_color = sqrt(res / 4.0);
    #else
        frag_color = sqrt(res);
    #endif
}
