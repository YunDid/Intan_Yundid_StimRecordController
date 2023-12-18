#include "rhs2000datablock.h"
#include "rhs2000evalboard.h"
#include "rhs2000registers.h"
#include <QVector>

//class Rhs2000EvalBoard;
//class Rhs2000DataBlock;
//class Rhs2000Registers;

class DeviceManager
{

public:
    DeviceManager(int sampleRateIndex, int stimStepIndex);
    ~DeviceManager();

    int connectToDevice();

private:

    int openInterfaceBoard(bool &expanderBoardDetected);

    void initializeInterfaceBoard();
    // 扫描连接到 SPI 端口的 RHS2000 放大器芯片，并配置与这些芯片的通信.
    void findConnectedAmplifiers();

    int deviceId(Rhs2000DataBlock *dataBlock, int stream);

    // 标识检测到的 SPI 端口数量.
    int numSpiPorts;
    // 开发板控制对象.
    Rhs2000EvalBoard *evalBoard;
    // 检测 I/O拓展版 是否连接.
    bool expanderBoardConnected;
    // 设备为模拟模式.
    bool synthMode;
    // 开发板模式.
    int evalBoardMode;
    // 采样率.
    int sampleRateIndex;
    // 刺激带宽.
    Rhs2000Registers::StimStepSize stimStep;
    // 定义电荷恢复过程中的电流限制选项.
    Rhs2000Registers::ChargeRecoveryCurrentLimit chargeRecoveryCurrentLimit;

    double cableLengthPortA;  // in meters
    double cableLengthPortB;  // in meters
    double cableLengthPortC;  // in meters
    double cableLengthPortD;  // in meters

//    double boardSampleRate;

//    unsigned int numUsbBlocksToRead;

//    QVector<bool> manualDelayEnabled;
//    QVector<int> manualDelay;

//    double desiredDspCutoffFreq;
//    double actualDspCutoffFreq;
//    double desiredUpperBandwidth;
//    double actualUpperBandwidth;
//    double desiredLowerBandwidth;
//    double desiredLowerSettleBandwidth;
//    double actualLowerBandwidth;
//    double actualLowerSettleBandwidth;
//    bool dspEnabled;
//    double notchFilterFrequency;
//    double notchFilterBandwidth;
//    bool notchFilterEnabled;
//    double highpassFilterFrequency;
//    bool highpassFilterEnabled;
//    double desiredImpedanceFreq;
//    double actualImpedanceFreq;
//    bool impedanceFreqValid;

//    SignalProcessor *signalProcessor;


    QVector<int> chipId;


};
