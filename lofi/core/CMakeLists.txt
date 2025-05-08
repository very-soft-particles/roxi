set(LOFI_INTERNAL_INCLUDE_PATH
  "${CMAKE_CURRENT_LIST_DIR}/include/")

add_subdirectory(include)

set(ECS_PATH "${LOFI_INTERNAL_INCLUDE_PATH}/ecs")

set(LOFI_HEADERS
  "${LOFI_INTERNAL_INCLUDE_PATH}l_definition.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_base.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_memory.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_meta.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_string.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_sync.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_thread_pool.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_tuple.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_variant.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_vocab.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_map.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_job.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_fiber.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_file.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_container.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_allocator.hpp"
  "${LOFI_INTERNAL_INCLUDE_PATH}l_arena.hpp"
  "{ECS_PATH}/l_ecs.hpp"
  "{ECS_PATH}/l_entity.hpp"
  "{ECS_PATH}/l_component.hpp"
  "{ECS_PATH}/l_system.hpp"

  # using potentially optimized compile time dispatch for faster compiling
  # ${LOFI_META_VOCAB}
)


