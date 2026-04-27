add_library(lookup_table INTERFACE LookUpTable.h)

target_include_directories(lookup_table INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/lookup_table>
)

#$<BUILD_INTERFACE:...> - используется при сборке проекта
#$<INSTALL_INTERFACE:...> - используется после установки (инсталляции) проекта