target_sources(${PROJECT_NAME}
        PUBLIC
        Entity.cpp
        Entity.h
        FoodEntity.cpp
        FoodEntity.h
        LivingEntity.cpp
        LivingEntity.h
        Main.cpp
        Matrix.cpp
        Matrix.h
        Renderer.cpp
        Renderer.h
        World.cpp
        World.h
        )

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_IMAGE_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_TTF_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARY} ${SDL2_TTF_LIBRARY})