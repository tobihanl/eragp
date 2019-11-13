target_sources(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Brain.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Brain.h
        ${CMAKE_CURRENT_LIST_DIR}/Entity.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Entity.h
        ${CMAKE_CURRENT_LIST_DIR}/FoodEntity.cpp
        ${CMAKE_CURRENT_LIST_DIR}/FoodEntity.h
        ${CMAKE_CURRENT_LIST_DIR}/LivingEntity.cpp
        ${CMAKE_CURRENT_LIST_DIR}/LivingEntity.h
        ${CMAKE_CURRENT_LIST_DIR}/Log.h
        ${CMAKE_CURRENT_LIST_DIR}/Main.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Matrix.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Matrix.h
        ${CMAKE_CURRENT_LIST_DIR}/Renderer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Renderer.h
        ${CMAKE_CURRENT_LIST_DIR}/Rng.h
        ${CMAKE_CURRENT_LIST_DIR}/Tile.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Tile.h
        ${CMAKE_CURRENT_LIST_DIR}/World.cpp
        ${CMAKE_CURRENT_LIST_DIR}/World.h
        )

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_IMAGE_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_TTF_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})
