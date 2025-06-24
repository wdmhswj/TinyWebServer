#!/bin/bash

# Git自动提交推送脚本

# 获取提交消息参数
MESSAGE="$1"

# 检查是否为Git仓库
if ! git rev-parse --is-inside-work-tree &>/dev/null; then
    echo "❌ 当前目录不是Git仓库" >&2
    exit 1
fi

# 获取当前分支名
branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
if [ $? -ne 0 ] || [ -z "$branch" ]; then
    echo "❌ 无法获取当前分支" >&2
    exit 1
fi
echo "📌 当前分支: $branch"

# 如果没有提供提交消息，则使用时间戳
if [ -z "$MESSAGE" ]; then
    timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    MESSAGE="自动提交: $timestamp"
    echo "📝 使用自动生成的提交消息: \"$MESSAGE\""
fi

# 执行 git pull
echo "⬇️ 正在执行: git pull origin $branch"
git pull origin "$branch"
if [ $? -ne 0 ]; then
    echo "⚠️ git pull 操作失败，可能存在冲突"
    read -p "是否继续执行？(y/n): " choice
    if [ "$choice" != "y" ]; then
        exit 1
    fi
fi

# 检查是否有变更
if [ -z "$(git status --porcelain)" ]; then
    echo "✅ 当前分支无变更，跳过提交和推送"
    exit 0
fi

# 执行 git add
echo "➕ 正在执行: git add --all"
git add --all
if [ $? -ne 0 ]; then
    echo "❌ git add 操作失败" >&2
    exit 1
fi

# 执行 git commit
echo "🗒️ 正在执行: git commit -m \"$MESSAGE\""
git commit -m "$MESSAGE"
if [ $? -ne 0 ]; then
    echo "❌ git commit 操作失败" >&2
    exit 1
fi

# 执行 git push
echo "⬆️ 正在执行: git push origin $branch"
git push origin "$branch"
if [ $? -ne 0 ]; then
    echo "❌ git push 操作失败" >&2
    exit 1
fi

echo "🎉 所有操作已完成！"
