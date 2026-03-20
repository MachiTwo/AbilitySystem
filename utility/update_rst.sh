#!/bin/sh

usage () { echo -e "Usage: Call from the Godot source root to update Ability System rst class documentation."; }
msg () {  echo -e "$@"; }

set -e

if [ ! -d "${PWD}/modules/ability_system" ]; then
    usage
    exit 1
fi

rm -rf /tmp/rst
./doc/tools/make_rst.py --output /tmp/rst doc/tools doc/classes/@GlobalScope.xml modules/ability_system/doc_classes || /bin/true
msg "Removing old rst class documentation..."
rm -f ./modules/ability_system/doc/source/classes/class_*
msg "Copying new rst class documentation..."
cp -r -f /tmp/rst/class_* modules/ability_system/doc/source/classes/
msg "Cleaning up..."
rm -f modules/ability_system/doc/source/classes/class_@globalscope.rst
msg "Done!"
