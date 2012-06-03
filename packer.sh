#!/bin/bash

rm index.html

files=( head.html menu.html downloads.html desc.html build.html license.html bug.html screenshots.html bottom.html )
for file in ${files[@]}
do
#    echo "<!-- $file -->" >> index.html
    cat $file >> index.html
    echo "" >> index.html
done
