#!/bin/bash
if [ -d "/usr/local/lib/python2.7" ]; then
	brew uninstall python@2
	rm -rf /usr/local/lib/python2.7
fi

brew install python@2
pip2 install appscript

clear
echo ' '
echo 'Building nexus-qt dmg file...'
echo '(Do not close or move windows until after this script says it finished.)'

echo 'export PATH="/usr/local/opt/python/libexec/bin:$PATH"' >> ~/.bash_profile
source ~/.bash_profile
python release/macdeploy/macdeployqtplus nexus-qt.app -dmg -fancy release/macdeploy/fancy.plist

echo ' '
echo 'Finished Building nexus-qt dmg file'
echo ' '
echo 'File can be found by opening Finder, clicking Go, then Home on'
echo 'the menu bar. Then open the Nexus folder and you should see'
echo 'a file called nexus-qt.dmg. Double click it and drag in the window'
echo 'that appears to the Applications folder and you are finished.'
echo ' '
