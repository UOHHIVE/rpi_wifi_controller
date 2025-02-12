#! /usr/bin/env bash

# set vars
DIR_SRC="/root/src"
DIR_LOCAL="/root/local"

apt update -y
apt upgrade -y

# cd to src
cd "$DIR_SRC"

# Fetch the latest changes from the remote
git fetch

# Get the current branch name
current_branch=$(git rev-parse --abbrev-ref HEAD)

# Check if the current branch is behind the remote
behind=$(git rev-list --count "${current_branch}".."origin/${current_branch}")

if [ "$behind" -gt 0 ]; then
  echo "Updates available for '$current_branch'. Pulling changes..."
  git pull origin "$current_branch"
  git submodule update --init --recursive --remote
  bash "NO_CLONE=1 $DIR_LOCAL/setup.sh"
else
    echo "Branch '$current_branch' is up to date."
fi

sleep 30