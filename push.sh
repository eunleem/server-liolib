#!/bin/bash
BOLD=$(tput bold)
NORMAL=$(tput sgr0)

SERVER=lifeino.com
USERNAME=eunleem
LOCAL_ROOT_DIR=~/devel/liolib
REMOTE_ROOT_DIR=devel/liolib

usage() { echo "Usage: $0 [-s SERVER] [-u USERNAME] [-l LOCAL_SOURCE_DIR] [-r REMOTE_DEST_DIR]" 1>&2; exit 1; }

while getopts ":s:u:l:r:" opt; do
  case "$opt" in
    s)
      SERVER=$OPTARG
      ;;
    u)
      USERNAME=$OPTARG
      ;;
    l)
      LOCAL_ROOT_DIR=$OPTARG
      ;;
    r)
      REMOTE_ROOT_DIR=$OPTARG
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      usage
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      usage
      exit 1
      ;;
  esac
done


read -p "Would you like to push ${BOLD}$LOCAL_ROOT_DIR ${NORMAL}to ${BOLD}$USERNAME@$SERVER:$REMOTE_ROOT_DIR${NORMAL}? " -n 1 -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
  echo -e "\nPushing $LOCAL_ROOT_DIR to $USERNAME@$SERVER:$REMOTE_ROOT_DIR "

  rsync -aruv \
    --exclude '*.o' --exclude '*.exe' --exclude '*.log' \
    --exclude '.git' --exclude '.gitignore' \
    --exclude 'tags' --exclude '*.vim'  \
    --exclude '.*.swp' --exclude '.*.swo' --exclude '.backup' \
    --exclude '*.bk' --exclude '*.bak' \
    --exclude '*.a' --exclude 'gmon.out' \
    --exclude 'push.sh' \
    $LOCAL_ROOT_DIR/ \
    $USERNAME@$SERVER:$REMOTE_ROOT_DIR
else
  echo -e "\nCancelled."
fi



