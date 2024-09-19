#include "resource.h"
#include "platform.h"
#include "endian.h"
#include "memory.h"
#include "vector.h"
#include "log.h"
#include "format.h"

#define SOURCE_NAMESPACE core.resource

static InlineList _resource_list_free = INLINE_LIST_INIT(_resource_list_free);
static InlineList _resource_list_inactive = INLINE_LIST_INIT(_resource_list_inactive);
static InlineList _resource_list_active = INLINE_LIST_INIT(_resource_list_active);
static Vector(struct LoadedResource *) _resource_list = VECTOR_INIT;
static uint32_t _resource_id = 1, _resource_gen = 1;

void LoadedResource_free(struct LoadedResource * resource) {
  switch(resource->type) {
  case ResourceType_image:
    GraphicsImage_unload(&resource->image);
    break;
  default:
    break;
  }

  InlineList_remove_self(&resource->list);
  InlineList_push(&_resource_list_free, &resource->list);
  _resource_list.data[resource->id - 1] = NULL;
}

void LoadedResource_gc(void) {
  struct LoadedResource * resource, * next_resource;
  INLINE_LIST_EACH_CONTAINER_SAFE(&_resource_list_inactive, resource, next_resource, list) {
    LoadedResource_free(resource);
  }
  InlineList_push_list(&_resource_list_inactive, &_resource_list_active);
  _resource_gen++;
}


static struct LoadedResource * _resource_allocate(enum ResourceType type) {
  struct LoadedResource * resource;
  if(InlineList_is_empty(&_resource_list_free)) {
    resource = memory_alloc(sizeof(*resource), alignof(*resource));
    InlineList_init(&resource->list);
    resource->id = _resource_id++;

    Vector_space_for(&_resource_list, 1);
    *Vector_push(&_resource_list) = NULL;
  } else {
    resource = INLINE_LIST_CONTAINER(InlineList_pop(&_resource_list_free), struct LoadedResource, list);
  }
  InlineList_push(&_resource_list_active, &resource->list);
  resource->type = type;
  resource->gen = _resource_gen;
  _resource_list.data[resource->id - 1] = resource;
  return resource;
}

void LoadedResource_touch(struct LoadedResource * resource) {
  if(resource->gen != _resource_gen) {
    InlineList_remove_self(&resource->list);
    InlineList_push(&_resource_list_active, &resource->list);
    resource->gen = _resource_gen;
  }
}

struct LoadedResource * LoadedResource_from_image(struct Image *img) {
  struct LoadedResource * resource;

  if(img->resource == NULL || img->resource_id != img->resource->id) {
    char path[1024];
    format_string(path, sizeof(path), "assets/%s", img->path);

    resource = _resource_allocate(ResourceType_image);
    GraphicsImage_load(&resource->image, path); 
    img->resource = resource;
    img->resource_id = resource->id;
  } else {
    LoadedResource_touch(img->resource);
  }

  return img->resource;
}

struct LoadedResource * LoadedResource_from_id(uint32_t id) {
  return _resource_list.data[id - 1];
}


