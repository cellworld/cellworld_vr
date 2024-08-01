#pragma once
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "JsonObjectConverter.h"
#include "HAL/Runnable.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Tickable.h"
#include "MessageClient.generated.h"

USTRUCT(Blueprintable)
struct FMessage
{

	GENERATED_BODY()

public:

	FMessage() {
		id = FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens);
	}

	FMessage(const FString &header):
	header(header),
	id(FGuid::NewGuid().ToString(EGuidFormats::DigitsWithHyphens)){	}

	static FString CleanJson(const FString& jsonString) {
		return jsonString.Replace(TEXT("\r"), TEXT("")).Replace(TEXT("\n"), TEXT("")).Replace(TEXT("\t"), TEXT("")).Replace(TEXT("\"iD\""), TEXT("\"id\""));
	}

	template<typename InStructType>
	static FMessage NewMessage(char* header_str, const InStructType& BodyStruct) {
		auto message = FMessage();
		message.header = FString(header_str);
		FString Buffer;
		FJsonObjectConverter::UStructToJsonObjectString(BodyStruct, Buffer, 0, 0, 0);
		message.body = CleanJson(Buffer);
		return message;
	}

	template<typename OutStructType>
	OutStructType GetBody() const {
		auto BodyStruct = OutStructType();
		FJsonObjectConverter::JsonObjectStringToUStruct(body, &BodyStruct, 0, 0);
		return BodyStruct;
	}


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FString header;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FString body;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FString id;
};

USTRUCT(Blueprintable)
struct FMessagePart
{
	GENERATED_BODY()
public:

	FMessage to_message();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FString header;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FString body;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		FString id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		int seq;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Environment)
		int parts;
};

struct FMessageParts
{
public:
	FMessageParts() = default;
	FMessageParts(const FMessage& Message);
	TArray<FMessagePart> Parts;
	void Add(FMessagePart Part);
	bool IsReady();
	FMessage Join();
	FMessagePart& GetPart(int Seq);
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMessageReceived, FMessage, message);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResponseReceived, FString, response);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResponseTimeOut);


UCLASS(Blueprintable)
class UMessageRoute : public UObject {
	GENERATED_BODY()
public:
	UMessageRoute() {}

	UPROPERTY(BlueprintAssignable, Category = "TCPMessagesEvents")
	FOnMessageReceived MessageReceived;
};

UCLASS(Blueprintable)
class URequest: public UObject {
	GENERATED_BODY()
public:
	URequest() {}
	float TimeOut;
	UPROPERTY(BlueprintAssignable, Category = "TCPMessagesEvents")
	FOnResponseReceived ResponseReceived;
	UPROPERTY(BlueprintAssignable, Category = "TCPMessagesEvents")
	FOnResponseTimeOut TimedOut;
};


class UMessageClient;

class UMessageClientThread : public FRunnable
{
public:
	UMessageClientThread(UMessageClient *Client);

	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	
	bool Receive(FString &MessageString);

	UMessageClient* Client;
	TMap <FString, FMessageParts> PartialMessages;
	TAtomic<bool> IsRunning;
	TAtomic<bool> Finished;
};


UCLASS(Blueprintable)
class TCPMESSAGES_API UMessageClient : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	// FTickableGameObject Begin
	virtual void Tick(float DeltaTime) override;

	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FMyTickableThing, STATGROUP_Tickables);
	}
	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}
	virtual bool IsTickableInEditor() const
	{
		return false;
	}
	// FTickableGameObject End

	UMessageClient();

	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	static UMessageClient *NewMessageClient();

	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	static FMessage NewMessage(const FString &header, const FString &body);

	bool Send(const FString & MessageString);

	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	URequest *Subscribe();

	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	URequest* Ping();

	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	bool SendMessage(const FMessage & message);
	
	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	URequest *SendRequest(const FString& Header, const FString& Body, float TimeOut = -1);
	
	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	bool Connect(FString ServerIp, int ServerPort);
	
	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	bool IsConnected();
	
	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	bool Disconnect();

	UFUNCTION(BlueprintCallable, Category = "TCPMessages")
	UMessageRoute *AddRoute(FString header);

	UPROPERTY(BlueprintAssignable, Category = "TCPMessagesEvents")
	FOnMessageReceived MessageReceivedEvent;

	UPROPERTY(BlueprintAssignable, Category = "TCPMessagesEvents")
	FOnMessageReceived UnroutedMessageEvent;

	FSocket* Host;
	UMessageClientThread *ClientThread = nullptr;
	FRunnableThread* RunningThread = nullptr;
	TQueue<FMessage> Messages;
	TMap <FString, UMessageRoute*> Routes;
	TMap <FString, URequest*> PendingRequests;
};

