// (c) Copyright 2019-2021 HP Development Company, L.P.

#include "HPGliaTypeConverter.h"
#include "HPGlia.h"

#include "Math/Vector.h"

#include <cstdint>

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::HeartRate& from, int& to)
{
    to = from.rate;
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const int64_t& domain, FDateTime& ue)
{
    FDateTime timeSinceEpoch = FDateTime(1970, 1, 1);

    // FTimespan tick has a resolution of 0.1 microseconds
    FTimespan modifyTime{ 10 * domain };
    timeSinceEpoch += modifyTime;

    ue = timeSinceEpoch;
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::Timestamp& domain, FDateTime& SystemTime)
{
    convertDomainTypeToUe4Type(domain.systemTimeMicroSeconds, SystemTime);
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::EyeTracking& domain, FEyeTracking& ue)
{
    convertDomainTypeToUe4Type(domain.timestamp, ue.SystemTime);
    ue.LeftGaze = FVector{ domain.leftGaze.z, -1 * domain.leftGaze.x, domain.leftGaze.y  };
    ue.LeftGazeConfidence = domain.leftGazeConfidence;

    ue.RightGaze = FVector{ domain.rightGaze.z, -1 * domain.rightGaze.x, domain.rightGaze.y,  };
    ue.RightGazeConfidence = domain.rightGazeConfidence;

    ue.CombinedGaze = FVector{ domain.combinedGaze.z, -1 * domain.combinedGaze.x, domain.combinedGaze.y };
    ue.CombinedGazeConfidence = domain.combinedGazeConfidence;

    ue.LeftPupilPosition = FVector{ 0, domain.leftPupilPosition.x, domain.leftPupilPosition.y};
    ue.LeftPupilPositionConfidence = domain.leftPupilPositionConfidence;

    ue.RightPupilPosition = FVector{ 0, domain.rightPupilPosition.x, domain.rightPupilPosition.y };
    ue.RightPupilPositionConfidence = domain.rightPupilPositionConfidence;
    

    ue.LeftPupilDilation = domain.leftPupilDilation;
    ue.LeftPupilDilationConfidence = domain.leftPupilDilationConfidence;

    ue.RightPupilDilation = domain.rightPupilDilation;
    ue.RightPupilDilationConfidence = domain.rightPupilDilationConfidence;
    
    ue.LeftEyeOpenness = domain.leftEyeOpenness;
    ue.LeftEyeOpennessConfidence = domain.leftEyeOpennessConfidence;

    ue.RightEyeOpenness = domain.rightEyeOpenness;
    ue.RightEyeOpennessConfidence = domain.rightEyeOpennessConfidence;
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::CognitiveLoad& domain, FCognitiveLoad& ue)
{
    ue.CognitiveLoad = domain.cognitiveLoad;
    ue.StandardDeviation = domain.standardDeviation;
    ue.DataState = FString(domain.dataState.c_str());
    convertDomainTypeToUe4Type(domain.startDataCollectTime.systemTimeMicroSeconds, ue.StartDataCollectTime);
    convertDomainTypeToUe4Type(domain.endDataCollectTime.systemTimeMicroSeconds, ue.EndDataCollectTime);
}

HP::Omnicept::Abi::LicensingModel HPGliaTypeConverter::LicenseModelConversion(ELicensingModel ue)
{
    HP::Omnicept::Abi::LicensingModel license;
    switch(ue)
    {
    case(ELicensingModel::CORE):
        license = HP::Omnicept::Abi::LicensingModel::CORE;
        break;
    case(ELicensingModel::TRIAL):
        license = HP::Omnicept::Abi::LicensingModel::TRIAL;
        break;

    case(ELicensingModel::ENTERPRISE):
        license = HP::Omnicept::Abi::LicensingModel::ENTERPRISE;
        break;

    case(ELicensingModel::REV_SHARE):
        license = HP::Omnicept::Abi::LicensingModel::REV_SHARE;
        break;

    default:
        license = HP::Omnicept::Abi::LicensingModel::UNKNOWN;
        break;
    }
    return license;
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::HeartRateVariability& domain, FHeartRateVariability& ue)
{
    ue.sdnn = domain.sdnn;
    ue.rmssd = domain.rmssd;
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::CameraImage& domain, FCameraImage& ue)
{
    switch (domain.format)
    {
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::UNKNOWN):
        ue.format = EFormatType::UNKNOWN;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::RGB888):
        ue.format = EFormatType::RGB888;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::YUY2):
        ue.format = EFormatType::YUY2;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::UYVY):
        ue.format = EFormatType::UYVY;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::YVYU):
        ue.format = EFormatType::YVYU;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::YUYV):
        ue.format = EFormatType::YUYV;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::AYUV):
        ue.format = EFormatType::AYUV;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::YV12):
        ue.format = EFormatType::YV12;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::NV12):
        ue.format = EFormatType::NV12;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::L8):
        ue.format = EFormatType::L8;
        break;
    case(HP::Omnicept::Abi::CameraImage::ImageFormat::CUSTOM):
        ue.format = EFormatType::CUSTOM;
        break;
    default:
        ue.format = EFormatType::UNKNOWN;
        break;
    }

    ue.imageData = TArray<uint8>(domain.imageData.data(), domain.width * domain.height);

    ue.width = domain.width;
    ue.height = domain.height;
    ue.frameNumber = domain.frameNumber;
    ue.fps = domain.framesPerSecond;
    ue.location = UTF8_TO_TCHAR(domain.sensorInfo.location.c_str());
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::DataVaultResult& domain, FDataVaultResults& rr)
{
    rr.sessionId = FString(domain.m_sessionId.c_str());
    switch (domain.m_result)
    {
    case(HP::Omnicept::Abi::DataVaultResultType::UNKNOWN):
        rr.result = EResultType::UNKNOWN;
        break;
    case(HP::Omnicept::Abi::DataVaultResultType::RECORDING_STARTED):
        rr.result = EResultType::RECORDING_STARTED;
        break;
    case(HP::Omnicept::Abi::DataVaultResultType::RECORDING_START_REJECTED):
        rr.result = EResultType::RECORDING_START_REJECTED;
        break;
    case(HP::Omnicept::Abi::DataVaultResultType::RECORDING_STOPPED):
        rr.result = EResultType::RECORDING_STOPPED;
        break;
    case(HP::Omnicept::Abi::DataVaultResultType::RECORDING_STOP_REJECTED):
        rr.result = EResultType::RECORDING_STOP_REJECTED;
        break;
    case(HP::Omnicept::Abi::DataVaultResultType::RECORDING_ERROR):
        rr.result = EResultType::RECORDING_ERROR;
        break;
    default:
        rr.result = EResultType::UNKNOWN;
        break;
    }
    switch (domain.m_error)
    {
    case(HP::Omnicept::Abi::DataVaultResultErrorType::UNKNOWN_ERROR):
        rr.resultError = EResultErrorType::UNKNOWN_ERROR;
        break;
    case(HP::Omnicept::Abi::DataVaultResultErrorType::SUCCESS_NO_ERROR):
        rr.resultError = EResultErrorType::SUCCESS_NO_ERROR;
        break;
    case(HP::Omnicept::Abi::DataVaultResultErrorType::INVALID_LICENSE_ERROR):
        rr.resultError = EResultErrorType::INVALID_LICENSE_ERROR;
        break;
    case(HP::Omnicept::Abi::DataVaultResultErrorType::CONNECTION_FAILED_ERROR):
        rr.resultError = EResultErrorType::CONNECTION_FAILED_ERROR;
        break;
    case(HP::Omnicept::Abi::DataVaultResultErrorType::CONNECTION_NEGOTIATING_ERROR):
        rr.resultError = EResultErrorType::CONNECTION_NEGOTIATING_ERROR;
        break;
    case(HP::Omnicept::Abi::DataVaultResultErrorType::RECORDING_HAS_ALREADY_STARTED_ERROR):
        rr.resultError = EResultErrorType::RECORDING_HAS_ALREADY_STARTED_ERROR;
        break;
    case(HP::Omnicept::Abi::DataVaultResultErrorType::RECORDING_HAS_NOT_STARTED_ERROR):
        rr.resultError = EResultErrorType::RECORDING_HAS_NOT_STARTED_ERROR;
        break;
    case(HP::Omnicept::Abi::DataVaultResultErrorType::RECORDING_STOP_NO_PERMISSON_ERROR):
        rr.resultError = EResultErrorType::RECORDING_STOP_NO_PERMISSON_ERROR;
        break;
    default:
        rr.resultError = EResultErrorType::UNKNOWN_ERROR;
        break;
    }
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::IMUFrame& domain, FIMUArray& uearr)
{
    for (auto& imu : domain.data)
    {
        FIMU fimu;
        fimu.Accelerometer = FVector{ imu.acc.x, imu.acc.y,imu.acc.z };
        fimu.Gyroscope = FVector{ imu.gyro.x, imu.gyro.y,imu.gyro.z };
        uearr.Imu.Add(fimu);
    }
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::PPGFrame& domain, FPPGArray& uearr)
{
    for (auto& ppg : domain.data)
    {
        FPPG fppg;
        for (auto& val : ppg.ledValues)
        {
            fppg.Values.Add(val);
        }
        convertDomainTypeToUe4Type(ppg.timestamp.systemTimeMicroSeconds, fppg.Timestamp);
        uearr.PpgArray.Add(fppg);
    }

    convertDomainTypeToUe4Type(domain.timestamp.systemTimeMicroSeconds, uearr.Timestamp);

}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::Subscription& domain, FSubscription& ue)
{
    switch (domain.getType())
    {
    case(HP::Omnicept::Abi::MessageType::eyeTracking):
        ue.MessageType = ESubscribeableMsgType::EyeTracking;
        break;
    case(HP::Omnicept::Abi::MessageType::cognitiveLoad):
        ue.MessageType = ESubscribeableMsgType::CognitiveLoad;
        break;
    case(HP::Omnicept::Abi::MessageType::heartRateVariability):
        ue.MessageType = ESubscribeableMsgType::HeartRateVariability;
        break;
    case(HP::Omnicept::Abi::MessageType::cameraImage):
        ue.MessageType = ESubscribeableMsgType::CameraImage;
        break;
    case(HP::Omnicept::Abi::MessageType::imuFrame):
        ue.MessageType = ESubscribeableMsgType::IMUArray;
        break;
    case(HP::Omnicept::Abi::MessageType::ppgFrame):
        ue.MessageType = ESubscribeableMsgType::PPGArray;
        break;
    case(HP::Omnicept::Abi::MessageType::heartRate):
        ue.MessageType = ESubscribeableMsgType::HeartRate;
        break;
    case(HP::Omnicept::Abi::MessageType::dataVaultResult):
        ue.MessageType = ESubscribeableMsgType::DataVaultResult;
        break;
    default:
        ue.MessageType = ESubscribeableMsgType::None;
        break;
    }
    ue.Sender = domain.getSender().c_str();
    ue.SensorInfo.Location = domain.getSensorInfo().location.c_str();
    ue.SensorInfo.SensorId = domain.getSensorInfo().deviceId.id.c_str();
    ue.SensorInfo.SensorSubId = domain.getSensorInfo().deviceId.subId.c_str();
    ue.Version = domain.getVersionSemantic().toString().c_str();
}

template <> static void HPGliaTypeConverter::convertDomainTypeToUe4Type(const HP::Omnicept::Abi::SubscriptionResultList& domain, FSubscriptionResultsArray& uearr)
{
    for (auto& subRet : domain.getSubscriptionResults())
    {
        FSubscriptionResults uesr;
        convertDomainTypeToUe4Type(subRet.subscription, uesr.subscription);
        if (uesr.subscription.MessageType == ESubscribeableMsgType::None)
        {
            continue;
        }

        switch (subRet.result)
        {
        case(HP::Omnicept::Abi::SubscriptionResultType::UNKNOWN):
            uesr.result = ESubscriptionResultType::UNKNOWN;
            break;
        case(HP::Omnicept::Abi::SubscriptionResultType::REJECTED):
            uesr.result = ESubscriptionResultType::REJECTED;
            break;
        case(HP::Omnicept::Abi::SubscriptionResultType::PENDING):
            uesr.result = ESubscriptionResultType::PENDING;
            break;
        case(HP::Omnicept::Abi::SubscriptionResultType::APPROVED):
            uesr.result = ESubscriptionResultType::APPROVED;
            break;
        default:
            uesr.result = ESubscriptionResultType::UNKNOWN;
            break;
        }
        switch (subRet.error)
        {
        case(HP::Omnicept::Abi::SubscriptionResultErrorType::UNKNOWN_ERROR):
            uesr.resultError = ESubscriptionResultErrorType::UNKNOWN_ERROR;
            break;
        case(HP::Omnicept::Abi::SubscriptionResultErrorType::SUCCESS_NO_ERROR):
            uesr.resultError = ESubscriptionResultErrorType::SUCCESS_NO_ERROR;
            break;
        case(HP::Omnicept::Abi::SubscriptionResultErrorType::NOT_LICENSED_ERROR):
            uesr.resultError = ESubscriptionResultErrorType::NOT_LICENSED_ERROR;
            break;
        case(HP::Omnicept::Abi::SubscriptionResultErrorType::REJECTED_BY_USER_ERROR):
            uesr.resultError = ESubscriptionResultErrorType::REJECTED_BY_USER_ERROR;
            break;
        case(HP::Omnicept::Abi::SubscriptionResultErrorType::SPECIFIED_VERSION_NOT_AVAILABLE_ERROR):
            uesr.resultError = ESubscriptionResultErrorType::SPECIFIED_VERSION_NOT_AVAILABLE_ERROR;
            break;
        default:
            uesr.resultError = ESubscriptionResultErrorType::UNKNOWN_ERROR;
            break;
        }
        uearr.subscriptionResultsArray.Add(uesr);
    }
}
