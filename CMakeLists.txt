cmake_minimum_required(VERSION 3.13)
project(lvgl-app LANGUAGES C CXX)
set(PROJECT_VERSION 1.0)

# 设置C++17标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 必须支持C++17，否则报错
set(CMAKE_CXX_EXTENSIONS OFF)        # 禁用GNU扩展（可选，如需严格标准）

# 构建选项：选择显示驱动（SDL2 或 X11），强制默认值为 SDL2，避免无效值
set(LV_DRIVER "SDL2" CACHE STRING "Select LCGL display driver (SDL2/X11)")
# if(NOT (LV_DRIVER STREQUAL "SDL2" OR LV_DRIVER STREQUAL "X11"))
#     set(LV_DRIVER "SDL2" CACHE STRING "Select LCGL display driver (SDL2/X11)" FORCE)
#     message(WARNING "Invalid LV_DRIVER, forced to SDL2")
# endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(BUILD_DIR ${WORKING_DIR}/obj)
set(BIN_DIR ${WORKING_DIR}/bin)

# 输出目录配置
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BUILD_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

# 编译选项（添加 -Wno-error 禁止警告转错误，确保 make 不中断）
set(CMAKE_C_FLAGS "-O0 -g")  # 关键修改：添加 -Wno-error
set(WARNINGS
    -Wall -Wextra
    -Wshadow -Wundef -Wmaybe-uninitialized -Wmissing-prototypes -Wno-discarded-qualifiers
    -Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized
    -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default
    -Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated
    -Wempty-body -Wshift-negative-value -Wstack-usage=2048
    -Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith
)
string(JOIN " " WARNINGS_STR ${WARNINGS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNINGS_STR}")

# 宏定义（对应 Makefile 的 DEFINES）
if(LV_DRIVER STREQUAL "SDL2")
    add_definitions(-DUSE_SDL)
else()
    add_definitions(-DUSE_X11)
endif()
add_definitions(-DSIMULATOR=1 -DLV_BUILD_TEST=0)

set(INCLUDE_DIRS    # 定义所有头文件目录
    ${SRC_DIR}/third_party/
    ${SRC_DIR}/third_party/lvgl/
    ${SRC_DIR}/third_party/lv_drivers/
    ${SRC_DIR}/base/
    ${SRC_DIR}/module/
)

set(LIBS    # 链接库（对应 Makefile 的 LDLIBS）
    ${LV_DRIVER}
    pthread
    m
)

file(GLOB_RECURSE SRCS  # 搜索所有 C/C++ 源文件（排除隐藏目录）
    ${SRC_DIR}/third_party/lvgl/*.c         # 显式包含 LVGL 核心源码
    ${SRC_DIR}/third_party/lv_drivers/*.c     # 若驱动在 LVGL 驱动目录，也添加
    ${SRC_DIR}/main/*.c
    ${SRC_DIR}/main/*.cpp
    ${SRC_DIR}/module/*.cpp
    ${SRC_DIR}/ui/*.cpp
    LIST_DIRECTORIES false
)

add_executable(app ${SRCS}) # 构建可执行文件

target_include_directories(app PRIVATE ${INCLUDE_DIRS}) # 指定头文件搜索目录
target_link_libraries(app PRIVATE ${LIBS})  # 链接依赖库
target_sources(app PRIVATE  # 依赖文件（确保配置文件修改时重新编译，对应 Makefile 的依赖项）
    ${SRC_DIR}/third_party/lv_conf.h
    ${SRC_DIR}/third_party/lv_demo_conf.h
    ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt
)

# ========== 拷贝图片资源到 build 目录 ==========
# 定义源图片目录（项目中存放图片的位置）
set(IMAGE_SRC_DIR ${SRC_DIR}/ui/asset/image)
# 定义目标图片目录（编译后图片要拷贝到的位置，这里和可执行文件同目录）
set(IMAGE_DST_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ui/asset/image)

# 拷贝图片（每次编译自动同步）
add_custom_command(
    TARGET app POST_BUILD  # 编译完 app 后执行
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${IMAGE_SRC_DIR}    # 源：项目中的图片目录
        ${IMAGE_DST_DIR}    # 目标：build/bin/ui/asset/image
    COMMENT "Copying image resources to build directory..."
)

# 安装规则（对应 Makefile 的 install 目标）
# install(TARGETS app
#     DESTINATION /usr/lib/${PROJECT_NAME}/bin
#     PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
# )
# install(DIRECTORY DESTINATION /usr/lib/${PROJECT_NAME}/bin)