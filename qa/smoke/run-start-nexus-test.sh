#!/bin/bash

# Copyright 2017 The Nexus Core Developers
# Initial Author: Bryan Gmyrek <bryangmyrekcom@gmail.com>
# License: GPL v2

# Script to test starting nexus on the command line
#  ./run-start-nexus-test.sh
#
# To auto-kill any existing nexus procs and delete the .lock file use
#  NEXUS_TEST_AUTOKILL=1 ./run-start-nexus-test.sh
#
# To clear out all test data and start over, run.
#  NEXUS_TEST_CLEAR=1 ./run-start-nexus-test.sh
#
# To set up the test/debug environment and then exit without starting do.
#  NEXUS_TEST_SETUP_ONLY=1 ./run-start-nexus-test.sh
#   This will set up the conf file, etc and drop a shell script in the test dir
#    which you can run to start nexus. This can also be useful if you
#    want grab the nexus run command from the .sh script for use in
#    e.g. an IDE step-through debugger.
#
# To set nexus up to be run in a step-through debugger, use.
#   NEXUS_TEST_NO_DAEMON=1 ./run-start-nexus-test.sh
#
# This is the command i've been using to test using a debugger:
#  NEXUS_TEST_CLEAR=1 NEXUS_TEST_SETUP_ONLY=1 NEXUS_TEST_NO_DAEMON=1 ./run-start-nexus-test.sh
#  Then I run nexus from the debugger itself.

set -o pipefail  # trace ERR through pipes
set -o errtrace  # trace ERR through 'time command' and other functions
set -o nounset   # set -u : exit the script if you try to use an uninitialised variable
set -o errexit   # set -e : exit the script if any statement returns a non-true return value
error() {
  echo "Error on or near line ${1}: ${2:-}; exiting with status ${3:-1}"
  exit "${3:-1}"
}
trap 'error ${LINENO}' ERR
set -v
set -x

NEXUS=/home/dev/code/Nexus/nexus
NEXUS_DATADIR_BASE=${HOME}/nexustest
NEXUS_TEST_TESTNET=1
NEXUS_DATADIR=${NEXUS_DATADIR_BASE}/testnet
NEXUS_CONF=${NEXUS_DATADIR}/nexus.conf
NEXUS_DEBUG_LOG=${NEXUS_DATADIR}/testnet25/debug.log
NEXUS_LOCK=${NEXUS_DATADIR}/testnet25/.lock
NEXUS_START_SH=${NEXUS_DATADIR}/run-test-nexus.sh
NEXUS_TEST_STORAGE=${NEXUS_DATADIR_BASE}/persiststorage
NEXUS_TEST_STORAGE_LLD=${NEXUS_TEST_STORAGE}/LLD

if [[ "${NEXUS_TEST_CLEAR:-}" == 1 ]]
then
    rm -rf ${NEXUS_DATADIR}
else
    echo "The existing test datadir will be reused if it exists already."
fi

if  ls $NEXUS_DATADIR 2>&1 >/dev/null
then
    echo "Data directory already existed."
else
    mkdir -p $NEXUS_DATADIR
    cd $NEXUS_DATADIR
    if [[ "${NEXUS_TEST_TESTNET:-}" != 1 ]]
    then
        echo "Setting Up Nexus blockchain bootstrap file."
        if -e ${NEXUS_TEST_STORAGE_LLD}
        then
            cp ${NEXUS_TEST_STORAGE_LLD}/recent.rar ${NEXUS_DATADIR}
        else
            echo "Have to download it, this could take a while. It will be cached for next time."
            mkdir -p ${NEXUS_TEST_STORAGE_LLD}
            cd ${NEXUS_TEST_STORAGE_LLD}
            which wget || sudo apt-get -y install wget
            LLD_BOOTSTRAP="recent.rar"
            ls ${LLD_BOOTSTRAP} ||  wget http://nexusearth.com/bootstrap/LLD-Database/${LLD_BOOTSTRAP}
            cp ${LLD_BOOTSTRAP} ${NEXUS_DATADIR}
            cd ${NEXUS_DATADIR}
            unrar x ${LLD_BOOTSTRAP}
            rm -f ${LLD_BOOTSTRAP}
        fi
    fi
fi

if ls $NEXUS_LOCK
then
    echo "nexus .lock file existed - view the output of ps to see if it's still running:"
    ps auxww | egrep nexu[s] || grep -v run-start-nexus || true
    if [[ "${NEXUS_TEST_AUTOKILL:-}" != "1" ]]; then
        while true; do
            read -p "Do you want to kill ALL(!!!) running nexus (if there are any) and blow away the .lock file?" yn
            case $yn in
                [Yy]* ) echo "OK!"; break;;
                [Nn]* ) echo "Bye!"; exit;;
                * ) echo "Please answer yes or no.";;
            esac
        done
    fi
    killall nexus || true
    echo "Sleeping for 5 seconds to ensure nexus processes finish."
    sleep 5
    rm -f $NEXUS_LOCK
fi

echo "Writing/ensuring nexus.conf exists."

if ! ls $NEXUS_CONF 2>&1 >/dev/null
then
    echo "testnet=1" >> ${NEXUS_CONF}
    echo "regtest=1" >> ${NEXUS_CONF}
    echo "istimeseed=1" >> ${NEXUS_CONF}
    echo "debug=1" >> ${NEXUS_CONF}
    echo "verbose=4" >> ${NEXUS_CONF}
    echo "unified=0" >> ${NEXUS_CONF}
    echo "llpallowip=*:18325" >> ${NEXUS_CONF}
    echo "llpallowip=*.*.*.*:18325" >> ${NEXUS_CONF}
    echo "llpallowip=127.0.0.1:18325" >> ${NEXUS_CONF}
    echo "mining=1" >> ${NEXUS_CONF}
    echo "server=1" >> ${NEXUS_CONF}
    echo "rpcuser=nxsnode1" >> ${NEXUS_CONF}
    echo "rpcpassword=nxsnode1pw" >> ${NEXUS_CONF}
    echo "rpcallowip=127.0.0.1" >> ${NEXUS_CONF}
    echo "rpcallowip=*.*.*.*" >> ${NEXUS_CONF}
    echo "rpcport=19336" >> ${NEXUS_CONF}
    echo "port=18313" >> ${NEXUS_CONF}
    if [[ "${NEXUS_TEST_NO_DAEMON:-}" == "1" ]]
    then
        # NOTE - daemon=0 is important if you want to run nexus in a debugger.
        #        otherwise you might want this to be 1
        echo "daemon=0" >> ${NEXUS_CONF}
    fi
    echo "listen=1" >> ${NEXUS_CONF}
    echo "stake=0" >> ${NEXUS_CONF}
    echo "checklevel=0" >> ${NEXUS_CONF}
    echo "checkbolcks=1" >> ${NEXUS_CONF}
fi

echo "Clearing previous debug log"

ls $NEXUS_DEBUG_LOG 2>&1 >/dev/null && rm $NEXUS_DEBUG_LOG

echo "Saving command used to ${NEXUS_START_SH} in case you want to run that script later to restart without running this whole script."

START_CMD="$NEXUS -datadir=$NEXUS_DATADIR -testnet -regtest -istimeseed -debug=1 -verbose=4 -server -keypool=1 -discover=0 -rest -mocktime=0"
echo "#!/bin/bash" >> ${NEXUS_START_SH}
echo ${START_CMD} >> ${NEXUS_START_SH}
chmod 755 ${NEXUS_START_SH} || true
if [[ "${NEXUS_TEST_SETUP_ONLY:-}" == "1" ]]; then
    echo "Setup only mode enabled. Not starting nexus. You can start it using ${NEXUS_START_SH} or by running the following command."
    echo "If you plan to use the CLI, use the command ./nexus -datadir=${NEXUS_DATADIR}"
    echo ${START_CMD}
    exit
fi

echo "Starting nexus"

${START_CMD} &
NEXUS_PID=$!
echo "PID is ${NEXUS_PID}"

sleep 1

tail -f ${NEXUS_DEBUG_LOG}

echo "The nexus daemon should end when you kill this script with Ctrl-C."

echo "THE TRUTH IS OUT THERE."
