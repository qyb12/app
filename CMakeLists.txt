cmake_minimum_required(VERSION 3.13)
project(lvgl-app LANGUAGES C CXX)

set(PROJECT_VERSION 1.0)

# 构建选项：选择显示驱动（SDL2 或 X11），强制默认值为 SDL2，避免无效值
set(LV_DRIVER "SDL2" CACHE STRING "Select LCGL display driver (SDL2/X11)")
if(NOT (LV_DRIVER STREQUAL "SDL2" OR LV_DRIVER STREQUAL "X11"))
    set(LV_DRIVER "SDL2" CACHE STRING "Select LCGL display driver (SDL2/X11)" FORCE)
    message(WARNING "Invalid LV_DRIVER, forced to SDL2")
endif()

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(BUILD_DIR ${WORKING_DIR}/obj)
set(BIN_DIR ${WORKING_DIR}/bin)
# set(UI_DIR ${SRC_DIR}/ui)

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

# 头文件包含路径（对应 Makefile 的 INC）
include_directories(
    ${SRC_DIR}/third_party/
    ${SRC_DIR}/third_party/lvgl/
    ${SRC_DIR}/third_party/lv_drivers/
    ${SRC_DIR}/base/
)

# 链接库（对应 Makefile 的 LDLIBS）
set(LIBS
    ${LV_DRIVER}
    pthread
    m
    # freetype avformat avcodec avutil swscale z  # 原 Makefile 注释项，如需启用可取消注释
)

# 搜索所有 C/C++ 源文件（排除隐藏目录）
file(GLOB_RECURSE SRCS
    ${SRC_DIR}/third_party/lvgl/src/*.c         # 显式包含 LVGL 核心源码
    ${SRC_DIR}/third_party/lvgl/demos/*.c
    ${SRC_DIR}/third_party/lv_drivers/*.c     # 若驱动在 LVGL 驱动目录，也添加
    ${SRC_DIR}/main/*.c
    ${SRC_DIR}/main/*.cpp
    LIST_DIRECTORIES false
)

# 构建可执行文件
add_executable(app ${SRCS})

# 链接依赖库
target_link_libraries(app PRIVATE ${LIBS})

# 依赖文件（确保配置文件修改时重新编译，对应 Makefile 的依赖项）
target_sources(app PRIVATE
    ${SRC_DIR}/third_party/lv_conf.h
    # ${SRC_DIR}/third_party/lv_drv_conf.h
    ${SRC_DIR}/third_party/lv_demo_conf.h
    ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt
)

# 安装规则（对应 Makefile 的 install 目标）
# install(TARGETS app
#     DESTINATION /usr/lib/${PROJECT_NAME}/bin
#     PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
# )
# install(DIRECTORY DESTINATION /usr/lib/${PROJECT_NAME}/bin)