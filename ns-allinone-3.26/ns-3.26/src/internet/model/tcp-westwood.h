/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 * and Greeshma Umapathi
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#ifndef TCP_WESTWOOD_H
#define TCP_WESTWOOD_H

#include "tcp-congestion-ops.h"
#include "ns3/sequence-number.h"
#include "ns3/traced-value.h"

namespace ns3 {

class Packet;
class TcpHeader;
class Time;
class EventId;

/**
 * \ingroup congestionOps
 *
 * \brief An implementation of TCP Westwood, WestwoodCRB and Westwood+.
 *
 * Westwood, WestwoodCRB and Westwood+ employ the AIAD (Additive Increase/Adaptive Decrease) 
 * congestion control paradigm. When a congestion episode happens, 
 * instead of halving the cwnd, these protocols try to estimate the network's
 * bandwidth and use the estimated value to adjust the cwnd. 
 * While Westwood performs the bandwidth sampling every ACK reception, 
 * Westwood+ samples the bandwidth every RTT <FILL IN THE BLANKS>.
 *
 * The three main methods in the implementation are the CountAck (const TCPHeader&),
 * the EstimateBW (int, const, Time) and the EstimateRE (int, const, Time). The CountAck method calculates
 * the number of acknowledged segments on the receipt of an ACK.
 * The EstimateBW estimates the bandwidth based on the value returned by CountAck
 * and the sampling interval (last ACK inter-arrival time for Westwood and last RTT for Westwood+).
 * The EstimateRE <FILL IN THE BLANKS>

 */
class TcpWestwood : public TcpNewReno
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TcpWestwood (void);
  /**
   * \brief Copy constructor
   * \param sock the object to copy
   */
  TcpWestwood (const TcpWestwood& sock);
  virtual ~TcpWestwood (void);

  /**
   * \brief Protocol variant (Westwood, Westwood+, WestwoodCRB)
   */
  enum ProtocolType 
  {
    WESTWOOD,
    WESTWOODPLUS,
    WESTWOODCRB
  };

  /**
   * \brief Filter type (None or Tustin)
   */
  enum FilterType 
  {
    NONE,
    TUSTIN
  };

  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);

  virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                          const Time& rtt);

  virtual Ptr<TcpCongestionOps> Fork ();

private:
  /**
   * Update the total number of acknowledged packets during the current RTT
   *
   * \param [in] acked the number of packets the currently received ACK acknowledges
   */
  void UpdateAckedSegments (int acked);

  /**
   * Estimate the network's bandwidth
   *
   * \param [in] rtt the RTT estimation.
   * \param [in] tcb the socket state.
   */
  void EstimateBW (const Time& rtt, Ptr<TcpSocketState> tcb);
  
  /**
   * \brief Estimate the network's RE
   *
   * \param [in] m_tvalue the T value used in the estimation of RE.
   * \param [in] tcb the socket state.
   */
  void EstimateRE (const Time& m_tvalue, Ptr<TcpSocketState> tcb);
        
  

protected:
  TracedValue<double>    m_currentBW;              //!< Current value of the estimated BW
  TracedValue<double>    m_currentRE;              //!< Current value of the estimated RE     
  Time                   m_tvalue;                 //!< T value used in Westwood CRB
  
  double                 m_lastSampleBW;           //!< Last bandwidth sample
  double                 m_lastBW;                 //!< Last bandwidth sample after being filtered
  
  double                 m_lastSampleRE;           //!< Last rate estimation (RE) sample
  double                 m_lastRE;                 //!< Last rate estimation (RE) sample after being filtered
  
  Time                   m_minRtt;                 //!< Minimum RTT
  enum ProtocolType      m_pType;                  //!< 0 for Westwood, 1 for Westwood+
  enum FilterType        m_fType;                  //!< 0 for none, 1 for Tustin

  int                    m_ackedSegments;          //!< The number of segments ACKed between RTTs
  int                    m_ackedSinceT;            //!< The number of segments ACKed between T values in WestwoodCRB
  bool                   m_IsCount;                //!< Start keeping track of m_ackedSegments for Westwood+ if TRUE
  EventId                m_bwEstimateEvent;        //!< The BW estimation event for Westwood+
  EventId                m_reEstimateEvent;        //!< The RE estimation event for WestwoodCRB

};

} // namespace ns3

#endif /* TCP_WESTWOOD_H */
