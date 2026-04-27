add_library(random_attack_strategy
            include/RandomAttackStrategy.h
            src/RandomAttackStrategy.cpp)
target_include_directories(random_attack_strategy PUBLIC
                           include
                           ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(random_attack_strategy attack_strategy)
