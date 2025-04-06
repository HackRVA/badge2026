#!/bin/bash

# this script is intended to run from the root of the repo

docker build -t hackrva/badge2025-user-docs -f ./tools/user-docs.dockerfile .

