#!/bin/bash
#Bash script to automatically download all Taucoin dependancies ,build it and install it
#Should work on any debian based distro and fully tested on ubuntu 12.xx-14.xx-16.xx
#by : lod
echo " _        _______  ______  "
echo "( \      (  ___  )(  __  \ "
echo "| (      | (   ) || (  \  )"
echo "| |      | |   | || |   ) |"
echo "| |      | |   | || |   | |"
echo "| |      | |   | || |   ) |"
echo "| (____/\| (___) || (__/  )"
echo "(_______/(_______)(______/ "
echo ""
echo "Tau Automaic installer By LOD"
echo "this will take some time depending on your CPU/RAM/Internet speed"
echo "please be patient !! "
echo "adding bitcoin repo for levelDB "
sudo add-apt-repository ppa:bitcoin/bitcoin -y;
echo "updating all repos "
sudo apt-get update;
echo "installing dependancies..."
sudo apt-get install build-essential libtool autotools-dev automake pkg-config libssl-dev libevent-dev bsdmainutils libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev libqt4-dev libprotobuf-dev protobuf-compiler libdb4.8-dev libdb4.8++-dev -y;
mkdir tau;
cd tau ;
echo "cloning tau repo from GitHub..."
git clone https://github.com/Tau-Coin/taucoin.git;
cd taucoin;
echo "building Tau..."
./autogen.sh;
./configure;
echo "now the slow part of compiling...................."
make;
echo "almost done !!"
make install;
echo "All done , thanks for being patient !! "
echo "Now you can start the deamon with taucoind to work with taucoin-cli"
echo "or Start QT with taucoin-qt wallet if you have a desktop environment"
