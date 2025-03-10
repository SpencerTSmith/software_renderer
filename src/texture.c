#include "texture.h"

#include <stdio.h>
#include <string.h>

#include <stb/stb_image.h>

#include "display.h"
#include "vector.h"

void texture_free(texture_t *texture) {
    free(texture->pixels);

    memset(texture, 0, sizeof(texture_t));
}

void load_png_texture_data(texture_t *texture, const char *filename) {
    int channels;
    stbi_uc *bytes = stbi_load(filename, &texture->width, &texture->height, &channels, 4);

    if (bytes == NULL) {
        fprintf(stderr, "Error in STB loading texture data");
        return;
    }

    int texture_size = texture->width * texture->height;
    texture->pixels = (color_t *)malloc(texture_size * sizeof(color_t));

    if (texture == NULL) {
        fprintf(stderr, "Error allocating memory for mesh texture");
        return;
    }

    for (int i = 0, j = 0; i < texture_size; i++, j += 4) {
        texture->pixels[i].r = bytes[j];
        texture->pixels[i].g = bytes[j + 1];
        texture->pixels[i].b = bytes[j + 2];
        texture->pixels[i].a = bytes[j + 3];
    }

    stbi_image_free(bytes);
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p) {
    // We get to reuse these from the derivation, only 2 crosses needed as well
    vec2_t ac = vec2_sub(c, a);
    vec2_t ab = vec2_sub(b, a);

    vec2_t pc = vec2_sub(c, p);
    vec2_t pb = vec2_sub(b, p);

    vec2_t ap = vec2_sub(p, a);

    float ac_cross_ab = vec2_cross(ac, ab);
    float pc_cross_pb = vec2_cross(pc, pb);
    float ac_cross_ap = vec2_cross(ac, ap);

    float alpha = pc_cross_pb / ac_cross_ab;
    alpha = alpha > 1.0f ? 1.0f : alpha < 0.0f ? 0.0f : alpha;

    float beta = ac_cross_ap / ac_cross_ab;
    beta = beta > 1.0f ? 1.0f : beta < 0.0f ? 0.0f : beta;

    float gamma = 1.0f - alpha - beta;
    gamma = gamma > 1.0f ? 1.0f : gamma < 0.0f ? 0.0f : gamma;

    vec3_t weights = {alpha, beta, gamma};
    return weights;
}

void draw_affine_texel(int x, int y, vec2_t a, vec2_t b, vec2_t c, tex2_t a_uv, tex2_t b_uv,
                       tex2_t c_uv, const texture_t *texture) {
    vec2_t p = {x, y};
    vec3_t weights = barycentric_weights(a, b, c, p);
    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    tex2_t interp = {
        .u = a_uv.u * alpha + b_uv.u * beta + c_uv.u * gamma,
        .v = a_uv.v * alpha + b_uv.v * beta + c_uv.v * gamma,
    };

    int tex_x = (int)fabsf(roundf(interp.u * texture->width)) % texture->width;
    int tex_y = (int)fabsf(roundf(interp.v * texture->height)) % texture->height;

    int index = tex_y * texture->width + tex_x;

    draw_pixel(x, y, texture->pixels[index]);
}

void draw_texel(int x, int y, vec4_t a, vec4_t b, vec4_t c, tex2_t a_uv, tex2_t b_uv, tex2_t c_uv,
                const texture_t *texture) {
    vec2_t p = {x, y};
    vec2_t a_2 = vec4_to_vec2(a);
    vec2_t b_2 = vec4_to_vec2(b);
    vec2_t c_2 = vec4_to_vec2(c);

    vec3_t weights = barycentric_weights(a_2, b_2, c_2, p);
    float alpha = weights.x;
    float beta = weights.y;
    float gamma = weights.z;

    // 1/z is related linearly to the point p, original z is saved in w so 1/w

    // interpolated U/w and V/w, perspective divide over the interpolated point
    tex2_t interp_uv = {
        .u = (a_uv.u / a.w) * alpha + (b_uv.u / b.w) * beta + (c_uv.u / c.w) * gamma,
        .v = (a_uv.v / a.w) * alpha + (b_uv.v / b.w) * beta + (c_uv.v / c.w) * gamma,
    };

    // we also need to interpolate 1/w for our current pixel
    float interp_inv_w = (1 / a.w) * alpha + (1 / b.w) * beta + (1 / c.w) * gamma;

    // now "undo"
    interp_uv.u /= interp_inv_w;
    interp_uv.v /= interp_inv_w;

    // Modulo is hacky clamp
    int tex_x = (int)fabsf(roundf(interp_uv.u * texture->width));
    tex_x = tex_x % texture->width;
    int tex_y = (int)fabsf(roundf(interp_uv.v * texture->height));
    tex_y = tex_y % texture->height;

    int index = tex_y * texture->width + tex_x;

    // Only draw the pixel if depth value is greater (closer) than already there
    // Remember 1/w will grow bigger when z is lower (closer)
    if (interp_inv_w > w_buffer_at(x, y)) {
        draw_pixel(x, y, texture->pixels[index]);
        update_w_buffer(x, y, interp_inv_w);
    }
}
