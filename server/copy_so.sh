#!/bin/bash

TARGET="PaperServer"
STATIC_LIB_DIR="/usr/local/lib"

echo ">>> 1. 正在复制动态库..."
# 解析 ldd 输出，提取绝对路径，过滤掉非 / 开头的行
# cp -L 表示跟随符号链接，复制真正的文件
ldd "$TARGET" | awk '{print $3}' | grep "^/" | sort -u | while read lib; do
    if [ -f "$lib" ]; then
        echo "复制: $(basename $lib)"
        cp -L "$lib" .
    fi
done

echo ""
echo ">>> 2. 正在复制 Poco 静态库..."
# 注意：静态库路径无法自动反推，这里使用了你之前提供的 Poco 路径列表
STATIC_LIBS=(
    "libPocoFoundation.a"
    "libPocoNet.a"
    "libPocoUtil.a"
    "libPocoJSON.a"
    "libPocoData.a"
    "libPocoDataSQLite.a"
)

for lib in "${STATIC_LIBS[@]}"; do
    src="$STATIC_LIB_DIR/$lib"
    if [ -f "$src" ]; then
        echo "复制: $lib"
        cp "$src" .
    else
        echo "警告: 静态库 $src 不存在，跳过。"
    fi
done

echo ""
echo ">>> 复制完成！"
