#include "res_mesh.h"

#include <render/render.h>

#include "mesh_ddf.h"

namespace dmGameSystem
{
    // TODO: will be replaced when we have a proper mesh compiler
    struct MeshVertex
    {
        float x, y, z;
        float nx, ny, nz;
        float u, v;
    };

    void CopyVertexData(dmMeshDDF::MeshDesc* mesh_desc, float* vertex_buffer);

    dmResource::CreateResult ResCreateMesh(dmResource::HFactory factory,
                                     void* context,
                                     const void* buffer, uint32_t buffer_size,
                                     dmResource::SResourceDescriptor* resource,
                                     const char* filename)
    {
        dmMeshDDF::MeshDesc* mesh_desc;
        dmDDF::Result e = dmDDF::LoadMessage(buffer, buffer_size, &dmMeshDDF_MeshDesc_DESCRIPTOR, (void**) &mesh_desc);
        if ( e != dmDDF::RESULT_OK )
        {
            return dmResource::CREATE_RESULT_UNKNOWN;
        }

        Mesh* mesh = new Mesh();

        dmGraphics::VertexElement ve[] =
        {
                {0, 3, dmGraphics::TYPE_FLOAT, 0, 0},
                {1, 3, dmGraphics::TYPE_FLOAT, 0, 0},
                {2, 2, dmGraphics::TYPE_FLOAT, 0, 0}
        };
        mesh->m_VertexDeclaration = dmGraphics::NewVertexDeclaration(ve, 3);

        // TODO: move this bit to the mesh compiler
        uint32_t vertex_count = 0;
        for (uint32_t i = 0; i < mesh_desc->m_Components.m_Count; ++i)
        {
            vertex_count += mesh_desc->m_Components[i].m_Positions.m_Count / 3;
            assert(vertex_count * 3 == mesh_desc->m_Components[i].m_Normals.m_Count);
            assert(mesh_desc->m_Components[i].m_Texcoord0.m_Count == 0 || vertex_count * 2 == mesh_desc->m_Components[i].m_Texcoord0.m_Count);
        }
        mesh->m_VertexBuffer = dmGraphics::NewVertexBuffer(vertex_count * sizeof(MeshVertex), 0x0, dmGraphics::BUFFER_USAGE_STATIC_DRAW);
        mesh->m_VertexCount = vertex_count;
        float* vertex_buffer = (float*)dmGraphics::MapVertexBuffer(mesh->m_VertexBuffer, dmGraphics::BUFFER_ACCESS_WRITE_ONLY);
        CopyVertexData(mesh_desc, vertex_buffer);
        dmGraphics::UnmapVertexBuffer(mesh->m_VertexBuffer);
        dmDDF::FreeMessage(mesh_desc);

        resource->m_Resource = (void*) mesh;

        return dmResource::CREATE_RESULT_OK;
    }

    dmResource::CreateResult ResDestroyMesh(dmResource::HFactory factory,
                                      void* context,
                                      dmResource::SResourceDescriptor* resource)
    {
        Mesh* mesh = (Mesh*)resource->m_Resource;
        dmGraphics::DeleteVertexDeclaration(mesh->m_VertexDeclaration);
        dmGraphics::DeleteVertexBuffer(mesh->m_VertexBuffer);
        delete mesh;

        return dmResource::CREATE_RESULT_OK;
    }

    dmResource::CreateResult ResRecreateMesh(dmResource::HFactory factory,
            void* context,
            const void* buffer, uint32_t buffer_size,
            dmResource::SResourceDescriptor* resource,
            const char* filename)
    {
        dmMeshDDF::MeshDesc* mesh_desc;
        dmDDF::Result e = dmDDF::LoadMessage(buffer, buffer_size, &dmMeshDDF_MeshDesc_DESCRIPTOR, (void**) &mesh_desc);
        if ( e != dmDDF::RESULT_OK )
        {
            return dmResource::CREATE_RESULT_UNKNOWN;
        }

        Mesh* mesh = (Mesh*)resource->m_Resource;
        uint32_t vertex_count = 0;
        for (uint32_t i = 0; i < mesh_desc->m_Components.m_Count; ++i)
        {
            vertex_count += mesh_desc->m_Components[i].m_Positions.m_Count / 3;
            assert(vertex_count * 3 == mesh_desc->m_Components[i].m_Normals.m_Count);
            assert(mesh_desc->m_Components[i].m_Texcoord0.m_Count == 0 || vertex_count * 2 == mesh_desc->m_Components[i].m_Texcoord0.m_Count);
        }
        dmGraphics::SetVertexBufferData(mesh->m_VertexBuffer, vertex_count * sizeof(MeshVertex), 0x0, dmGraphics::BUFFER_USAGE_STATIC_DRAW);
        mesh->m_VertexCount = vertex_count;
        float* vertex_buffer = (float*)dmGraphics::MapVertexBuffer(mesh->m_VertexBuffer, dmGraphics::BUFFER_ACCESS_WRITE_ONLY);
        CopyVertexData(mesh_desc, vertex_buffer);
        dmGraphics::UnmapVertexBuffer(mesh->m_VertexBuffer);

        dmDDF::FreeMessage(mesh_desc);

        return dmResource::CREATE_RESULT_OK;
    }

    void CopyVertexData(dmMeshDDF::MeshDesc* mesh_desc, float* vertex_buffer)
    {
        MeshVertex* v = (MeshVertex*)vertex_buffer;
        uint32_t vi = 0;
        for (uint32_t i = 0; i < mesh_desc->m_Components.m_Count; ++i)
        {
            dmMeshDDF::MeshComponent& comp = mesh_desc->m_Components[i];
            uint32_t vertex_count = comp.m_Positions.m_Count / 3;
            for (uint32_t j = 0; j < vertex_count; j++)
            {
                v[vi].x = comp.m_Positions.m_Data[j*3+0];
                v[vi].y = comp.m_Positions.m_Data[j*3+1];
                v[vi].z = comp.m_Positions.m_Data[j*3+2];

                v[vi].nx = comp.m_Normals.m_Data[j*3+0];
                v[vi].ny = comp.m_Normals.m_Data[j*3+1];
                v[vi].nz = comp.m_Normals.m_Data[j*3+2];

                if (comp.m_Texcoord0.m_Count)
                {
                    v[vi].u = comp.m_Texcoord0.m_Data[j*2+0];
                    v[vi].v = comp.m_Texcoord0.m_Data[j*2+1];
                }
                ++vi;
            }
        }

    }
}
