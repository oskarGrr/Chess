#pragma once
#include <WS2tcpip.h>
#include <string_view>
#include <optional>
#include <exception>

class networkException : public std::exception
{
public: 

    networkException(std::string_view msg, int winsockError = 0) 
        : std::exception(msg.data()), m_errorMsg{msg}, m_winsockError(winsockError), 
          m_hasValidWinsockError(false)
    {
        if(winsockError != 0)
            m_hasValidWinsockError = true;
    }

    inline char const* what() const override {return m_errorMsg.c_str();}
    inline int  getWinsockError() const      {return m_winsockError;}
    inline bool hasValidWinsockError() const {return m_hasValidWinsockError;}

private:

    std::string m_errorMsg;
    int  m_winsockError;
    bool m_hasValidWinsockError;//not every networkException has to have a winsock error
};

//for now the multyplayer will be a "peer to peer" TCP direct connection where
//one of the players will need to be the "server" and 
//port forward on port SERVER_PORT (defined in ChessNetworking.cpp).
//for now this interface only uses windows sockets but in the future I might use
//boost asio or something similar so that this whole application can be platform independent.
class P2PChessConnection
{
public:

    P2PChessConnection();
    ~P2PChessConnection();

    //every message sent will be prefixed with 1 byte (the first byte) as a "header"
    //of type NetMessageType that will act as a indicator for what the following bytes mean.
    //some messages only need to be the size of this "header". the RESIGN
    //and DRAW_OFFER for example are a total of 1 byte in the payload of the tcp packet
    //which is just the NetMessageType as no extra information is needed in those cases.
    enum struct NetMessageType : uint8_t 
    {
        INVALID = 0,

        //WHICH_SIDE is used when ChessApp::onNewConnection() is called to tell 
        //the other user what the side was that the ConnectionType::CLIENT 
        //picked so the ConnectionType::SERVER can pick the other side (white or black pieces)
        WHICH_SIDE,

        MOVE,
        RESIGN,
        DRAW_OFFER,
        REMATCH_REQUEST,
        REMATCH_ACCEPT
    };

    //ConnectionType::INVALID is used when not connected to anyone
    enum struct ConnectionType : uint8_t {INVALID = 0, SERVER, CLIENT};

    //the layout of the NetMessageType::MOVE type of message: 
    //|0|1|2|3|4|5|6|
    //byte 0 will be the NetMessageType
    //byte 1 will be the file (0-7) of the square where the piece is that will be moving
    //byte 2 will be the rank (0-7) of the square where the piece is that will be moving
    //byte 3 will be the file (0-7) of the square where the piece will be moving to
    //byte 4 will be the rank (0-7) of the square where the piece will be moving to
    //byte 5 will be the PromoType (enum defined in board.h) of the promotion if there is one
    //byte 6 will be the MoveInfo (enum defined in board.h) of the move that is happening
    static constexpr std::size_t s_moveMessageSize        {sizeof(NetMessageType) + 6};
    static constexpr std::size_t s_ResignMessageSize      {sizeof(NetMessageType)};
    static constexpr std::size_t s_DrawOfferMessageSize   {sizeof(NetMessageType)};

    //for sending a rematch request and used for accepting a rematch request
    static constexpr std::size_t s_rematchMessageSize {sizeof(NetMessageType)};

    //layout of s_WhichSideMessageSize:
    //|0|1|
    //byte 0 is the NetMessageType
    //byte 1 is the Side (an enum defined in ChessApplication.h)
    static constexpr std::size_t s_WhichSideMessageSize {sizeof(NetMessageType) + 1};

private:

    ConnectionType m_connectType;
    WSADATA m_winSockData;
    SOCKET  m_socket;//used by both peers (server and client) to send and recv chess moves
    std::string m_ipv4OfPeer;
    bool m_wasConnectionLostOrClosed;

public:

    void sendMessage(NetMessageType messageType, std::string_view message);
    void connect2Server(std::string_view targetIP);
    void waitForClient2Connect(long seconds = 0, long ms = 0);
    std::optional<std::string> recieveMessageIfAvailable(long seconds = 0, long ms = 0);
    inline bool isUserConnected() const {return m_connectType != ConnectionType::INVALID;}
    inline ConnectionType isUserServerOrClient() const {return m_connectType;}
    inline std::string const& getIpv4OfPeer() const {return m_ipv4OfPeer;}
    inline bool wasConnectionClosedOrLost() const {return m_wasConnectionLostOrClosed;}
    inline void resetWasConnectionLostBool() {m_wasConnectionLostOrClosed = false;}
    void disconnect();
};