#!/bin/bash

PATCH_DIR=patches
LIB_DIR=lib

case $1 in
log)
    project_func=git_log
    ;;
status)
    project_func=repo_status
    ;;
create)
    project_func=create_patches
    ;;
apply)
    project_func=apply_patches
    ;;
abort-broken)
    project_func=abort_broken_patches
    ;;
restore)
    project_func=restore_to_origin
    ;;
custom)
    project_func=exec_custom
    custom_func=${@:2}
    ;;
*)
    echo "Usage: $0 [log|status|create|apply|restore|abort-broken|custom...]"
    exit 1
    ;;
esac

is_git() {
    if [ ! -e .git ]; then
        echo "Error: $project is not a git repository"
        return 1
    fi
    return 0
}

get_patch_dir() {
    echo ../../$PATCH_DIR/$1
}

get_origin_head() {
    cat $(get_patch_dir $1)/info
}

get_ahead_count() {
    git rev-list --count $1..
}

git_log() {
    project=$1
    cd $LIB_DIR/$project
    git log --pretty=oneline --abbrev-commit --graph
}

repo_status() {
    project=$1
    cd $LIB_DIR/$project
    is_git || return 1
    origin_commit=$(get_origin_head $project)
    ahead=$(get_ahead_count $origin_commit)
    echo "Project: $project ($origin_commit..) - $ahead patches ahead"
}

create_patches() {
    project=$1
    patch_dir=$(get_patch_dir $project)
    cd $LIB_DIR/$project
    is_git || return 1
    origin_commit=$(get_origin_head $project)
    ahead=$(get_ahead_count $origin_commit)
    mkdir -p $patch_dir
    git format-patch -N -$ahead -o $patch_dir
    rmdir $patch_dir
}

apply_patches() {
    project=$1
    patch_dir=$(get_patch_dir $project)
    restore_to_origin $project || return 1
    git am --keep-cr --ignore-date $patch_dir/*.patch
}

abort_broken_patches() {
    project=$1
    cd $LIB_DIR/$project
    is_git || return 1
    git am --abort
}

restore_to_origin() {
    project=$1
    patch_dir=$(get_patch_dir $project)
    cd $LIB_DIR/$project
    is_git || return 1
    origin_commit=$(cat $patch_dir/info)
    git fetch
    git checkout $origin_commit
}

exec_custom() {
    project=$1
    cd $LIB_DIR/$project
    eval "$custom_func"
}

root_path=$(pwd)
# projects=$(cd $LIB_DIR; ls -d */)
projects=$(cd $PATCH_DIR; ls -d */)
echo $projects
for project in $projects; do
    echo "Processing $project"
    ( $project_func $project )
    echo --------------------
    cd $root_path
done