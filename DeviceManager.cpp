#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

#include <QWidget>
#include <qglobal.h>

#include <QFile>
#include <QTime>
#include <QSound>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>

#include "DeviceManager.h"
#include "globalconstants.h"
#include "rhs2000evalboard.h"
#include "rhs2000datablock.h"
#include "rhs2000registers.h"


DeviceManager::DeviceManager(int sampleRateIndex, int stimStepIndex)
{

    cableLengthPortA = 1.0;
    cableLengthPortB = 1.0;
    cableLengthPortC = 1.0;
    cableLengthPortD = 1.0;

    chipId.resize(MAX_NUM_DATA_STREAMS);
    chipId.fill(-1);

    //   numSpiPorts = connectToDevice(expanderBoardConnected);
}

DeviceManager::~DeviceManager()
{

}

int DeviceManager::connectToDevice()
{

    // 打开与Intan设备的接口板连接
    numSpiPorts = openInterfaceBoard(expanderBoardConnected);
    return numSpiPorts;


    //    // 分配USB读取缓冲区
    //    // 注意：您需要提供 MAX_NUM_BLOCKS_TO_READ 和 Rhs2000DataBlock 的定义
    //    int maxPossibleDataStreams = 2 * numSpiPorts;
    //    int usbBufferSize = MAX_NUM_BLOCKS_TO_READ * 2 * Rhs2000DataBlock::calculateDataBlockSizeInWords(maxPossibleDataStreams);
    //    usbReadBuffer = new unsigned char [usbBufferSize];

    //    // 创建和配置USB数据流FIFO
    //    const unsigned int numSeconds = 10;  // 假设最大采样率的RAM缓冲区大小（秒）
    //    const unsigned int maxSamplingRate = 40000; // 最大采样率（样本/秒）
    //    unsigned int fifoBufferSize = numSeconds * maxSamplingRate * 2 * (Rhs2000DataBlock::calculateDataBlockSizeInWords(maxPossibleDataStreams) / SAMPLES_PER_DATA_BLOCK);
    //    usbStreamFifo = new DataStreamFifo(fifoBufferSize);

    //    // 创建USB数据线程
    //    // 注意：您需要提供 UsbDataThread 的定义和实现
    //    usbDataThread = new UsbDataThread(evalBoard, usbStreamFifo, this);
    //    // ... [连接信号和槽，如有必要]

        // 初始化界面板
        // 注意：您需要提供 initializeInterfaceBoard 的实现细节
    //    initializeInterfaceBoard();

        // 设置采样率
        // 注意：您需要提供 changeSampleRate 的实现细节
    //    changeSampleRate(sampleRateIndex, true);

        // 扫描端口
        // 注意：您需要提供 scanPorts 的实现细节
    //    scanPorts();

        // ... [其他初始化代码]
}

int DeviceManager::openInterfaceBoard(bool &expanderBoardDetected)
{

    evalBoard = new Rhs2000EvalBoard;
    expanderBoardDetected = false;

    // Open Opal Kelly XEM6310 board.
    int errorCode = evalBoard->open();

    // cerr << "In MainWindow::openInterfaceBoard.  errorCode = " << errorCode << "\n";

    if (errorCode < 1) {

        if (errorCode == -1) {
            std::cerr << "Cannot load Opal Kelly FrontPanel DLL.\n"
                         "Opal Kelly USB drivers not installed.\n";
        } else {
            std::cerr << "Intan Stimulation / Recording Controller Not Found.\n"
                         "Intan Technologies Stimulation / Recording Controller not found on any USB port.\n";
        }

        exit(EXIT_FAILURE); // abort application
    }

    // Load Rhythm FPGA configuration bitfile (provided by Intan Technologies).
    string bitfilename =
            QString(QCoreApplication::applicationDirPath() + "/main.bit").toStdString();

    if (!evalBoard->uploadFpgaBitfile(bitfilename)) {
        std::cerr << "Hardware Configuration File Upload Error.\n"
                     "Cannot upload configuration file to Intan Stimulation / Recording Controller.\n"
                     "Make sure file main.bit is in the same directory as the executable file.\n";
        exit(EXIT_FAILURE); // abort application
    }

    evalBoard->resetBoard();
    // Read 4-bit board mode.
    evalBoardMode = evalBoard->getBoardMode();

    if (evalBoardMode != RHS_BOARD_MODE) {
        std::cerr << "Incompatible Controller Error.\n"
                     "The device connected to the USB port is not an Intan Stimulation / Recording Controller.\n"
                     "Make sure file main.bit is in the same directory as the executable file.\n";
        exit(EXIT_FAILURE); // abort application
    }

    evalBoard->readDigitalInManual(expanderBoardDetected);
    evalBoard->readDigitalInExpManual();
    return 4;
}

// Initialize a USB interface board connected to a USB port.
void DeviceManager::initializeInterfaceBoard()
{
    // Initialize interface board.
    evalBoard->initialize();

    evalBoard->enableDcAmpConvert(true);
    evalBoard->setExtraStates(0);

    // Set sample rate and upload all auxiliary SPI command sequences.
//    changeSampleRate(sampleRateIndex, true);

    // Since our longest command sequence is 128 commands, we run the SPI
    // interface for 128 samples.
    evalBoard->setMaxTimeStep(SAMPLES_PER_DATA_BLOCK);
    evalBoard->setContinuousRunMode(false);

    // Start SPI interface.
    evalBoard->run();

    // Wait for the 128-sample run to complete.
    while (evalBoard->isRunning()) {
        qApp->processEvents();
    }

    // Read the resulting single data block from the USB interface.
    Rhs2000DataBlock *dataBlock = new Rhs2000DataBlock(evalBoard->getNumEnabledDataStreams());
    evalBoard->readDataBlock(dataBlock);

    // We don't need to do anything with this data block; it was used to configure
    // the RHS2000 amplifier chips and to run ADC calibration.
    delete dataBlock;

    // Set default configuration for all eight DACs on interface board.
    evalBoard->enableDac(0, false);
    evalBoard->enableDac(1, false);
    evalBoard->enableDac(2, false);
    evalBoard->enableDac(3, false);
    evalBoard->enableDac(4, false);
    evalBoard->enableDac(5, false);
    evalBoard->enableDac(6, false);
    evalBoard->enableDac(7, false);
    evalBoard->selectDacDataStream(0, 8);   // Initially point DACs to DacManual1 input
    evalBoard->selectDacDataStream(1, 8);
    evalBoard->selectDacDataStream(2, 8);
    evalBoard->selectDacDataStream(3, 8);
    evalBoard->selectDacDataStream(4, 8);
    evalBoard->selectDacDataStream(5, 8);
    evalBoard->selectDacDataStream(6, 8);
    evalBoard->selectDacDataStream(7, 8);
    evalBoard->selectDacDataChannel(0, 0);
    evalBoard->selectDacDataChannel(1, 1);
    evalBoard->selectDacDataChannel(2, 0);
    evalBoard->selectDacDataChannel(3, 0);
    evalBoard->selectDacDataChannel(4, 0);
    evalBoard->selectDacDataChannel(5, 0);
    evalBoard->selectDacDataChannel(6, 0);
    evalBoard->selectDacDataChannel(7, 0);
    evalBoard->setDacManual(32768);
    evalBoard->setDacGain(0);
    evalBoard->setAudioNoiseSuppress(0);

    evalBoard->setCableLengthMeters(Rhs2000EvalBoard::PortA, 0.0);
    evalBoard->setCableLengthMeters(Rhs2000EvalBoard::PortB, 0.0);
    evalBoard->setCableLengthMeters(Rhs2000EvalBoard::PortC, 0.0);
    evalBoard->setCableLengthMeters(Rhs2000EvalBoard::PortD, 0.0);
}

// Return the Intan chip ID stored in ROM register 255.  If the data is invalid
// (due to a SPI communication channel with the wrong delay or a chip not present)
// then return -1.
int DeviceManager::deviceId(Rhs2000DataBlock *dataBlock, int stream)
{
    bool intanChipPresent;

    // First, check ROM registers 251-253 to verify that they hold 'INTAN'.
    // This is just used to verify that we are getting good data over the SPI
    // communication channel.
    intanChipPresent = ((char) ((dataBlock->auxiliaryData[stream][0][61] & 0xff00) >> 8) == 'I' &&
                        (char) ((dataBlock->auxiliaryData[stream][0][61] & 0x00ff) >> 0) == 'N' &&
                        (char) ((dataBlock->auxiliaryData[stream][0][60] & 0xff00) >> 8) == 'T' &&
                        (char) ((dataBlock->auxiliaryData[stream][0][60] & 0x00ff) >> 0) == 'A' &&
                        (char) ((dataBlock->auxiliaryData[stream][0][59] & 0xff00) >> 8) == 'N' &&
                        (char) ((dataBlock->auxiliaryData[stream][0][59] & 0x00ff) >> 0) == 0);

    if (!intanChipPresent) {
        return -1;
    } else {
        return dataBlock->auxiliaryData[stream][0][57]; // chip ID (Register 255)
    }
}

// Scan SPI Ports A-D to find all connected RHS2000 amplifier chips.
// Read the chip ID from on-chip ROM register 63 to determine the number
// of amplifier channels on each port.  This process is repeated at all
// possible MISO delays in the FPGA, and the cable length on each port
// is inferred from this.
void DeviceManager::findConnectedAmplifiers()
{
    int delay, stream, id, i, channel, port;
    // int auxName, vddName;
    int numChannelsOnPort[MAX_NUM_SPI_PORTS] = {0, 0, 0, 0};
    QVector<int> portIndex, portIndexOld, chipIdOld, commandStream;

    portIndex.resize(MAX_NUM_DATA_STREAMS);
    portIndexOld.resize(MAX_NUM_DATA_STREAMS);
    chipIdOld.resize(MAX_NUM_DATA_STREAMS);
    commandStream.resize(MAX_NUM_DATA_STREAMS);

    chipId.fill(-1);
    chipIdOld.fill(-1);
    portIndexOld.fill(-1);
    portIndex.fill(-1);
    commandStream.fill(-1);

    if (!synthMode) {
        // Set sampling rate to highest value for maximum temporal resolution.
//        changeSampleRate(sampleRateComboBox->count() - 1);

        portIndexOld[0]  = 0;
        portIndexOld[1]  = 0;
        portIndexOld[2]  = 1;
        portIndexOld[3]  = 1;
        portIndexOld[4]  = 2;
        portIndexOld[5]  = 2;
        portIndexOld[6]  = 3;
        portIndexOld[7]  = 3;

        // Enable all data streams
        for (i = 0; i < MAX_NUM_DATA_STREAMS; i++) {
            evalBoard->enableDataStream(i, true);
        }

//        evalBoard->selectAuxCommandBank(Rhs2000EvalBoard::PortA,
//                                        Rhs2000EvalBoard::AuxCmd3, 0);
//        evalBoard->selectAuxCommandBank(Rhs2000EvalBoard::PortB,
//                                        Rhs2000EvalBoard::AuxCmd3, 0);
//        evalBoard->selectAuxCommandBank(Rhs2000EvalBoard::PortC,
//                                        Rhs2000EvalBoard::AuxCmd3, 0);
//        evalBoard->selectAuxCommandBank(Rhs2000EvalBoard::PortD,
//                                        Rhs2000EvalBoard::AuxCmd3, 0);

        // Since our longest command sequence is 128 commands, we run the SPI
        // interface for 128 samples.
        evalBoard->setMaxTimeStep(SAMPLES_PER_DATA_BLOCK);
        evalBoard->setContinuousRunMode(false);

        Rhs2000DataBlock *dataBlock =
                new Rhs2000DataBlock(evalBoard->getNumEnabledDataStreams());

        QVector<int> sumGoodDelays(MAX_NUM_DATA_STREAMS, 0);
        QVector<int> indexFirstGoodDelay(MAX_NUM_DATA_STREAMS, -1);
        QVector<int> indexSecondGoodDelay(MAX_NUM_DATA_STREAMS, -1);

        // Run SPI command sequence at all 16 possible FPGA MISO delay settings
        // to find optimum delay for each SPI interface cable.
        for (delay = 0; delay < 16; ++delay) {
            evalBoard->setCableDelay(Rhs2000EvalBoard::PortA, delay);
            evalBoard->setCableDelay(Rhs2000EvalBoard::PortB, delay);
            evalBoard->setCableDelay(Rhs2000EvalBoard::PortC, delay);
            evalBoard->setCableDelay(Rhs2000EvalBoard::PortD, delay);

            // Start SPI interface.
            evalBoard->run();

            // Wait for the 128-sample run to complete.
            while (evalBoard->isRunning()) {
                qApp->processEvents();
            }

            // Read the resulting single data block from the USB interface.
            evalBoard->readDataBlock(dataBlock);

            // Read the Intan chip ID number from each RHS2000 chip found.
            // Record delay settings that yield good communication with the chip.
            for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
                id = deviceId(dataBlock, stream);

                if (id == CHIP_ID_RHS2116) {
                    // cout << "Delay: " << delay << " on stream " << stream << " is good." << endl;
                    sumGoodDelays[stream] = sumGoodDelays[stream] + 1;
                    if (indexFirstGoodDelay[stream] == -1) {
                        indexFirstGoodDelay[stream] = delay;
                        chipIdOld[stream] = id;
                    } else if (indexSecondGoodDelay[stream] == -1) {
                        indexSecondGoodDelay[stream] = delay;
                        chipIdOld[stream] = id;
                    }
                }
            }
        }

        // Set cable delay settings that yield good communication with each
        // RHS2000 chip.
        QVector<int> optimumDelay(MAX_NUM_DATA_STREAMS, 0);
        for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
            if (sumGoodDelays[stream] == 1 || sumGoodDelays[stream] == 2) {
                optimumDelay[stream] = indexFirstGoodDelay[stream];
            } else if (sumGoodDelays[stream] > 2) {
                optimumDelay[stream] = indexSecondGoodDelay[stream];
            }
        }

        evalBoard->setCableDelay(Rhs2000EvalBoard::PortA,
                                 qMax(optimumDelay[0], optimumDelay[1]));
        evalBoard->setCableDelay(Rhs2000EvalBoard::PortB,
                                 qMax(optimumDelay[2], optimumDelay[3]));
        evalBoard->setCableDelay(Rhs2000EvalBoard::PortC,
                                 qMax(optimumDelay[4], optimumDelay[5]));
        evalBoard->setCableDelay(Rhs2000EvalBoard::PortD,
                                 qMax(optimumDelay[6], optimumDelay[7]));

        cout << "Port A cable delay: " << qMax(optimumDelay[0], optimumDelay[1]) << endl;
        cout << "Port B cable delay: " << qMax(optimumDelay[2], optimumDelay[3]) << endl;
        cout << "Port C cable delay: " << qMax(optimumDelay[4], optimumDelay[5]) << endl;
        cout << "Port D cable delay: " << qMax(optimumDelay[6], optimumDelay[7]) << endl;

        cableLengthPortA =
                evalBoard->estimateCableLengthMeters(qMax(optimumDelay[0], optimumDelay[1]));
        cableLengthPortB =
                evalBoard->estimateCableLengthMeters(qMax(optimumDelay[2], optimumDelay[3]));
        cableLengthPortC =
                evalBoard->estimateCableLengthMeters(qMax(optimumDelay[4], optimumDelay[5]));
        cableLengthPortD =
                evalBoard->estimateCableLengthMeters(qMax(optimumDelay[6], optimumDelay[7]));

        delete dataBlock;

    } else {
        // If we are running with synthetic data (i.e., no interface board), just assume
        // that one RHS2116 is plugged into Port A.
        chipIdOld[0] = CHIP_ID_RHS2116;
        portIndexOld[0] = 0;
    }

    // Now that we know which RHS2000 amplifier chips are plugged into each SPI port,
    // add up the total number of amplifier channels on each port and calcualate the number
    // of data streams necessary to convey this data over the USB interface.
    int numStreamsRequired = 0;
    for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
        if (chipIdOld[stream] == CHIP_ID_RHS2116) {
            numStreamsRequired++;
            if (numStreamsRequired <= MAX_NUM_DATA_STREAMS) {
                numChannelsOnPort[portIndexOld[stream]] += 16;
            }
        }
    }

    // Reconfigure USB data streams in consecutive order to accommodate all connected chips.
    stream = 0;
    for (int oldStream = 0; oldStream < MAX_NUM_DATA_STREAMS; ++oldStream) {
        if ((chipIdOld[oldStream] == CHIP_ID_RHS2116) && (stream < MAX_NUM_DATA_STREAMS)) {
            chipId[stream] = CHIP_ID_RHS2116;
            portIndex[stream] = portIndexOld[oldStream];
            if (!synthMode) {
                evalBoard->enableDataStream(oldStream, true);
            }
            commandStream[stream] = oldStream;
            stream++ ;
        } else {
            if (!synthMode) {
                evalBoard->enableDataStream(oldStream, false);
            }
        }
    }

//    // Add channel descriptions to the SignalSources object to create a list of all waveforms.
//    for (port = 0; port < numSpiPorts; ++port) {
//        if (numChannelsOnPort[port] == 0) {
//            signalSources->signalPort[port].channel.clear();
//            signalSources->signalPort[port].enabled = false;
//        } else if (signalSources->signalPort[port].numAmplifierChannels() !=
//                   numChannelsOnPort[port]) {  // if number of channels on port has changed...
//            signalSources->signalPort[port].channel.clear();  // ...clear existing channels...
//            // ...and create new ones.
//            channel = 0;
//            // Create amplifier channels for each chip.
//            for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
//                if (portIndex[stream] == port) {
//                    if (chipId[stream] == CHIP_ID_RHS2116) {
//                        for (i = 0; i < 16; ++i) {
//                            signalSources->signalPort[port].addAmplifierChannel(channel, i, stream);
//                            signalSources->signalPort[port].channel[channel].commandStream = commandStream[stream];
//                            channel++;
//                        }
//                    }
//                }
//            }
//        } else {    // If number of channels on port has not changed, don't create new channels (since this
//                    // would clear all user-defined channel names.  But we must update the data stream indices
//                    // on the port.
//            channel = 0;
//            // Update stream indices for amplifier channels.
//            for (stream = 0; stream < MAX_NUM_DATA_STREAMS; ++stream) {
//                if (portIndex[stream] == port) {
//                    if (chipId[stream] == CHIP_ID_RHS2116) {
//                        for (i = channel; i < channel + 16; ++i) {
//                            signalSources->signalPort[port].channel[i].boardStream = stream;
//                        }
//                        channel += 16;
//                    }
//                }
//            }
//        }
//    }
}


