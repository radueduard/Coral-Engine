find_package(glm REQUIRED)
add_library(math
        src/math.ixx
        src/math.vector.ixx
        src/math.matrix.ixx
        src/math.quaternion.ixx
        src/math.rect.ixx
        src/math.frustum.ixx
        src/math.aabb.ixx
        src/math.transform.ixx
        src/math.constants.ixx
        src/math.data.ixx
        src/types.ixx)
target_link_libraries(math PUBLIC glm::glm)