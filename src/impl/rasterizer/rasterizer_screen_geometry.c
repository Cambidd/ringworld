#include "../math/color.h"
#include "../exception/exception.h"
#include "rasterizer_dx9.h"
#include "rasterizer_dx9_shader_effect.h"
#include "rasterizer_dx9_vertex.h"
#include "rasterizer_dx9_vertex_shader.h"
#include "rasterizer_dx9_texture.h"
#include "../render/render.h"
#include "rasterizer_screen.h"
#include "rasterizer_screen_geometry.h"


static void rasterizer_screen_geometry_hud_meter_draw(BitmapData **meter_maps, RasterizerMeterParams *meter_params, RasterizerDynamicVertex *vertices) {
    ASSERT(meter_params->tint_mode_2);
    ASSERT(meter_params->gradient == 1.0f);

    for(size_t i = 0; i < 3 && meter_maps[i] != NULL; i++) {
        rasterizer_dx9_texture_set_bitmap_data_directly(i, meter_maps[i]);
    }

    rasterizer_dx9_set_pixel_shader(NULL);
    rasterizer_dx9_set_render_state(D3DRS_ALPHATESTENABLE, TRUE);
    rasterizer_dx9_set_render_state(D3DRS_ALPHAREF, meter_params->gradient_min_color >> 0x18);
    rasterizer_dx9_set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
    rasterizer_dx9_set_render_state(D3DRS_SRCBLEND, D3DBLEND_ONE);
    rasterizer_dx9_set_render_state(D3DRS_DESTBLEND, D3DBLEND_ONE);
    rasterizer_dx9_set_render_state(D3DRS_BLENDOP, D3DBLENDOP_ADD);

    rasterizer_dx9_set_render_state(D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL);
    rasterizer_dx9_set_render_state(D3DRS_TEXTUREFACTOR, meter_params->gradient_min_color);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    rasterizer_dx9_set_texture_stage_state(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    rasterizer_dx9_set_texture_stage_state(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    rasterizer_dx9_draw_primitive_up(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(RasterizerDynamicVertex));

    rasterizer_dx9_set_render_state(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    rasterizer_dx9_set_render_state(D3DRS_TEXTUREFACTOR, meter_params->background_color);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    rasterizer_dx9_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    rasterizer_dx9_set_texture_stage_state(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    rasterizer_dx9_set_texture_stage_state(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    rasterizer_dx9_draw_primitive_up(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(RasterizerDynamicVertex));
}

void rasterizer_screen_geometry_draw(RasterizerDynamicScreenGeometryParams *params, RasterizerDynamicVertex *vertices) {
    IDirect3DDevice9 *device = rasterizer_dx9_device();
    RenderGlobals *render_globals = render_get_globals();
    RasterizerGlobals *rasterizer_globals = rasterizer_dx9_get_globals();
    D3DCAPS9 *device_caps = rasterizer_dx9_device_caps();

    ASSERT(device != NULL);
    ASSERT(render_globals != NULL);

    if(render_globals->time_delta_since_tick != 1) {
        return;
    }

    ASSERT(params != NULL);
    ASSERT(params->map[0] != NULL);
    ASSERT(!params->map[2] || params->map[1]);
    ASSERT(!params->map[1] || !params->meter_parameters);

    rasterizer_dx9_set_render_state(D3DRS_CULLMODE, D3DCULL_NONE);
    rasterizer_dx9_set_render_state(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
    rasterizer_dx9_set_render_state(D3DRS_ALPHABLENDENABLE, TRUE);
    rasterizer_dx9_set_render_state(D3DRS_ALPHATESTENABLE, TRUE);
    rasterizer_dx9_set_render_state(D3DRS_ALPHAREF, 1);
    rasterizer_dx9_set_render_state(D3DRS_ZENABLE, D3DZB_FALSE);
    rasterizer_dx9_set_render_state(D3DRS_FOGENABLE, FALSE);
    
    rasterizer_dx9_set_framebuffer_blend_function(params->framebuffer_blend_function);

    rasterizer_dx9_set_vertex_declaration(VERTEX_DECLARATION_DYNAMIC_SCREEN);

    uint32_t vertex_processing_method = rasterizer_dx9_vertex_get_processing_method(VERTEX_DECLARATION_DYNAMIC_SCREEN);
    bool software_processing = vertex_processing_method & D3DUSAGE_SOFTWAREPROCESSING != 0;
    IDirect3DDevice9_SetSoftwareVertexProcessing(device, software_processing);

    IDirect3DVertexShader9 *vertex_shader = rasterizer_dx9_shader_get_vertex_shader(VSH_SCREEN2);
    rasterizer_dx9_set_vertex_shader(vertex_shader);

    Rectangle2D *viewport_bounds = &render_globals->camera.viewport_bounds;
    float viewport_width = viewport_bounds->right - viewport_bounds->left;
    float viewport_height = viewport_bounds->top - viewport_bounds->bottom;
    float inv_screen_width = 2.0f / rasterizer_screen_get_width();
    float inv_screen_height = -2.0f / rasterizer_screen_get_height();

    float offset_x, offset_y;
    if(params->offset == NULL) {
        offset_x = 0.0f;
        offset_y = 0.0f;
    } 
    else {
        offset_x = params->offset->x * inv_screen_width;
        offset_y = params->offset->y * inv_screen_height;
    }

    VertexShaderScreenprojConstants screenproj = {0};
    screenproj.projection.x[0] = inv_screen_width;
    screenproj.projection.x[1] = 0.0f;
    screenproj.projection.x[2] = 0.0f;
    screenproj.projection.x[3] = -1.0f - 1.0f / viewport_width + offset_x;
    screenproj.projection.y[0] = 0.0f;
    screenproj.projection.y[1] = inv_screen_height;
    screenproj.projection.y[2] = 0.0f;
    screenproj.projection.y[3] = 1.0f + 1.0f / viewport_height + offset_y;
    screenproj.projection.z[0] = 0.0f;
    screenproj.projection.z[1] = 0.0f;
    screenproj.projection.z[2] = 0.0f;
    screenproj.projection.z[3] = 0.5f;
    screenproj.projection.w[0] = 0.0f;
    screenproj.projection.w[1] = 0.0f;
    screenproj.projection.w[2] = 0.0f;
    screenproj.projection.w[3] = 1.0f;
    screenproj.texture_scale.x = params->map_texture_scale[0].x;
    screenproj.texture_scale.y = params->map_texture_scale[0].y;

    ASSERT(rasterizer_dx9_set_vertex_shader_constant_f(VSH_CONSTANTS_SCREENPROJ_OFFSET, &screenproj, VSH_CONSTANTS_SCREENPROJ_COUNT));

    VertexShaderScreenproj2Constants screenproj2;
    screenproj2.screen_texture_scales_1 = params->map_texture_scale[1];
    screenproj2.screen_texture_scales_2 = params->map_texture_scale[2];
    
    if(params->map_anchor_screen[0]) { 
        screenproj2.screen_mask_0.x = 1.0f;
        screenproj2.screen_mask_0.y = 0.0f;
    } 
    else {
        screenproj2.screen_mask_0.x = 0.0f;
        screenproj2.screen_mask_0.y = 1.0f;
    }
    
    if(params->map_anchor_screen[1]) {
        screenproj2.screen_mask_1.x = 1.0f;
        screenproj2.screen_mask_1.y = 0.0f;
    } 
    else {
        screenproj2.screen_mask_1.x = 0.0f;
        screenproj2.screen_mask_1.y = 1.0f;
    }
    
    if(params->map_anchor_screen[2]) {
        screenproj2.screen_mask_2.x = 1.0f;
        screenproj2.screen_mask_2.y = 0.0f;
    } 
    else {
        screenproj2.screen_mask_2.x = 0.0f;
        screenproj2.screen_mask_2.y = 1.0f;
    }
    
    if(params->map_offset[0] != NULL) {
        screenproj2.screen_offset_0 = *params->map_offset[0];
    } 
    else {
        screenproj2.screen_offset_0.x = 0.0f;
        screenproj2.screen_offset_0.y = 0.0f;
    }
    
    if(params->map_offset[1] != NULL) {
        screenproj2.screen_offset_1 = *params->map_offset[1];
    } 
    else {
        screenproj2.screen_offset_1.x = 0.0f;
        screenproj2.screen_offset_1.y = 0.0f;
    }
    
    if(params->map_offset[2] != NULL) {
        screenproj2.screen_offset_2 = *params->map_offset[2];
    } 
    else {
        screenproj2.screen_offset_2.x = 0.0f;
        screenproj2.screen_offset_2.y = 0.0f;
    }
    
    screenproj2.screen_scale_0 = params->map_scale[0];
    screenproj2.screen_scale_1 = params->map_scale[1];
    screenproj2.screen_scale_2 = params->map_scale[2];

    ASSERT(rasterizer_dx9_set_vertex_shader_constant_f(VSH_CONSTANTS_SCREENPROJ2_OFFSET, &screenproj2, VSH_CONSTANTS_SCREENPROJ2_COUNT));

    for(size_t i = 0; i < 3 && params->map[i] != NULL; i++) {
        if(params->map_wrapped[i]) {
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        }
        else {
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        }

        if(params->point_sampled) {
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_MINFILTER, D3DTEXF_POINT);
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
        }
        else {
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
            rasterizer_dx9_set_sampler_state(i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
        }
    }
    
    if(params->meter_parameters != NULL) {
        rasterizer_screen_geometry_hud_meter_draw(params->map, params->meter_parameters, vertices);
        return;
    }

    RasterizerDx9ShaderEffect *effect = NULL;
    PixelShaderScreenGeometryConstants psh_constants = {0};

    if(params->map[0] != NULL) {
        ColorRGB *map_tint_0 = params->map_tint[0];
        if(map_tint_0 == NULL) {
            map_tint_0 = &color_rgb_white;
        }

        float map_fade_0;
        if(params->map_fade[0] != NULL) {
            map_fade_0 = *params->map_fade[0];
        }
        else {
            map_fade_0 = 1.0f;
        }

        rasterizer_dx9_set_sampler_state(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        rasterizer_dx9_set_sampler_state(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        
        // Draw the screen geometry with only one texture
        if(params->map[1] == NULL && params->map[2] == NULL) {
            ColorARGB texture_factor_real;
            texture_factor_real.a = map_fade_0;
            texture_factor_real.r = map_tint_0->r;
            texture_factor_real.g = map_tint_0->g;
            texture_factor_real.b = map_tint_0->b;
            
            ColorARGBInt texture_factor = color_encode_a8r8g8b8(&texture_factor_real);
            rasterizer_dx9_set_render_state(D3DRS_TEXTUREFACTOR, texture_factor);

            rasterizer_dx9_texture_set_bitmap_data_directly(0, params->map[0]);
            rasterizer_dx9_set_render_state(D3DRS_ALPHATESTENABLE, 0);
            rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
            rasterizer_dx9_set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            rasterizer_dx9_set_texture_stage_state(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            rasterizer_dx9_set_texture_stage_state(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
            rasterizer_dx9_set_texture_stage_state(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
            rasterizer_dx9_set_texture_stage_state(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            rasterizer_dx9_set_texture_stage_state(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            rasterizer_dx9_set_texture_stage_state(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            rasterizer_dx9_set_texture_stage_state(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            rasterizer_dx9_set_texture_stage_state(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            rasterizer_dx9_set_texture_stage_state(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            rasterizer_dx9_set_pixel_shader(NULL);
            rasterizer_dx9_draw_primitive_up(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(RasterizerDynamicVertex));
            IDirect3DDevice9_SetSoftwareVertexProcessing(device, rasterizer_globals->using_software_vertex_processing);
            return;
        }

        // If the D3D9 device doesn't support pixel shader 1.1, we can't draw the screen geometry with more than one texture
        if(device_caps->PixelShaderVersion < D3DPS_VERSION(1, 1)) {
            IDirect3DDevice9_SetSoftwareVertexProcessing(device, rasterizer_globals->using_software_vertex_processing);
            return;
        }

        ColorRGB *map_tint_1 = params->map_tint[1];
        if(map_tint_1 == NULL) {
            map_tint_1 = &color_rgb_white;
        }

        float map_fade_1;
        if(params->map_fade[1] != NULL) {
            map_fade_1 = *params->map_fade[1];
        }
        else {
            map_fade_1 = 1.0f;
        }

        ColorRGB *map_tint_2 = params->map_tint[2];
        if(map_tint_2 == NULL) {
            map_tint_2 = &color_rgb_white;
        }

        float map_fade_2;
        if(params->map_fade[2] != NULL) {
            map_fade_2 = *params->map_fade[2];
        }
        else {
            map_fade_2 = 1.0f;
        }

        psh_constants.map_tint_0 = *map_tint_0;
        psh_constants.map_fade_0 = map_fade_0;
        psh_constants.map_tint_1 = *map_tint_1;
        psh_constants.map_fade_1 = map_fade_1;
        psh_constants.map_tint_2 = *map_tint_2;
        psh_constants.map_fade_2 = map_fade_2;
        psh_constants.plasma_fade.r = params->plasma_fade.r;
        psh_constants.plasma_fade.g = params->plasma_fade.g;
        psh_constants.plasma_fade.b = params->plasma_fade.b;
        psh_constants.plasma_fade_alpha = params->plasma_fade.a;

        uint32_t shader_effect_index = 0;

        if(params->map[1] != NULL) {
            switch(params->map0_to_1_blend_function) {
                case 1:
                    shader_effect_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_DOT_SUBTRACT;
                    break;
                case 2: 
                    shader_effect_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_MULTIPLY2X_SUBTRACT;
                    break;
                case 3:
                    shader_effect_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_MULTIPLY_SUBTRACT;
                    break;
                case 4:
                    shader_effect_index = SHADER_EFFECT_SCREEN_MULTITEXTURE_ADD_SUBTRACT;
                    break;
            }
        }

        if(params->map[2] != NULL) {
            switch(params->map1_to_2_blend_function) {
                case 1:
                    shader_effect_index += 2;
                    break;
                case 2: 
                    shader_effect_index += 4;
                    break;
                case 3:
                    shader_effect_index += 3;
                    break;
                case 4:
                    shader_effect_index += 1;
                    break;
            }
        }

        effect = rasterizer_dx9_shader_effect_get(shader_effect_index);
    }

    if(device_caps->PixelShaderVersion >= D3DPS_VERSION(1, 1)) {
        if(effect != NULL) {
            for(size_t i = 0; i < 3 && params->map[i] != NULL; i++) {
                rasterizer_dx9_texture_set_bitmap_data_directly(i, params->map[i]);
            }
            rasterizer_dx9_set_pixel_shader(effect->pixel_shaders->pixel_shader);
            rasterizer_dx9_set_texture_stage_state(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
            rasterizer_dx9_set_texture_stage_state(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            rasterizer_dx9_set_pixel_shader_constant_f(PSH_CONSTANTS_SCREEN_OFFSET, &psh_constants, PSH_CONSTANTS_SCREEN_COUNT);
            rasterizer_dx9_draw_primitive_up(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(RasterizerDynamicVertex));
        }
    }
    else {
        ColorARGB texture_factor_real;
        texture_factor_real.a = psh_constants.map_fade_0;
        texture_factor_real.r = psh_constants.map_tint_0.r;
        texture_factor_real.g = psh_constants.map_tint_0.g;
        texture_factor_real.b = psh_constants.map_tint_0.b;
        ColorARGBInt texture_factor = color_encode_a8r8g8b8(&texture_factor_real);
        rasterizer_dx9_set_render_state(D3DRS_TEXTUREFACTOR, texture_factor);
        rasterizer_dx9_draw_primitive_up(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(RasterizerDynamicVertex));
    }

    IDirect3DDevice9_SetSoftwareVertexProcessing(device, rasterizer_globals->using_software_vertex_processing);
}
