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
 * Authors: Siddharth Gangadhar <siddharth@ittc.ku.edu>,
 *          Truc Anh N. Nguyen <annguyen@ittc.ku.edu>,
 *          Greeshma Umapathi
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

#include "tcp-westwood.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "rtt-estimator.h"
#include "tcp-socket-base.h"

NS_LOG_COMPONENT_DEFINE ("TcpWestwood");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpWestwood);

TypeId
TcpWestwood::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::TcpWestwood")
    .SetParent<TcpNewReno>()
    .SetGroupName ("Internet")
    .AddConstructor<TcpWestwood>()
    .AddAttribute("FilterType", "Use this to choose no filter or Tustin's approximation filter",
                  EnumValue(TcpWestwood::TUSTIN), MakeEnumAccessor(&TcpWestwood::m_fType),
                  MakeEnumChecker(TcpWestwood::NONE, "None", TcpWestwood::TUSTIN, "Tustin"))
    .AddAttribute("ProtocolType", "Use this to let the code run as Westwood or WestwoodPlus",
                  EnumValue(TcpWestwood::WESTWOOD),
                  MakeEnumAccessor(&TcpWestwood::m_pType),
                  MakeEnumChecker(TcpWestwood::WESTWOOD, "Westwood",TcpWestwood::WESTWOODPLUS, "WestwoodPlus",TcpWestwood::WESTWOODCRB, "WestwoodCRB"))
    .AddTraceSource("EstimatedBW", "The estimated bandwidth",
                    MakeTraceSourceAccessor(&TcpWestwood::m_currentBW),
                    "ns3::TracedValueCallback::Double")
    .AddTraceSource("EstimatedRE", "The estimated rate",
                    MakeTraceSourceAccessor(&TcpWestwood::m_currentRE),
                    "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

TcpWestwood::TcpWestwood (void) :
  TcpNewReno (),
  m_currentBW (0),
  m_currentRE (0),
  m_tvalue (Seconds (0.4)),
  m_lastSampleBW (0),
  m_lastBW (0),
  m_lastSampleRE (0),
  m_lastRE (0),
  m_minRtt (Time (0)),
  m_ackedSegments (0),
  m_ackedSinceT (0),
  m_IsCount (false)
{
  NS_LOG_FUNCTION (this);
}

TcpWestwood::TcpWestwood (const TcpWestwood& sock) :
  TcpNewReno (sock),
  m_currentBW (sock.m_currentBW),
  m_currentRE (sock.m_currentRE),
  m_lastSampleBW (sock.m_lastSampleBW),
  m_lastBW (sock.m_lastBW),
  m_lastSampleRE (sock.m_lastSampleRE),
  m_lastRE (sock.m_lastRE),
  m_minRtt (Time (0)),
  m_pType (sock.m_pType),
  m_fType (sock.m_fType),
  m_IsCount (sock.m_IsCount)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("Invoked the copy constructor");
}

TcpWestwood::~TcpWestwood (void)
{
}


void
TcpWestwood::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t packetsAcked,
                        const Time& rtt)
{
   
  
  NS_LOG_FUNCTION (this << tcb << packetsAcked << rtt);

  if (rtt.IsZero ())
    {
      NS_LOG_WARN ("RTT measured is zero!");
      return;
    }

  m_ackedSegments += packetsAcked;
  m_ackedSinceT += packetsAcked;
  // Update minRtt
  if (m_minRtt.IsZero ())
    {
      m_minRtt = rtt;
    }
  else
    {
      if (rtt < m_minRtt)
        {
          m_minRtt = rtt;
        }
    }

  NS_LOG_LOGIC ("MinRtt: " << m_minRtt.GetMilliSeconds () << "ms");

  if (m_pType == TcpWestwood::WESTWOOD)
    {
      EstimateBW (rtt, tcb);
    }
  else if (m_pType == TcpWestwood::WESTWOODPLUS)
    {
      if (!(rtt.IsZero () || m_IsCount))
        {
          m_IsCount = true;
          m_bwEstimateEvent.Cancel ();
          m_bwEstimateEvent = Simulator::Schedule (rtt, &TcpWestwood::EstimateBW,
                                                   this, rtt, tcb);
        }
    }
   else if (m_pType == TcpWestwood::WESTWOODCRB)
   {
         if (!m_IsCount)
        {
          m_IsCount = true;
          m_reEstimateEvent.Cancel ();
          m_reEstimateEvent = Simulator::Schedule (m_tvalue, &TcpWestwood::EstimateRE,
                                                   this, m_tvalue, tcb);
        } 
      EstimateBW(rtt,tcb);    
   
   
   }
}


void
TcpWestwood::EstimateRE (const Time &tvalue, Ptr<TcpSocketState> tcb)
{
 
  NS_LOG_FUNCTION (this);
  
  NS_ASSERT (!tvalue.IsZero ());
    
  m_currentRE = m_ackedSinceT * tcb->m_segmentSize / tvalue.GetSeconds ();
    
  m_ackedSinceT = 0;
      
  if (m_pType == TcpWestwood::WESTWOODCRB)
    {
      m_IsCount = false;
    }

  // Filter the BW sample
  double alpha = 0.9; 

  if (m_fType == TcpWestwood::NONE)
    {
   
    }
  else if (m_fType == TcpWestwood::TUSTIN)
    {
      double sample_ree = m_currentRE;
      m_currentRE = (alpha * m_lastRE) + ((1 - alpha) * ((sample_ree + m_lastSampleRE) / 2));
      m_lastSampleRE = sample_ree;
      m_lastRE = m_currentRE;
    }
   
  NS_LOG_LOGIC ("Estimated RE after filtering: " << m_currentRE);
}

void
TcpWestwood::EstimateBW (const Time &rtt, Ptr<TcpSocketState> tcb)
{

  NS_LOG_FUNCTION (this);

  NS_ASSERT (!rtt.IsZero ());

  m_currentBW = m_ackedSegments * tcb->m_segmentSize / rtt.GetSeconds ();

  if (m_pType == TcpWestwood::WESTWOODPLUS)
    {
      m_IsCount = false;
    }

  m_ackedSegments = 0;
  NS_LOG_LOGIC ("Estimated BW: " << m_currentBW);

  // Filter the BW sample
  double alpha = 0.9;

  if (m_fType == TcpWestwood::NONE)
    {
    }
  else if (m_fType == TcpWestwood::TUSTIN)
    {
      double sample_bwe = m_currentBW;
      m_currentBW = (alpha * m_lastBW) + ((1 - alpha) * ((sample_bwe + m_lastSampleBW) / 2));
      m_lastSampleBW = sample_bwe;
      m_lastBW = m_currentBW;
    }

  NS_LOG_LOGIC ("Estimated BW after filtering: " << m_currentBW);
}

uint32_t
TcpWestwood::GetSsThresh (Ptr<const TcpSocketState> tcb,
                          uint32_t bytesInFlight)
{

  (void) bytesInFlight;
  NS_LOG_LOGIC ("CurrentBW: " << m_currentBW << " minRtt: " <<
                m_minRtt << " ssthresh: " <<
                m_currentBW * static_cast<double> (m_minRtt.GetSeconds ()));
 
 
  double theta = 1.4; 
 
  // in TCPW-CRB, if the min_RTT is zero, we return the BW estimate instead of the RE estimate
  if(m_minRtt.GetSeconds () !=0 && m_currentRE!=0 && (tcb->m_cWnd/(uint32_t (m_currentRE * static_cast<double> (m_minRtt.GetSeconds ()))) < theta))
  {
     return std::max (2*tcb->m_segmentSize,
                   uint32_t (m_currentRE * static_cast<double> (m_minRtt.GetSeconds ())));
  }
  
  else
  { 
    return std::max (2*tcb->m_segmentSize,
                   uint32_t (m_currentBW * static_cast<double> (m_minRtt.GetSeconds ())));
  }

  
}

Ptr<TcpCongestionOps>
TcpWestwood::Fork ()
{
  return CreateObject<TcpWestwood> (*this);
}

} // namespace ns3
