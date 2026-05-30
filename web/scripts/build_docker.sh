#!/bin/bash

# this script is intended to run from the root of the repo

# if [ -d ./build_wasm ]; then
  # echo "The directory ./build_wasm already exists."
  # echo "Please remove it before running this script again."
  # exit 1
# fi

docker build --platform linux/amd64 -t hackrva/badge2026-web -f ./web/deployments/Dockerfile .


# run:
# docker run -it -p 8080:8080 hackrva/badge2026-user-docs

# deploy
# docker push hackrva/badge2026

