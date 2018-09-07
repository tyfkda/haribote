#!/bin/sh

USER_NAME=dev
USER_UID=${LOCAL_UID:-9001}
USER_GID=${LOCAL_GID:-9001}

groupadd --non-unique --gid $USER_GID $USER_NAME
useradd --non-unique --create-home --uid $USER_UID --gid $USER_GID $USER_NAME

export HOME=/home/$USER_NAME

exec gosu $USER_NAME "$@"
