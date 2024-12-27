#include <ringworld/rasterizer/rasterizer_geometry_group.h>
#include <ringworld/rasterizer/rasterizer_dx9.h>

#include "../exception/exception.h"

extern uint16_t *dynamic_vertices;

uint16_t rasterizer_get_dynamic_vertex_buffer_type(uint16_t dynamic_vertex_index) {
    ASSERT(rasterizer_dx9_device());
    ASSERT(dynamic_vertex_index != -1);
    return dynamic_vertices[dynamic_vertex_index * 8];
}

uint16_t rasterizer_get_vertex_buffer_type(TransparentGeometryGroup *group) {
    ASSERT(group);
    if(group->vertex_buffers == NULL) {
        ASSERT(group->dynamic_vertex_buffer_index == -1);
        return rasterizer_get_dynamic_vertex_buffer_type(group->dynamic_vertex_buffer_index);
    }
    return group->vertex_buffers->type;
}
