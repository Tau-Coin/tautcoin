# How to be a miner leader in TAU
Do as follows:

## 1. Taucoin download
Download taucoin’s repository from github. If you a git user, you can type command in your terminal: 
```
git clone https://github.com/Tau-Coin/taucoin.git
```

## 2. Taucoin setup
In our first version, only Ubuntu operating system(16.04&&14.04) are fully tested. We have provided the setup tutorial, more details can be seen in taucoin/build-unix.md

## 3. Launch taucoin’s PC wallet
After a successful setup, you will see the executable files(taucoind, taucoin-cli, taucoin-tx) in taucoin/src and executable file(taucoin-qt) in taucoin/src/qt.
Taucoin-qt is recommended, which has an interface for easy operation.

## 4. Block data sync in PC wallet
The first step is data synchronization of TAU when you start the taucoin-qt. Just wait a few minutes in this step.

## 5. Be familiar with command console in PC wallet
When you finish block data synchronization, you need to learn how to operate in PC wallet.
Console can be found in taucoin-qt/help/Debug Window. You can see basic information of taucoin chain in first child window-information, and see console in second child window.
You can type “help” in the console window to see all operation commands of taucoin. If you want to know what’s the meaning of specified command and how to use it, just type it in console window and press enter.

## 6. Be a miner leader

### 6.1 Confirm your address
You will get a TAU address automatically when you launch the pc wallet. You can use “importprivkey” command to import your specified address.

### 6.2 Start with a signal transaction 
Details of signal transaction can be seen in TAU’s whitepaper. The signal transaction means that you resets the mining power to yourself. The signal transaction can be executed in taucoin-qt/Send or by commands(sendtoaddress, sendtransactiontoaddress) in console window.

To ensure a successful signal transaction, you should make a setting in pc wallet-> taucoin-qt-> Settings-> Options-> Wallet-> enable coin control features, which will enable the inputs option in Send interface. You can send a signal transaction to your mining address by your own mining address.

### 6.3 Mining TAU
You can start your mining when TAU admits your signal transaction in chain. Type “generate” or “generatetoaddress” in console window to see how to use mining commands. You will be a true miner after you mine a block and confirmed by the whole chain, congratulations.

## Questions in mining TAU

### 1. Supported operating systems for now
Only ubuntu(14.04&&16.04) are fully tested.

### 2. Taucoin-qt disappered in src/qt/
Taucoin-qt's build needs the third-part GUI software: Qt.
If you want to build Taucoin-qt, make sure that the required packages for Qt development are installed.

### 3. Data sync 
#### 3.1 Storage
You can change the storage data directory when you launch the taucoin-qt at first time. The default data directory is /home/\*\*\*/.taucoin. You can find different kinds of information about taucoin chain in storage directory.
#### 3.2 Sync details
You can check this synchronous process in details: pc wallet->taucoin-qt->Help->Debug window:information.

#### 3.3 Connections
Make sure that you have numbers of connections in your wallet, which can be seen in debug window->Information-> Network. If you have no connections, please go to Debug Window->Peers to have a check. You should have banned some trusted taucoin servers. We have provide serveral stable servers for whole network:
```
13.230.102.215:8686 
54.202.74.168:8686
52.62.202.42:8686
54.93.70.111:8686
```