#!/bin/bash
if [ ! -d "/usr/local/lib/python2.7/site-packages/appscript-1.0.1.dist-info" ]; then
	brew install python
	pip2 install appscript
fi

echo ' '
echo 'Building nexus-qt dmg file...'
echo '(Do not close or move windows until after this script says it finished.)'

export PATH="/usr/local/opt/python/libexec/bin:$PATH"
python release/macdeploy/macdeployqtplus nexus-qt.app -dmg -fancy release/macdeploy/fancy.plist

echo ' '
echo Finished Building nexus-qt dmg file
echo ' '
