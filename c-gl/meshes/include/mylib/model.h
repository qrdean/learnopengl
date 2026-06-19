#ifndef MODEL_H
#define MODEL_H

#include "stb_image.h"
#include "mesh.h"
#include "cgltf.h"
#include <stddef.h>

typedef struct Model {
  Mesh *meshes;
  uint32_t numMeshes; 
  char *directory;
} Model;

Model loadModel(const char *path);
void modelDraw(Model m, struct Shader shader);
// void processNode(aiNode *node, const aiScene *scene);
// Mesh processMesh(aiMesh *mesh, const aiScene *scene);
// Texture loadMaterialTextures(aiMaterial *mat, aiTextureType type, const char* typeName)
// {}
//
#define LOAD_ATTRIBUTE(accesor, numComp, srcType, dstPtr) \
{ \
  int n = 0; \
  srcType *buffer = (srcType *)accesor->buffer_view->buffer->data + accesor->buffer_view->offset/sizeof(srcType) + accesor->offset/sizeof(srcType);\
  for (unsigned int k = 0; k < accesor->count; k++) \
  { \
    for (int l = 0; l < numComp; l++) \
    {\
      dstPtr[numComp*k + l] = (srcType)buffer[n + l];\
    }\
    n += (int)(accesor->stride/sizeof(srcType));\
  } \
}

void modelDraw(Model m, struct Shader shader)
{
  for(unsigned int i = 0; i < m.numMeshes; i++)
  {
    Draw(&m.meshes[i], shader);
  }
}

Model loadModel(const char* filename)
{
  Model model = {0};

  int filesize = 0;
  unsigned char *fileData = LoadFileData(filename, &filesize);

  cgltf_options options = {0};
  cgltf_data* data = NULL;
  cgltf_result result = cgltf_parse(&options, fileData, filesize, &data);
  if (result == cgltf_result_success)
  {
    if (data->file_type == cgltf_file_type_glb) printf("model: [%s] Model basic data (glb) loaded\n", filename);
    else if(data->file_type == cgltf_file_type_gltf) printf("model: [%s] Model basic data (gltf) loaded\n", filename);

    printf(" > Mesh count: %zu\n", data->meshes_count);
    printf(" > Materials count: %zu\n", data->materials_count);
    printf(" > Buffers count: %zu\n", data->buffers_count);
    printf(" > Images count: %zu\n", data->images_count);
    printf(" > Textures count: %zu\n", data->textures_count);

    printf("data loaded success %s\n", filename);
    result = cgltf_load_buffers(&options, data, filename);
    if (result != cgltf_result_success) printf("model: [%s] failed to load mesh/material buffers", filename);

    int primitives_count = 0;
    for (unsigned int i = 0; i < data->nodes_count; i++)
    {
      cgltf_node *node = &(data->nodes[i]);
      cgltf_mesh *mesh = node->mesh;
      if (!mesh) continue;

      for (unsigned int p = 0; p < mesh->primitives_count; p++)
      {
        if (mesh->primitives[p].type == cgltf_primitive_type_triangles) primitives_count++;
      }
    }

    printf(" > Prim count based on hierarchy: %i\n", primitives_count);

    model.numMeshes = primitives_count;
    model.meshes = (Mesh*)calloc(model.numMeshes, sizeof(Mesh));
    printf("num mesh %i\n", primitives_count);
    // printf("sizeof %lu\n", sizeof(model.meshes));

    Vertex vertices = {0};
    model.meshes->vertices = &vertices;

    // data->meshes_count
    //
    //
    // Loading material datao
    
    // for (unsigned int i = 0, j = 1; i < data->materials_count; i++, j++)
    // {
    //   Texture texture = {0}; 
    //   // const char *texPath = filename;
    //   // if (data->materials[i].has_pbr_metallic_roughness) {
    //   //   if (data->materials[i].pbr_metallic_roughness.base_color_texture.texture)
    //   //   {
    //   //     model.meshes[j].textures->type = DIFFUSE;
    //   //     model.meshes[j].textures->id = DIFFUSE;
    //   //   }
    //   // }
    // }

    // Loading Mesh data

    int meshIndex = 0;
    for (unsigned int i = 0; i < data->nodes_count; i++)
    {
      cgltf_node *node = &(data->nodes[i]);
      cgltf_mesh *mesh = node->mesh;
      if (!mesh) continue;

      // materials
      int material_count = (int)data->materials_count;
      int images_count = (int)data->images_count;
      printf(" > materials count: %i\n", material_count);
      printf(" > images count: %i\n", images_count);
      TextureDecode decoded_textures[images_count];

      char *loaded_images[images_count];
      for (uint32_t m = 0; m < images_count; m++) {
        printf("raw image: %s\n", data->images[m].name);
        printf("uri image: %s\n", data->images[m].uri);
        TextureDecode decoded_texture = {0};


        if (data->images[m].buffer_view) {
          uint8_t *dd = (uint8_t*)data->images[m].buffer_view->buffer->data + data->images[m].buffer_view->offset;
          decoded_texture.pixels = stbi_load_from_memory(dd, (int)data->images[m].buffer_view->size, &decoded_texture.width, &decoded_texture.height, &decoded_texture.channels, 0);
        } else if(data->images[m].uri){
          int skip = 0;
          for(uint32_t ll = 0; ll < images_count; ll++)
          {
            if (loaded_images[images_count] != NULL) {
              if (strcmp(loaded_images[images_count], data->images[m].uri)) {
                skip = 1;
                break;
              }
            } else {
              loaded_images[images_count] = data->images[m].uri;
              break;
            } 
          }

          if (!skip) {
            uint8_t *dd = (uint8_t*)data->images[m].buffer_view->buffer->data + data->images[m].buffer_view->offset;
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "assets/%s", data->images[m].uri);
            decoded_texture.pixels = stbi_load(full_path, &decoded_texture.width, &decoded_texture.height, &decoded_texture.channels, 0);
          }
        }

        decoded_textures[m] = decoded_texture;
        // printf("loaded: %s\n", x_loaded);
        // image_p = data->images[m].uri;
      }

      model.meshes[meshIndex].numTextures = images_count;
      Texture *textures = calloc(images_count, sizeof(Texture));
      for (unsigned int tt = 0; tt < images_count; tt++)
      {
        unsigned int textureId = SetupTexture(decoded_textures[tt]);
        Texture tex = {0};
        tex.type = decoded_textures[tt].type;
        tex.id = textureId;
        textures[tt] = tex;
      }
      model.meshes[meshIndex].textures = textures;

      // for (uint32_t m = 0; m < material_count; m++)
      // {
      //   if (data->materials[m].has_pbr_metallic_roughness) {
      //     cgltf_image *image = data->materials[m].pbr_metallic_roughness.base_color_texture.texture->image;
      //     printf("pbr metallic roughness: images [%s]\n", image->uri);
      //   }
      //
      //   if (data->materials[m].has_pbr_specular_glossiness) {
      //     cgltf_image *specimage = data->materials[m].pbr_specular_glossiness.specular_glossiness_texture.texture->image;
      //     cgltf_image *diffimage = data->materials[m].pbr_specular_glossiness.diffuse_texture.texture->image;
      //     printf("pbrspec: images [%s]\n", specimage->uri);
      //     printf("pbrdiff: images [%s]\n", diffimage->uri);
      //   }
      //
      //   if (data->materials[m].has_pbr_metallic_roughness) {
      //   }
      //   if (data->materials[m].has_diffuse_transmission) {
      //     cgltf_image *image = data->materials[m].diffuse_transmission.diffuse_transmission_texture.texture->image;
      //     printf("diffuse: images [%s]\n", image->uri);
      //   }
      //
      //   if (data->materials[m].has_specular) {
      //     cgltf_image *image = data->materials[m].specular.specular_texture.texture->image;
      //     printf("specular: image [%s]\n", image->uri);
      //   }
      // }

      for (uint32_t p = 0; p < mesh->primitives_count; p++)
      {
        if (mesh->primitives[p].type != cgltf_primitive_type_triangles) continue;
        // setup indices
        if (mesh->primitives[p].indices != NULL)
        {
          cgltf_accessor *index_accessor = mesh->primitives[p].indices;
          
          uint32_t index_count = (uint32_t)index_accessor->count;
          unsigned short *index_buff = (unsigned short *)malloc(index_count*sizeof(unsigned short));
          // uint32_t *indices = calloc(256, sizeof(uint32_t));

          cgltf_size indices_unpacked = cgltf_accessor_unpack_indices(index_accessor, index_buff, sizeof(unsigned short), index_count);
          if (indices_unpacked < index_accessor->count) {
            printf("ERROR: only unpacked %zu out of %u", indices_unpacked, index_count);
            break;
          }

          model.meshes[meshIndex].numIndices = index_count;
          // for (uint32_t i = 0; i < index_count; i++)
          // {
          //   model.meshes[meshIndex].indices[i] = index_buff[i];
          // }
          model.meshes[meshIndex].indices = index_buff;
          // free(index_buff);
        }

        cgltf_accessor *pos_accessor = NULL;
        Vertex *tempVert = NULL;
        for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
        {
          if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_position)
          {
            pos_accessor = mesh->primitives[p].attributes[j].data;
            break;
          }
        }

        // Vertices Positions

        if (pos_accessor == NULL) continue;
        else
        {
          uint32_t vertex_count = (uint32_t)pos_accessor->count;
          float *positions = (float *)malloc(vertex_count*3*sizeof(float));
          tempVert = calloc(vertex_count, sizeof(Vertex));

          for (unsigned int v = 0; v < vertex_count; v++)
          {
            tempVert[v].Normal[0] = 1.0f;
            tempVert[v].Normal[1] = 0.0f;
            tempVert[v].Normal[2] = 0.0f;
            tempVert[v].TexCoords[0] = 0.0f;
            tempVert[v].TexCoords[1] = 0.0f;
          }

          cgltf_size vertices_unpacked = cgltf_accessor_unpack_floats(pos_accessor, positions, vertex_count * 3);
          if (vertices_unpacked < vertex_count)
          {
            printf("ERROR: only unpacked %zu out of %u", vertices_unpacked, vertex_count);
            break;
          }

          for (unsigned int i = 0; i < vertex_count; i++)
          {
            unsigned int idx = i * 3;
            tempVert[i].Position[0] = positions[idx];
            tempVert[i].Position[1] = positions[idx + 1];
            tempVert[i].Position[2] = positions[idx + 2];
          }

          model.meshes[meshIndex].numVertices = vertex_count;
          free(positions);
        }

        cgltf_accessor *normal_accessor = NULL;
        for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
        {
          if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_normal)
          {
            normal_accessor = mesh->primitives[p].attributes[j].data;
            break;
          }
        }

        if (normal_accessor != NULL)
        {
          uint32_t norm_count = (uint32_t)normal_accessor->count;
          float *normals = malloc(norm_count*3*sizeof(float));
          cgltf_size norms_unpacked = cgltf_accessor_unpack_floats(normal_accessor, normals, norm_count * 3);
          if (norms_unpacked < norm_count) 
          {
            printf("ERROR: only unpacked %zu out of %u", norms_unpacked, norm_count);
            break;
          }

          for (unsigned int i = 0; i < norm_count; i++)
          {
            unsigned int idx = i * 3;
            tempVert[i].Normal[0] = normals[idx];
            tempVert[i].Normal[1] = normals[idx + 1];
            tempVert[i].Normal[2] = normals[idx + 2];
          }

          free(normals);
        }

        cgltf_accessor *texcoords_accessor = NULL;
        for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
        {
          if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_texcoord)
          {
            texcoords_accessor = mesh->primitives[p].attributes[j].data;
          }
        }

        if (texcoords_accessor != NULL) 
        {
          if (texcoords_accessor->type == cgltf_type_vec2)
          {
            uint32_t texcoord_count = (uint32_t)texcoords_accessor->count;
            float *texcoords = malloc(texcoord_count*2*sizeof(float));
            cgltf_size texcoords_unpacked = cgltf_accessor_unpack_floats(texcoords_accessor, texcoords, texcoord_count * 2);
            if (texcoords_unpacked < texcoord_count)
            {
              printf("ERROR: only unpacked %zu out of %u", texcoords_unpacked, texcoord_count);
              break;
            }

            for (unsigned int i = 0; i < texcoord_count; i++)
            {
              unsigned int idx = i * 2;  
              tempVert[i].TexCoords[0] = texcoords[idx]; 
              tempVert[i].TexCoords[1] = texcoords[idx + 1]; 
            }
            free(texcoords);
          }
        }

        model.meshes[meshIndex].vertices = tempVert;
        setupMesh(&model.meshes[meshIndex]);
        free(tempVert);
        meshIndex++;
      }
      // free(indices);

      // for (unsigned int p = 0; p < mesh->primitives_count; p++)
      // {
      //   Vertex *vertices = (Vertex*)calloc(mesh->primitives_count, sizeof(Vertex));
      //   if (mesh->primitives[p].type != cgltf_primitive_type_triangles) continue;
      //   for (unsigned int j = 0; j < mesh->primitives[p].attributes_count; j++)
      //   {
      //     if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_position)
      //     {
      //       printf("loading positions\n");
      //       cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;
      //       if (model.meshes[meshIndex].vertices != NULL) printf("vertices attribute data alreadyloaded\n");
      //       else
      //       {
      //         model.meshes[meshIndex].numVertices = (int)attribute->count;
      //         float *tempVert = (float *)malloc(attribute->count*3*sizeof(float));
      //         LOAD_ATTRIBUTE(attribute, 3, float, tempVert); 
      //
      //         // Vertex *vertices = model.meshes[meshIndex].vertices;
      //         Vertex *vertices = (Vertex *)calloc(attribute->count, sizeof(Vertex));
      //         for (unsigned int k = 0; k < attribute->count; k++)
      //         {
      //           vec3 vt = {tempVert[3*k], tempVert[3*k+1], tempVert[3*k+2]};
      //           vertices->Position[3*k] = vt[0];  
      //           vertices->Position[3*k+1] = vt[1];  
      //           vertices->Position[3*k+2] = vt[2];  
      //         }
      //         model.meshes[meshIndex].vertices = vertices;
      //         free(tempVert);
      //         free(vertices);
      //       }
      //     }
      //     else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_normal) 
      //     {
      //       printf("loading normals\n");
      //       cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;
      //       // model.meshes[meshIndex].numVertices = (int)attribute->count;
      //       float *tempNorm = (float *)malloc(attribute->count*3*sizeof(float));
      //       LOAD_ATTRIBUTE(attribute, 3, float, tempNorm); 
      //
      //       Vertex *vertices = model.meshes[meshIndex].vertices;
      //       for (unsigned int k = 0; k < attribute->count; k++)
      //       {
      //         vec3 nt = {tempNorm[3*k], tempNorm[3*k+i], tempNorm[3*k+2]};
      //         vertices->Normal[0] = nt[0];  
      //         vertices->Normal[1] = nt[1];  
      //         vertices->Normal[2] = nt[2];  
      //       }
      //       free(tempNorm);
      //     }
      //     else if (mesh->primitives[p].attributes[j].type == cgltf_attribute_type_texcoord) 
      //     {
      //       cgltf_accessor *attribute = mesh->primitives[p].attributes[j].data;
      //       // model.meshes[meshIndex].numVertices = (int)attribute->count;
      //       if (attribute->type == cgltf_type_vec2)
      //       {
      //         printf("loading textures\n");
      //         float *texcoordPtr = (float *)malloc(attribute->count*2*sizeof(float));
      //         LOAD_ATTRIBUTE(attribute, 2, float, texcoordPtr); 
      //
      //         Vertex *vertices = model.meshes[meshIndex].vertices;
      //         for (unsigned int k = 0; k < attribute->count; k++)
      //         {
      //           vec2 tc = {texcoordPtr[2*k], texcoordPtr[2*k+i]};
      //           vertices->TexCoords[0] = tc[0];  
      //           vertices->TexCoords[1] = tc[1];  
      //         }
      //         free(texcoordPtr);
      //       } else {
      //         Vertex *vertices = model.meshes[meshIndex].vertices;
      //         vertices->TexCoords[0] = 0.0f;
      //         vertices->TexCoords[1] = 0.0f;
      //       }
      //     } 
      //     // attribute->buffer_view->buffer->data;
      //
      //     // vec3{attribute-}
      //     // model.meshes[meshIndex].vertices = (float *)malloc(attribute->count*3*sizeof(float));
      //   }
      //
      //   if (mesh->primitives[p].indices->buffer_view != NULL)
      //   {
      //     cgltf_accessor *attribute = mesh->primitives[p].indices;
      //
      //     model.meshes[meshIndex].numIndices = (int)attribute->count/3;
      //
      //     model.meshes[meshIndex].indices = (unsigned int *)malloc(attribute->count*sizeof(unsigned int *));
      //     LOAD_ATTRIBUTE(attribute, i , unsigned int, model.meshes[meshIndex].indices);
      //   }
      //
      //
      //   // for (unsigned int m = 0; data->materials; m++)
      //   // {
      //   //   if (&data->materials[m] == mesh->primitives[p].material)
      //   //   {
      //   //     break;
      //   //   }
      //   // }
      //   meshIndex++;
      // }
    }

    cgltf_free(data);
  }
  else 
  {
    printf("\ndid not load from %s\n", filename);
  }

  printf("\nsuccessfully initialied %s\n", filename);
  return model;
}

#endif
