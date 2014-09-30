#pragma once

namespace vosvideo
{
	namespace communication{
		class Peer
		{
		public:
			Peer(){}
			Peer(std::wstring peerId);
			virtual ~Peer(void);

			std::wstring const& GetPeerId() const;

			friend std::wstring operator+(std::wstring const& leftStr, Peer const& peer);
			std::wstring operator+(std::wstring const& str) const;	
		private:
			std::wstring peerId_;
		};
	}
}
