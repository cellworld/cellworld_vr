#include "MessageClient.h"
#include "Networking.h"
#include "JsonObjectConverter.h"
#include "Misc/Guid.h"

bool UMessageClient::SendMessage(const FMessage& message) {
	if (!IsConnected()) {
		UE_LOG(LogTemp, Error, TEXT("[UMessageClient::SendMessage] Client disconnected. Returning FALSE."));
		return false;
	}
	
	auto parts = FMessageParts(message);
	for (auto& part : parts.Parts) {
		FString Buffer;
		FJsonObjectConverter::UStructToJsonObjectString(part, Buffer, 0, 0, 0, 0, false);
		auto messageString = FMessage::CleanJson(Buffer);
		if (!Send(messageString)) return false;
	}
	return true;
}

UMessageClient::UMessageClient() {
	Host = nullptr;
}

bool UMessageClient::Connect(FString ServerIp, int ServerPort) {
	if (IsConnected()) return false;
	FIPv4Address ip;
	FIPv4Address::Parse(ServerIp, ip);
	TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(ServerPort);
	FSocket *HostT = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	if (HostT->Connect(*addr)) {
		Host = HostT;
		ClientThread = new UMessageClientThread(this);
		RunningThread = FRunnableThread::Create(ClientThread, TEXT("Tcp-messages Thread"));
		return true;
	}
	return false;
}

bool UMessageClient::IsConnected() {
	return (Host != nullptr);
}

bool UMessageClient::Disconnect() {
	if (!IsConnected()) return false;
	ClientThread->IsRunning = false;
	while(!ClientThread->Finished);
	ClientThread->Stop();
	ClientThread->Exit();
	delete ClientThread;
	Host->Close();
	Host = nullptr;
	return true;
}

URequest* UMessageClient::SendRequest(const FString& Header, const FString& Body, float TimeOut)
{
	auto request = NewObject<URequest>();
	request->TimeOut = TimeOut;
	request->AddToRoot();
	FMessage request_message(Header);
	request_message.body = Body;
	PendingRequests.Add(request_message.id, request);
	SendMessage(request_message);
	return request;
}

UMessageRoute *UMessageClient::AddRoute(FString header)
{
	auto c = NewObject<UMessageRoute>();
	Routes.Add(header, c);
	c->AddToRoot();
	return c;
}

UMessageClient* UMessageClient::NewMessageClient()
{
	auto c = NewObject<UMessageClient>();
	c->AddToRoot();
	return c;
}

FMessage UMessageClient::NewMessage(const FString& header, const FString& body)
{
	FMessage message(header);
	message.body = body;
	return message;
}

bool UMessageClient::Send(const FString& MessageString) {
	const TCHAR* seriallizedChar = MessageString.GetCharArray().GetData();
	int32 size = FCString::Strlen(seriallizedChar) + 1;
	int32 sent = 0;
	if (!Host->Send((uint8*)TCHAR_TO_UTF8(seriallizedChar), size, sent))
	{
		Disconnect();
		return false;
	}
	return sent == size;
}

URequest* UMessageClient::Subscribe()
{
	return SendRequest("!subscribe", "");
}


URequest* UMessageClient::Ping()
{
	return SendRequest("!ping", "");
}

FMessageParts::FMessageParts(const FMessage& message)
{
	int parts_count = (message.body.Len() / 1024) + 1;
	for (int i = 0; i < parts_count; i++) {
		auto part = FMessagePart();
		part.header = message.header;
		part.id = message.id;
		part.body = message.body.Mid(i * 1024, 1024);
		part.parts = parts_count;
		part.seq = i;
		Parts.Add(part);
	}
}

bool FMessageParts::IsReady()
{
	if (Parts.Num() == 0) return false;
	return Parts[0].parts == Parts.Num();
}

void FMessageParts::Add(FMessagePart part) {
	Parts.Add(part);
}

FMessage FMessageParts::Join()
{
	if (Parts.Num() == 0) return FMessage();
	auto message = GetPart(0).to_message();
	for (int i = 1; i < Parts.Num(); i++) {
		message.body += GetPart(i).body;
	}
	return message;
}

FMessagePart& FMessageParts::GetPart(int seq)
{
	for (auto& p : Parts) {
		if (p.seq == seq) return p;
	}
	return Parts[0];
}


FMessage FMessagePart::to_message()
{
	FMessage message;
	message.body = body;
	message.header = header;
	message.id = id;
	return message;
}

UMessageClientThread::UMessageClientThread(UMessageClient *Client) :
	Client(Client) {
}

bool UMessageClientThread::Init()
{
	return true;
}

#define buffersize 40000
uint32 UMessageClientThread::Run()
{
	IsRunning = true;
	Finished = false;
	FString messageString;
	FMessagePart message_part;
	while (IsRunning) {
		if (Receive(messageString)) {
			FJsonObjectConverter::JsonObjectStringToUStruct(messageString, &message_part, 0, 0);
			if (!PartialMessages.Contains(message_part.id)) {
				PartialMessages.Add(message_part.id, FMessageParts());
			}
			PartialMessages[message_part.id].Add(message_part);
			if (PartialMessages[message_part.id].IsReady()) {
				auto message = PartialMessages[message_part.id].Join();
				Client->Messages.Enqueue(message);
				PartialMessages.Remove(message_part.id);
			}
		}
	}
	Finished = true;
	return 0;
}

void UMessageClientThread::Stop()
{
	IsRunning = false;
}

bool UMessageClientThread::Receive(FString& MessageString)
{
	uint8 buffer[buffersize];
	auto received_data = reinterpret_cast<const char*>(buffer);
	int offset = 0;
	IsRunning = true;
	Finished = false;
	uint32 pendingData = 0;
	bool complete = false;
	bool broken_pipe = false;
	// while ((Client->IsValidLowLevelFast()) && (Client->Host->HasPendingData(pendingData) || offset) && !complete && !broken_pipe)
	while ((Client->Host->HasPendingData(pendingData) || offset) && !complete && !broken_pipe)
	{
		while (!Client->Host->HasPendingData(pendingData));
		int32 read = 0;
		if (Client->Host->Recv(buffer + offset, 1, read)) {
			if (read == 0) {
				*(buffer + offset) = 0;
				broken_pipe = true;
				break;
			}
			if (*(buffer + offset) == 0) {
				complete = true;
				break;
			}
			offset++;
		}
		else {
			break;
		}
	}
	if (complete) {
		MessageString = FString(ANSI_TO_TCHAR(received_data));
		return true;
	} else {
		return false;
	}
}

void UMessageClient::Tick(float DeltaTime) {
	while (!Messages.IsEmpty()) {
		FMessage message;
		if (Messages.Dequeue(message)) {
			if (PendingRequests.Contains(message.id)) {
				PendingRequests[message.id]->ResponseReceived.Broadcast(message.body);
				PendingRequests[message.id]->RemoveFromRoot();
				PendingRequests.Remove(message.id);
			}
			else {
				MessageReceivedEvent.Broadcast(message);
				if (Routes.Contains(message.header)) {
					Routes[message.header]->MessageReceived.Broadcast(message);
				}
				else {
					UnroutedMessageEvent.Broadcast(message);
				}
			}
		}
	}
	TArray<FString> RequestsIds;
	PendingRequests.GetKeys(RequestsIds);
	for (auto &RequestId:RequestsIds)
	{
		if (PendingRequests[RequestId]->TimeOut >= 0) { // TimeOut is dissabled for this request
			if (PendingRequests[RequestId]->TimeOut < DeltaTime) {
				PendingRequests[RequestId]->TimedOut.Broadcast();
				PendingRequests[RequestId]->RemoveFromRoot();
				PendingRequests.Remove(RequestId);
				continue;
			}
			else {
				PendingRequests[RequestId]->TimeOut -= DeltaTime;
			}
		}
	}
}