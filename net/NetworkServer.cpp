#include "NetworkServer.h"

#include "net/Network.h"
#include "net/NetworkPeer.h"

#include "srv/Time.h"
#include "common/CRC32.h"
#include "common/Timer.h"
#include "common/util.h"
#include <cmath>

namespace CGameEngine
{
    /// Network ///////////////////////////////////////////////////////////////

    NetworkServer::NetworkServer(SoftwareVersion* swv, uint16_t port, SafeQueue<Datagram*>* dbbuff, std::string hostname /*= ""*/) : Network(swv, port, dbbuff, hostname)
    {
        if(!m_isActive) { Logger::getInstance().Log(Logs::FATAL, Logs::Network, "NetworkServer::NetworkServer()", "NetworkServer failed to start using Network() base constructor!"); }
        else
        {
            m_sendThread = new std::thread(startSendLoop, this); // send processing
            m_listenThread = new std::thread(startListenLoop, this); // network (tcp/udp) listen thread
            m_updateThread = new std::thread(startUpdateLoop, this); // all network traffic update loop
            m_networkType = NetworkType::Server;

           Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkServer::NetworkServer()", "Started threads with srcAddress \033[1m{}", getIPString((struct sockaddr_storage*)m_srcAddress->ai_addr));
        }
    }

    NetworkServer::~NetworkServer()
    {
        while(!shutdownSockets())
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::~NetworkServer()", "Waiting on shutdownSockets() to complete.");
        }
    }

    bool NetworkServer::shutdownSockets()
    {
        m_isActive = false;
        m_netListening = false;
        m_sendQueue.clear();
        while(!m_packetBuffer.empty()) { m_packetBuffer.front().destroy(); m_packetBuffer.pop(); }

        // clear net connections
        for(auto& it : m_netConnections) { safeDelete(it.second); }
        m_netConnections.clear();

		Logger::getInstance().Log(Logs::DEBUG, "NetworkServer::shutdownSockets()", "Completed Successfully!");
        return true;
    }

    /// \TODO: Evaluate breaking out processing into separate function
    void NetworkServer::updateLoop()
    {
        std::queue<std::string> toBeClosed;
        std::unordered_map<std::string, NetConnection*>::iterator nit = m_netConnections.begin();
        std::unordered_map<uint32_t, StoredSequence*>::iterator sit = m_storedSequences.begin();
        std::mutex ulmutex;
        std::unique_lock<std::mutex> updateLock(ulmutex);

        Timer connUpdate = Timer("connUpdate", 500); // MS

        while(m_isActive)
        {
            // lock to wait for data
            if(m_updateCV.wait_until(updateLock, std::chrono::_V2::steady_clock::now() + HEARTBEAT_INTERVAL) == std::cv_status::timeout)
            {
                int pkts = m_packetBuffer.size();
                while(pkts > 0)
                {
                    // packet exists, grab first one and it's sender
                    PacketPair* pp = &m_packetBuffer.front();
                    Packet* p = pp->pkt;
                    sockaddr_storage* sender = &pp->sas;
                    uint32_t pktCRC = 0;

                    // try-catch bad or missing data
                    if(!p || !sender) { Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::updateLoop()", "Broken PacketPair!"); /*p = nullptr; sender = nullptr;*/}
                    else
                    {
                        // check identifier
                        bool broken = true;
                        try { broken = memcmp(p->identifier, m_identifier.c_str(), IDENT_SIZE); }
                        catch (...) { broken = true; }

                        if(broken)
                        {
                           Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::updateLoop()", "Packet did NOT meet identifier! [{}] ({} remaining)", m_identifier, pkts);
                        }
                        else
                        {
                            NetConnection* nc = nullptr;
                            std::string ipStr = getIPString(sender);
                            std::unordered_map<std::string, NetConnection*>::iterator it = m_netConnections.find(ipStr);

                            // handle these first as they are important
                            if(p->op_code == m_reqConnOP)
                            {
                                std::unordered_map<std::string, uint32_t>::iterator deadIt = m_closedConnections.find(ipStr); // see if it is recently dead
                                if(it == m_netConnections.end() && deadIt == m_closedConnections.end()) // not found, requesting access
                                {
                                   Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkServer::updateLoop()", "Creating new connection for {} [Count: {}], sending 'accepted' reply.", ipStr, m_netConnections.size());

                                    // generate unique identifier for internal use
                                    uint32_t sentID = 0;
                                    try { ConnectionRequest_Struct* csr = (ConnectionRequest_Struct*)p->data; sentID = csr->uniqID; }
                                    catch (...) { }
                                    nc = new NetConnection(this, sender, ipStr, sentID, m_datagramBuffer); // create the connection
                                    if(nc)
                                    {
                                       Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetworkServer::updateLoop()", "\033[1mConnection [{}] being sent 'accepted' reply.\033[0m", nc->getUniqueID());
                                        m_netConnections.insert(std::make_pair(ipStr, nc)); // add connection to connection map
                                        sendSimple(nc->getSource(), m_acceptConnOP); // let connecting client know we're receiving and accepting
                                        nc->addPacket(&p); // add ConnectionAccepted packet for initial datagram hand-off
                                    }
                                }
                                else if(it == m_netConnections.end() && deadIt != m_closedConnections.end())
                                {
                                   Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::updateLoop()", "Connection for {} recently closed, sending Disconnect/Deny reply.", ipStr);
                                    sendSimple(sender, m_denyConnOP);
                                }
                                else
                                {
                                    /// \TODO: Some kind of logging to catch this
                                   Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::updateLoop()", "Connection for {} *ALREADY* exists. [Count: {}]", ipStr, m_netConnections.size());
                                }
                            }
                            // normal traffic
                            else if(it != m_netConnections.end())
                            {
                                switch(p->op_code)
                                {
                                    case OP_RetransmissionAck:
                                    {
                                        /// \TODO: Destroy sequence within connection
                                       Logger::getInstance().Log(Logs::DEBUG, "NetworkServer::updateLoop()", "Received retransmissions ACK! seqID [{}], pktNum [{}]", p->seqIdent, p->pktNum);
                                        //it->second->receiveRetryAck(p->seqIdent, p->pktNum);
                                        it->second->addPacket(&p);
                                        break;
                                    }
                                    case OP_RetransmissionImpossible:
                                    {
                                        /// \TODO: Destroy sequence within connection
                                       Logger::getInstance().Log(Logs::DEBUG, "NetworkServer::updateLoop()", "Received retransmissions impossible! seqID [{}], pktNum [{}]", p->seqIdent, p->pktNum);
                                        it->second->addPacket(&p);
                                        break;
                                    }
                                    case OP_ConnectionDisconnect:
                                    {
                                       Logger::getInstance().Log(Logs::DEBUG, "NetworkServer::updateLoop()", "Closing connection for {} [New Count: {}]", ipStr, m_netConnections.size()-1);
                                        Datagram* d = new Datagram(OP_ConnectionDisconnect, it->second->getUniqueID());
                                        m_datagramBuffer->push(d);
                                        it->second->setClosed(true);
                                        m_closedConnections.insert(ipStr, Time::getInstance().now()+5);
                                        break;
                                    }
                                    case OP_Ack: // sequence has completed or destroyed, let sender know
                                    {
                                       Logger::getInstance().Log(Logs::DEBUG, "NetworkServer::updateLoop()", "Received ACK for seqID [{}].", p->seqIdent);
                                        it->second->addPacket(&p);
                                        break;
                                    }
                                    default: // normal packets
                                    {
                                        //Logger::getInstance().Log(Logs::DEBUG, "NetworkServer::updateLoop()", "'default' case hit, OP [{}]", p->op_code);
                                        it->second->addPacket(&p); // add packet to connection
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                // error?
                               Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::updateLoop()", "'else' statement in updateLoop with OPCode [{}], this is bad. IP [{}]", p->op_code, ipStr);
                            }

                            // cleanup
                            nc = nullptr;
                        }
                    }

                    /// \TODO: Find out if this is a memory leak or not
                    safeDelete(p);
                    p = nullptr;
                    sender = nullptr;
                    m_packetBuffer.pop();
                    pkts--;
                }

                // timer to prevent wasteful CPU usage
                if(connUpdate.isExpired())
                {
                    uint64_t timestamp = Time::getInstance().nowMS();

                    // check for closed connections
                    for(std::unordered_map<std::string, NetConnection*>::iterator nit = m_netConnections.begin(); nit != m_netConnections.end(); ++nit)
                    {
                        bool destroy = nit->second->update(timestamp);
                        if(destroy) { toBeClosed.push(nit->first); }
                    }

                    // delete netConnection object, remove from uo_map
                    while(toBeClosed.size() > 0)
                    {
                       Logger::getInstance().Log(Logs::VERBOSE, Logs::Network, "NetworkServer::updateLoop()", "Closing connection [{}]", toBeClosed.front());
                        safeDelete(m_netConnections.at(toBeClosed.front()));
                        m_netConnections.erase(toBeClosed.front());
                        toBeClosed.pop();
                    }

                    // remove aged-out stored packet sequences
                    std::queue<uint32_t> expiredToRemove;
                    uint64_t time = Time::getInstance().nowMS();
                    for(std::unordered_map<uint32_t, StoredSequence*>::iterator sit = m_storedSequences.begin(); sit != m_storedSequences.end(); ++sit)
                    {
                        if(sit->second->isExpired(time)) { expiredToRemove.push(sit->first); }
                    }

                    while(expiredToRemove.size() > 0)
                    {
                        safeDelete(m_storedSequences.at(expiredToRemove.front()));
                        m_storedSequences.erase(expiredToRemove.front());
                        expiredToRemove.pop();
                    }

                    // remove connections from 'recently disconnected'
                    std::queue<std::string> closedToRemove;
                    for(auto itr = m_closedConnections.begin(); itr != m_closedConnections.end(); ++itr)
                    {
                        if(itr->second < timestamp) { closedToRemove.push(itr->first); }
                    }

                    while(closedToRemove.size() > 0)
                    {
                        m_closedConnections.erase(closedToRemove.front());
                        closedToRemove.pop();
                    }

                    connUpdate.restart();
                }
            }
        }

        // release lock
        updateLock.unlock();

       Logger::getInstance().Log(Logs::INFO, Logs::Network, "NetworkServer::updateLoop()", "Exiting updateLoop().");
    }

/*  void NetworkServer::addPeer(NetworkPeer* peer)
    {
        if(!peer) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkServer::addPeer()", "No NetworkPeer pointer passed!"); return; }

        // check if already exists
        if(m_internalConnections.find(peer->getUniqueID()) == m_internalConnections.end())
        {
            m_internalConnections.insert(peer->getUniqueID(), peer);
        }
        else
        {
           Logger::getInstance().Log(Logs::WARN, Logs::Network, "NetworkServer::addPeer()", "NetworkPeer already exists in m_internalConnections.");
        }
    }*/

/*    void NetworkServer::changeBuffer(NetConnection* nc, NetworkPeer* np)
    {
        if(!nc) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkServer::changeBuffer()", "NetworkConnection ptr was null!"); }
        else if(!np) { Logger::getInstance().Log(Logs::CRIT, Logs::Network, "NetworkServer::changeBuffer()", "NetworkPeer ptr was null!"); }
        else { nc->setDatagramBuffer(np->getSendDGQueue()); }
    }*/

    /// NetworkServer private functions below ///////////////////////////////////////
}









