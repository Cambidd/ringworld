#include <stdint.h>
#include <d3d9.h>

#include <ringworld/rasterizer/rasterizer_dx9.h>

#include "../exception/exception.h"

extern IDirect3DDevice9 ***d3d9_device;
extern PixelShader *pixel_shaders;
extern VertexShader *vertex_shaders;
extern short *vertex_shader_permutations;
extern VertexDeclaration *vertex_declarations;

IDirect3DDevice9 *rasterizer_dx9_device(void) {
    return **d3d9_device;
}

IDirect3DPixelShader9 *rasterizer_dx9_get_pixel_shader(uint16_t index) {
    ASSERT(index < PIXEL_SHADER_FUNCTIONS_MAX);
    return pixel_shaders[index].shader;
}

IDirect3DVertexShader9 *rasterizer_dx9_get_vertex_shader(uint16_t index) {
    ASSERT(index < VERTEX_SHADER_FUNCTIONS_MAX);
    return vertex_shaders[index].shader;
}

IDirect3DVertexShader9 *rasterizer_dx9_get_vertex_shader_for_permutation(uint16_t vertex_shader_permutation, uint16_t vertex_buffer_type) {
    return rasterizer_dx9_get_vertex_shader(vertex_shader_permutations[vertex_shader_permutation + vertex_buffer_type * 6]);
}

void rasterizer_dx9_set_vertex_shader(IDirect3DVertexShader9 *vertex_shader) {
    ASSERT(**d3d9_device);
    IDirect3DDevice9_SetVertexShader(**d3d9_device, vertex_shader);
}

IDirect3DVertexDeclaration9 *rasterizer_dx9_get_vertex_declaration(uint16_t vertex_buffer_type) {
    ASSERT(vertex_buffer_type < VERTEX_SHADER_DECLARATIONS_MAX);
    return vertex_declarations[vertex_buffer_type].declaration;
}

void rasterizer_dx9_set_vertex_declaration(uint16_t vertex_buffer_type) {
    ASSERT(**d3d9_device);
    IDirect3DDevice9_SetVertexDeclaration(**d3d9_device, rasterizer_dx9_get_vertex_declaration(vertex_buffer_type));
}

void rasterizer_dx9_set_pixel_shader(IDirect3DPixelShader9 *pixel_shader) {
    ASSERT(**d3d9_device);
    IDirect3DDevice9_SetPixelShader(**d3d9_device, pixel_shader);
}

void rasterizer_dx9_set_render_state(D3DRENDERSTATETYPE state, DWORD value) {
    ASSERT(**d3d9_device);
    IDirect3DDevice9_SetRenderState(**d3d9_device, state, value);
}

void rasterizer_dx9_set_sampler_state(uint16_t sampler, D3DSAMPLERSTATETYPE type, DWORD value) {
    ASSERT(**d3d9_device);
    IDirect3DDevice9_SetSamplerState(**d3d9_device, sampler, type, value);
}
