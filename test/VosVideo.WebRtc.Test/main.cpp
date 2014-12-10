// VosVideo.Communication.Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include <string>
#include <talk/base/scoped_ref_ptr.h>
#include <talk/app/webrtc/peerconnectioninterface.h>
#include <vosvideocommon/StringUtil.h>
#include "VosVideo.WebRtc/WebRtcPeerConnection.h"
#include "VosVideo.Data/DtoFactory.h"
#include "VosVideo.Data/LiveVideoOfferMsg.h"
#include "VosVideo.Data/WebSocketMessageParser.h"

using namespace std;
using namespace util;
using namespace vosvideo::data;
using namespace vosvideo::vvwebrtc;

// this is fake SDP
static string testSdpMsg ="{\"fp\":\"acd30709-bbee-4dd1-a36f-74548ab1f681\",\"tp\":\"acda50bb-ebcd-4503-a178-fb3fbbd4f3c3\",\"mt\":4,\"m\":[{\"camusername\":\"admin\",\"campassword\":\"123456\",\"camip\":\"192.168.1.179\",\"camport\":\"80\"},{\"sdp\":\"v=0\\r\\no=- 2298094395986478623 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=group:BUNDLE audio\\r\\na=msid-semantic: WMS\\r\\nm=audio 1 RTP/SAVPF 111 103 104 0 8 107 106 105 13 126\\r\\nc=IN IP4 0.0.0.0\\r\\na=rtcp:1 IN IP4 0.0.0.0\\r\\na=ice-ufrag:bCAio61dCPgGESI0\\r\\na=ice-pwd:HeFsDsbZfJcfzk6QcoXbMpvP\\r\\na=ice-options:google-ice\\r\\na=fingerprint:sha-256 87:4D:47:6A:0A:99:5A:48:F2:45:5D:37:02:B6:6C:4D:16:5D:87:75:A2:8E:76:68:7F:65:08:27:D2:E5:2F:AB\\r\\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\\r\\na=recvonly\\r\\na=mid:audio\\r\\na=rtcp-mux\\r\\na=crypto:0 AES_CM_128_HMAC_SHA1_32 inline:f8FZSo7IfGulkiFA8nYJSwqLOsPlfdg5HiHj7yg5\\r\\na=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:T54btrX3jt7/sC5TK/7VA4xKpnbCYCnRgq+bcNuc\\r\\na=rtpmap:111 opus/48000/2\\r\\na=fmtp:111 minptime=10\\r\\na=rtpmap:103 ISAC/16000\\r\\na=rtpmap:104 ISAC/32000\\r\\na=rtpmap:0 PCMU/8000\\r\\na=rtpmap:8 PCMA/8000\\r\\na=rtpmap:107 CN/48000\\r\\na=rtpmap:106 CN/32000\\r\\na=rtpmap:105 CN/16000\\r\\na=rtpmap:13 CN/8000\\r\\na=rtpmap:126 telephone-event/8000\\r\\na=maxptime:60\\r\\n\",\"type\":\"offer\"}]}";
static string testIceMsg ="{\"fp\":\"11a6a831-28f4-41f1-a421-dd304ef25ddd\",\"tp\":\"d3c57d4c-ba51-4f97-b86d-3d63c5b93bc2\",\"mt\":6,\"m\":{\"sdpMLineIndex\":0,\"sdpMid\":\"audio\",\"candidate\":\"a=candidate:2787077078 1 udp 2113937151 192.168.1.23 61737 typ host generation 0\\r\\n\"}}";

TEST(VosVideoMessageParser, ParseSdpDto)
{
	WebSocketMessageParser parser(testSdpMsg);
	EXPECT_EQ(MsgType::LiveVideoOfferMsg, parser.GetMessageType());
}

TEST(VosVideoWebRtcHandshake, CreateSdpDto)
{
	DtoFactory dtoFactory;
	shared_ptr<WebSocketMessageParser> msgParser(new WebSocketMessageParser(testSdpMsg));
	auto dto = dtoFactory.Create(msgParser->GetMessageType());
	dto->Init(msgParser);

	wstring wpayload = dto->ToString();
	string payload = StringUtil::ToString(wpayload);

	EXPECT_EQ("[{\"camusername\":\"admin\",\"campassword\":\"123456\",\"camip\":\"192.168.1.179\",\"camport\":\"80\"},{\"sdp\":\"v=0\\r\\no=- 2298094395986478623 2 IN IP4 127.0.0.1\\r\\ns=-\\r\\nt=0 0\\r\\na=group:BUNDLE audio\\r\\na=msid-semantic: WMS\\r\\nm=audio 1 RTP/SAVPF 111 103 104 0 8 107 106 105 13 126\\r\\nc=IN IP4 0.0.0.0\\r\\na=rtcp:1 IN IP4 0.0.0.0\\r\\na=ice-ufrag:bCAio61dCPgGESI0\\r\\na=ice-pwd:HeFsDsbZfJcfzk6QcoXbMpvP\\r\\na=ice-options:google-ice\\r\\na=fingerprint:sha-256 87:4D:47:6A:0A:99:5A:48:F2:45:5D:37:02:B6:6C:4D:16:5D:87:75:A2:8E:76:68:7F:65:08:27:D2:E5:2F:AB\\r\\na=extmap:1 urn:ietf:params:rtp-hdrext:ssrc-audio-level\\r\\na=recvonly\\r\\na=mid:audio\\r\\na=rtcp-mux\\r\\na=crypto:0 AES_CM_128_HMAC_SHA1_32 inline:f8FZSo7IfGulkiFA8nYJSwqLOsPlfdg5HiHj7yg5\\r\\na=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:T54btrX3jt7/sC5TK/7VA4xKpnbCYCnRgq+bcNuc\\r\\na=rtpmap:111 opus/48000/2\\r\\na=fmtp:111 minptime=10\\r\\na=rtpmap:103 ISAC/16000\\r\\na=rtpmap:104 ISAC/32000\\r\\na=rtpmap:0 PCMU/8000\\r\\na=rtpmap:8 PCMA/8000\\r\\na=rtpmap:107 CN/48000\\r\\na=rtpmap:106 CN/32000\\r\\na=rtpmap:105 CN/16000\\r\\na=rtpmap:13 CN/8000\\r\\na=rtpmap:126 telephone-event/8000\\r\\na=maxptime:60\\r\\n\",\"type\":\"offer\"}]", payload);

	wstring worigin;
	dto->GetFromPeer(worigin);

	string origin = StringUtil::ToString(worigin);
	EXPECT_EQ("acd30709-bbee-4dd1-a36f-74548ab1f681", origin);
}

TEST(VosVideoWebRtcHandshake, CreateIceDto)
{
	DtoFactory dtoFactory;
	shared_ptr<WebSocketMessageParser> msgParser(new WebSocketMessageParser(testIceMsg));
	auto dto = dtoFactory.Create(msgParser->GetMessageType());
	dto->Init(msgParser);

	wstring wpayload = dto->ToString();
	string payload = StringUtil::ToString(wpayload);

	EXPECT_EQ("{\"sdpMLineIndex\":0,\"sdpMid\":\"audio\",\"candidate\":\"a=candidate:2787077078 1 udp 2113937151 192.168.1.23 61737 typ host generation 0\\r\\n\"}", payload);

	wstring worigin;	
	dto->GetFromPeer(worigin);

	string origin = StringUtil::ToString(worigin);
	EXPECT_EQ("11a6a831-28f4-41f1-a421-dd304ef25ddd", origin);
}

TEST(VosVideoWebRtcPeerConnection, Create)
{
	talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface>peer_connection_factory  = webrtc::CreatePeerConnectionFactory();

	DtoFactory dtoFactory;
	shared_ptr<WebSocketMessageParser> msgParser(new WebSocketMessageParser(testSdpMsg));
	auto dto = dtoFactory.Create(msgParser->GetMessageType());
	dto->Init(msgParser);
	wstring srvPeer;
	wstring clientPeer;
	msgParser->GetFromPeer(clientPeer);
	msgParser->GetToPeer(srvPeer);

//	talk_base::scoped_refptr<WebRtcPeerConnection>pc = 
//		new talk_base::RefCountedObject<WebRtcPeerConnection>(clientPeer, srvPeer, peer_connection_factory, std::shared_ptr<vosvideo::communication::CommunicationManager>());

//	EXPECT_TRUE(pc.get()!=NULL);
}

int _tmain(int argc, _TCHAR* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
