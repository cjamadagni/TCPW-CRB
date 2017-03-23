/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 NITK Surathkal
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
 * Authors: Chirag Jamadagni (chirag.jamadagni@gmail.com)
 *          Amita Ajith Kamath (amita.kamath95@gmail.com)
 */

#include "ns3/test.h"
#include "ns3/log.h"
#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-westwood.h"
#include "ns3/string.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpWestwoodTestSuite");

/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief Test case for TCP Westwood
 */

class TcpWestwoodTest : public TestCase
{
public:
  /**
   * \brief Constructor
   *
   * \param cWnd congestion window
   * \param segmentSize segment size
   * \param ssThresh slow start threshold
   * \param packetsAcked packets acked
   * \param rtt RTT
   * \param name Name of the test
   */

  TcpWestwoodTest (uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                   uint32_t packetsAcked, Time rtt, const std::string &name);

private:
  virtual void DoRun (void);
  /** \brief Execute the test
   */
  void ExecuteTest (void);

  uint32_t m_cWnd;               //!< Congestion window
  uint32_t m_segmentSize;        //!< Segment size
  uint32_t m_packetsAcked;       //!< Number of packets acknowledged
  uint32_t m_ssThresh;           //!< Slow start threshold
  Time m_rtt;                    //!< Round trip time
  Ptr<TcpSocketState> m_state;   //!< State
};

TcpWestwoodTest::TcpWestwoodTest (uint32_t cWnd, uint32_t segmentSize, uint32_t ssThresh,
                                  uint32_t packetsAcked, Time rtt, const std::string &name)
  : TestCase (name),
    m_cWnd (cWnd),
    m_segmentSize (segmentSize),
    m_packetsAcked (packetsAcked),
    m_ssThresh (ssThresh),
    m_rtt (rtt)
{
}

void
TcpWestwoodTest::DoRun ()
{
  Simulator::Schedule (Seconds (0.0), &TcpWestwoodTest::ExecuteTest, this);
  Simulator::Run ();
  Simulator::Destroy ();
}

void
TcpWestwoodTest::ExecuteTest ()
{
  m_state = CreateObject <TcpSocketState> ();
  m_state->m_cWnd = m_cWnd;
  m_state->m_ssThresh = m_ssThresh;
  m_state->m_segmentSize = m_segmentSize;

  Ptr<TcpWestwood> cong = CreateObject <TcpWestwood> ();

  cong->PktsAcked (m_state,m_packetsAcked,m_rtt);
  m_state->m_ssThresh = cong->GetSsThresh (m_state, 32);
  m_packetsAcked++;
  m_rtt = MilliSeconds (105);

  cong->PktsAcked (m_state,m_packetsAcked,m_rtt);
  m_state->m_ssThresh = cong->GetSsThresh (m_state, 32);
  m_packetsAcked++;
  m_rtt = MilliSeconds (80);

  cong->PktsAcked (m_state,m_packetsAcked,m_rtt);
  m_state->m_ssThresh = cong->GetSsThresh (m_state, 32);
  m_packetsAcked++;
  m_rtt = MilliSeconds (100);

  cong->PktsAcked (m_state,m_packetsAcked,m_rtt);
  m_state->m_ssThresh = cong->GetSsThresh (m_state, 32);
  m_packetsAcked++;
  m_rtt = MilliSeconds (110);

  cong->PktsAcked (m_state,m_packetsAcked,m_rtt);
  m_state->m_ssThresh = cong->GetSsThresh (m_state, 32);
  m_packetsAcked++;
  m_rtt = MilliSeconds (140);

  cong->PktsAcked (m_state,m_packetsAcked,m_rtt);
  m_state->m_ssThresh = cong->GetSsThresh (m_state, 32);
  m_packetsAcked++;
  m_rtt = MilliSeconds (150);

  cong->PktsAcked (m_state,m_packetsAcked,m_rtt);
  m_state->m_ssThresh = cong->GetSsThresh (m_state, 32);
  m_packetsAcked++;
  m_rtt = MilliSeconds (190);

  cong->PktsAcked (m_state,m_packetsAcked,m_rtt);
  m_state->m_ssThresh = cong->GetSsThresh (m_state, 32);


  NS_TEST_ASSERT_MSG_EQ (m_state->m_ssThresh.Get (), 3994U,
                         "ssThresh has not updated correctly");
}

/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief TCP Westwood TestSuite
 */
class TcpWestwoodTestSuite : public TestSuite
{
public:
  TcpWestwoodTestSuite () : TestSuite ("tcp-westwood-test", UNIT)
  {
    AddTestCase (new TcpWestwoodTest (2 * 1446, 1446, 4 * 1446, 4, MilliSeconds (100), "Testing ssThresh value of Westwood"), TestCase::QUICK);
  }
};

static TcpWestwoodTestSuite g_tcpwestwoodTest;     //!< static var for test initialization
