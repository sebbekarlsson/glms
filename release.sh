#!/bin/bash

NOTES=$(git --no-pager log --decorate=short --pretty=oneline -n1 --no-color)
CHANGELOG="> $NOTES"

echo $CHANGELOG > changelog.md

mkdir -p build
cd build && cmake .. && make -j8 && \
    mkdir -p release && \
    cp glms_e release/. && \
    cp *.a release/. && \
    cp *.so release/. && \
    tar -czf release.tar.gz release/*


RELEASE_VERSION=$(date +%s)

gh release create $RELEASE_VERSION -F ../changelog.md release.tar.gz
