cmake_minimum_required(VERSION 3.10)
project(PaperServer)

# 设置 C++ 标准为 17 (代码中使用了 std::filesystem, std::optional 等)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==========================================
# 1. 查找 OpenCV (apt 安装)
# ==========================================
# OpenCV 动态库通常位于 /usr/lib/x86_64-linux-gnu
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "Found OpenCV: ${OpenCV_VERSION} (${OpenCV_DIR})")
endif()

# ==========================================
# 2. 查找 SQLite3
# ==========================================
# Poco::Data::SQLite 依赖 sqlite3，静态链接 Poco 时需要显式链接它
find_package(SQLite3 REQUIRED)
if(SQLITE3_FOUND)
    message(STATUS "Found SQLite3: ${SQLITE3_INCLUDE_DIRS} ${SQLITE3_LIBRARIES}")
endif()

# ==========================================
# 3. 查找 Poco (静态编译)
# ==========================================
# 尝试自动查找。如果未找到 CMake 配置文件，则使用手动配置路径。
# 你提到的静态库路径为 /usr/local/lib

find_package(Poco QUIET COMPONENTS Foundation Net Util JSON Data DataSQLite)

if (NOT Poco_FOUND)
    message(STATUS "Poco CMake config not found, using manual configuration for static libs in /usr/local/lib...")
    
    # 手动设置 Poco 变量
    set(Poco_FOUND TRUE)
    # 假设头文件在 /usr/local/include
    set(Poco_INCLUDE_DIRS "/usr/local/include")
    
    # 手动指定静态库文件列表
    # 注意：链接顺序很重要，Foundation 通常放在最后
    set(Poco_LIBRARIES 
        /usr/local/lib/libPocoNet.a
        /usr/local/lib/libPocoUtil.a
        /usr/local/lib/libPocoJSON.a
        /usr/local/lib/libPocoData.a
        /usr/local/lib/libPocoDataSQLite.a
        /usr/local/lib/libPocoFoundation.a
    )
    
    # 添加头文件搜索路径
    include_directories(${Poco_INCLUDE_DIRS})
    # 添加库文件搜索路径 (虽然上面用了绝对路径，但加上以防万一)
    link_directories(/usr/local/lib)
else()
    message(STATUS "Found Poco automatically.")
endif()

# ==========================================
# 4. 源文件设置
# ==========================================
set(SOURCES
    main.cpp
    paper_server.cpp
    image_controller.cpp
    token_store.cpp
)

set(HEADERS
    paper_server.h
    image_controller.h
    token_store.h
)

# ==========================================
# 5. 构建可执行文件
# ==========================================
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# 添加 OpenCV 头文件路径
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})

# ==========================================
# 6. 链接库
# ==========================================

# 1. OpenCV 库
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# 2. Poco 库
if (TARGET Poco::Foundation)
    # 如果 CMake 找到了 Poco 的 Target，使用 Target 方式链接
    target_link_libraries(${PROJECT_NAME}
        Poco::Foundation
        Poco::Net
        Poco::Util
        Poco::JSON
        Poco::Data
        Poco::DataSQLite
    )
else()
    # 否则使用手动指定的静态库列表链接
    target_link_libraries(${PROJECT_NAME} ${Poco_LIBRARIES})
endif()

# 3. SQLite3 库 (必须链接，否则 Poco::Data 会报未定义引用)
if(TARGET SQLite3::SQLite3)
    target_link_libraries(${PROJECT_NAME} SQLite3::SQLite3)
else()
    target_link_libraries(${PROJECT_NAME} ${SQLITE3_LIBRARIES})
endif()

# 4. 系统线程库 和动态加载库 (Poco 依赖)
target_link_libraries(${PROJECT_NAME} pthread dl)

# 5. std::filesystem 补丁 (GCC < 9.0 需要显式链接 stdc++fs)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
    message(STATUS "Linking stdc++fs for filesystem support on older GCC")
    target_link_libraries(${PROJECT_NAME} stdc++fs)
endif()
