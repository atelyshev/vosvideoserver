#include "stdafx.h"
#include "CbWebsocketClientEngine.h"

using vosvideo::communication::casablanca::CbWebsocketClientEngine;
using namespace vosvideo::communication;
using namespace web::web_sockets::client;

CbWebsocketClientEngine::CbWebsocketClientEngine(std::shared_ptr<PubSubService> pubsubService) : WebsocketClientEngine(pubsubService){

}

CbWebsocketClientEngine::~CbWebsocketClientEngine(void){

}

void CbWebsocketClientEngine::Connect(std::wstring const& wUri)
{
	try
	{
		_client.connect(wUri)
			.then([=]{
			//Websocket connection opened. Publish to interested parties
			auto dto = _dtoFactory.Create(vosvideo::data::MsgType::ConnectionOpenedMsg);
			pubSubService_->Publish(dto);

			//Start listening for incoming messages
			StartListeningForMessages();
		}).wait();
	}
	catch (websocket_exception& ex){
		std::cout << "There was an error connecting to websocket server: " << ex.what();
		LOG_ERROR("There was an error connecting to websocket server: " << ex.what());
	}
}

void CbWebsocketClientEngine::Send(std::string const& msg)
{
	websocket_outgoing_message outgoingMsg = websocket_outgoing_message();
	outgoingMsg.set_utf8_message(msg);
	_client.send(outgoingMsg);
}

void CbWebsocketClientEngine::Close()
{
	_client.close();
}

void CbWebsocketClientEngine::StartListeningForMessages(){
	auto background_context = Concurrency::task_continuation_context::use_default();

	//As long as the internal task return true this task will continue to get invoked over and over again.
	create_task([=]() {
		AsyncDoWhile([=](){
			return _client.receive().then([=](pplx::task<websocket_incoming_message> inMsgTask){
				websocket_incoming_message inMsg = inMsgTask.get();

				return inMsg.extract_string().then([=](pplx::task<std::string> strTask) {
					std::string payload = strTask.get();
					std::shared_ptr<vosvideo::data::WebSocketMessageParser> msgParser(new vosvideo::data::WebSocketMessageParser(payload));
					LOG_TRACE("Received message with payload:" << payload);

					auto dto = _dtoFactory.Create(msgParser->GetMessageType());
					dto->Init(msgParser);
					pubSubService_->Publish(dto);

				}, background_context);

			}).then([](pplx::task<void> end_task){
				try
				{
					end_task.get();
					return true;
				}
				catch (websocket_exception ex)
				{
					LOG_ERROR("Websocket Connection failed with a websocket_exception, Error code: " << ex.error_code());
				}
				catch (...)
				{
					LOG_ERROR("Websocket Connection failed with no error code");
				}

				// We are here means we encountered some exception.
				// Return false to break the asynchronous loop.
				return false;
			});
		});
	}, background_context);

}

pplx::task<void> CbWebsocketClientEngine::AsyncDoWhile(std::function<pplx::task<bool>(void)> func)
{
	return DoWhileImpl(func).then([](bool){});
}

pplx::task<bool> CbWebsocketClientEngine::DoWhileIteration(std::function<pplx::task<bool>(void)> func)
{
	pplx::task_completion_event<bool> ev;

	func().then([=](bool task_completed)
	{
		ev.set(task_completed);
	});

	return pplx::create_task(ev);
}

pplx::task<bool> CbWebsocketClientEngine::DoWhileImpl(std::function<pplx::task<bool>(void)> func)
{
	return DoWhileIteration(func).then([=](bool continue_next_iteration) -> pplx::task<bool>
	{
		return ((continue_next_iteration) ? DoWhileImpl(func) : pplx::task_from_result(false));
	});
}


