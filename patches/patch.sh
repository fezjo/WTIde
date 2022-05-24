#!/bin/bash

case $1 in
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
*)
    echo "Usage: $0 [create|apply|restore|abort-broken]"
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

get_branch() {
    echo $(git branch --show-current)
}

get_ahead_count() {
    echo $(git rev-list --count origin/$1..)
}

create_patches() {
    project=$1
    cd lib/$project
    is_git || return 1
    patch_dir=../../patches/$project
    mkdir -p $patch_dir
    branch=$(get_branch)
    count=$(get_ahead_count $branch)
    git format-patch -N -$count -o $patch_dir
    rmdir $patch_dir
}

apply_patches() {
    project=$1
    cd lib/$project
    git am --keep-cr --ignore-date ../../patches/$project/*
}

abort_broken_patches() {
    project=$1
    cd lib/$project
    git am --abort
}

restore_to_origin() {
    project=$1
    cd lib/$project
    is_git || return 1
    branch=$(git branch --show-current)
    count=$(get_ahead_count $branch)
    [[ $count -eq 0 ]] && return 0
    git checkout origin/$branch
    cd ..
    git add $project
    cd $project
    git checkout $branch
}

root_path=$(pwd)
projects=$(cd lib; ls -d */)
echo $projects
for project in $projects; do
    echo "Processing $project"
    echo $(pwd)
    ( $project_func $project )
    cd $root_path
done