/**
 * @file dnode.cpp
 * @author slguan (slguan@taosdata.com)
 * @brief MNODE module dnode tests
 * @version 1.0
 * @date 2022-01-06
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "sut.h"

class MndTestDnode : public ::testing::Test {
 public:
  void SetUp() override {}
  void TearDown() override {}

 public:
  static void SetUpTestSuite() {
    test.Init("/tmp/dnode_test_dnode1", 9023);
    const char* fqdn = "localhost";
    const char* firstEp = "localhost:9023";

    server2.Start("/tmp/dnode_test_dnode2", fqdn, 9024, firstEp);
    server3.Start("/tmp/dnode_test_dnode3", fqdn, 9025, firstEp);
    server4.Start("/tmp/dnode_test_dnode4", fqdn, 9026, firstEp);
    server5.Start("/tmp/dnode_test_dnode5", fqdn, 9027, firstEp);
    taosMsleep(300);
  }

  static void TearDownTestSuite() {
    server2.Stop();
    server3.Stop();
    server4.Stop();
    server5.Stop();
    test.Cleanup();
  }

  static Testbase   test;
  static TestServer server2;
  static TestServer server3;
  static TestServer server4;
  static TestServer server5;
};

Testbase   MndTestDnode::test;
TestServer MndTestDnode::server2;
TestServer MndTestDnode::server3;
TestServer MndTestDnode::server4;
TestServer MndTestDnode::server5;

TEST_F(MndTestDnode, 01_ShowDnode) {
  test.SendShowMetaReq(TSDB_MGMT_TABLE_DNODE, "");
  CHECK_META("show dnodes", 7);

  CHECK_SCHEMA(0, TSDB_DATA_TYPE_SMALLINT, 2, "id");
  CHECK_SCHEMA(1, TSDB_DATA_TYPE_BINARY, TSDB_EP_LEN + VARSTR_HEADER_SIZE, "endpoint");
  CHECK_SCHEMA(2, TSDB_DATA_TYPE_SMALLINT, 2, "vnodes");
  CHECK_SCHEMA(3, TSDB_DATA_TYPE_SMALLINT, 2, "support_vnodes");
  CHECK_SCHEMA(4, TSDB_DATA_TYPE_BINARY, 10 + VARSTR_HEADER_SIZE, "status");
  CHECK_SCHEMA(5, TSDB_DATA_TYPE_TIMESTAMP, 8, "create_time");
  CHECK_SCHEMA(6, TSDB_DATA_TYPE_BINARY, 24 + VARSTR_HEADER_SIZE, "offline_reason");

  test.SendShowRetrieveReq();
  EXPECT_EQ(test.GetShowRows(), 1);

  CheckInt16(1);
  CheckBinary("localhost:9023", TSDB_EP_LEN);
  CheckInt16(0);
  CheckInt16(16);
  CheckBinary("ready", 10);
  CheckTimestamp();
  CheckBinary("", 24);
}

TEST_F(MndTestDnode, 02_ConfigDnode) {
  int32_t contLen = sizeof(SMCfgDnodeReq);

  SMCfgDnodeReq* pReq = (SMCfgDnodeReq*)rpcMallocCont(contLen);
  pReq->dnodeId = htonl(1);
  strcpy(pReq->config, "ddebugflag 131");

  SRpcMsg* pRsp = test.SendReq(TDMT_MND_CONFIG_DNODE, pReq, contLen);
  ASSERT_NE(pRsp, nullptr);
  ASSERT_EQ(pRsp->code, 0);
}

TEST_F(MndTestDnode, 03_Create_Dnode) {
  {
    int32_t contLen = sizeof(SCreateDnodeReq);

    SCreateDnodeReq* pReq = (SCreateDnodeReq*)rpcMallocCont(contLen);
    strcpy(pReq->fqdn, "");
    pReq->port = htonl(9024);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_CREATE_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, TSDB_CODE_MND_INVALID_DNODE_EP);
  }

  {
    int32_t contLen = sizeof(SCreateDnodeReq);

    SCreateDnodeReq* pReq = (SCreateDnodeReq*)rpcMallocCont(contLen);
    strcpy(pReq->fqdn, "localhost");
    pReq->port = htonl(-1);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_CREATE_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, TSDB_CODE_MND_INVALID_DNODE_EP);
  }

  {
    int32_t contLen = sizeof(SCreateDnodeReq);

    SCreateDnodeReq* pReq = (SCreateDnodeReq*)rpcMallocCont(contLen);
    strcpy(pReq->fqdn, "localhost");
    pReq->port = htonl(123456);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_CREATE_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, TSDB_CODE_MND_INVALID_DNODE_EP);
  }

  {
    int32_t contLen = sizeof(SCreateDnodeReq);

    SCreateDnodeReq* pReq = (SCreateDnodeReq*)rpcMallocCont(contLen);
    strcpy(pReq->fqdn, "localhost");
    pReq->port = htonl(9024);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_CREATE_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, 0);
  }

  {
    int32_t contLen = sizeof(SCreateDnodeReq);

    SCreateDnodeReq* pReq = (SCreateDnodeReq*)rpcMallocCont(contLen);
    strcpy(pReq->fqdn, "localhost");
    pReq->port = htonl(9024);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_CREATE_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, TSDB_CODE_MND_DNODE_ALREADY_EXIST);
  }

  taosMsleep(1300);

  test.SendShowMetaReq(TSDB_MGMT_TABLE_DNODE, "");
  CHECK_META("show dnodes", 7);
  test.SendShowRetrieveReq();
  EXPECT_EQ(test.GetShowRows(), 2);

  CheckInt16(1);
  CheckInt16(2);
  CheckBinary("localhost:9023", TSDB_EP_LEN);
  CheckBinary("localhost:9024", TSDB_EP_LEN);
  CheckInt16(0);
  CheckInt16(0);
  CheckInt16(16);
  CheckInt16(16);
  CheckBinary("ready", 10);
  CheckBinary("ready", 10);
  CheckTimestamp();
  CheckTimestamp();
  CheckBinary("", 24);
  CheckBinary("", 24);
}

TEST_F(MndTestDnode, 04_Drop_Dnode) {
  {
    int32_t contLen = sizeof(SDropDnodeReq);

    SDropDnodeReq* pReq = (SDropDnodeReq*)rpcMallocCont(contLen);
    pReq->dnodeId = htonl(-3);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_DROP_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, TSDB_CODE_MND_INVALID_DNODE_ID);
  }

  {
    int32_t contLen = sizeof(SDropDnodeReq);

    SDropDnodeReq* pReq = (SDropDnodeReq*)rpcMallocCont(contLen);
    pReq->dnodeId = htonl(5);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_DROP_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, TSDB_CODE_MND_DNODE_NOT_EXIST);
  }

  {
    int32_t contLen = sizeof(SDropDnodeReq);

    SDropDnodeReq* pReq = (SDropDnodeReq*)rpcMallocCont(contLen);
    pReq->dnodeId = htonl(2);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_DROP_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, 0);
  }

  {
    int32_t contLen = sizeof(SDropDnodeReq);

    SDropDnodeReq* pReq = (SDropDnodeReq*)rpcMallocCont(contLen);
    pReq->dnodeId = htonl(2);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_DROP_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, TSDB_CODE_MND_DNODE_NOT_EXIST);
  }

  test.SendShowMetaReq(TSDB_MGMT_TABLE_DNODE, "");
  CHECK_META("show dnodes", 7);
  test.SendShowRetrieveReq();
  EXPECT_EQ(test.GetShowRows(), 1);

  CheckInt16(1);
  CheckBinary("localhost:9023", TSDB_EP_LEN);
  CheckInt16(0);
  CheckInt16(16);
  CheckBinary("ready", 10);
  CheckTimestamp();
  CheckBinary("", 24);

  taosMsleep(2000);
  server2.Stop();
  server2.DoStart();
}

TEST_F(MndTestDnode, 05_Create_Drop_Restart_Dnode) {
  {
    int32_t contLen = sizeof(SCreateDnodeReq);

    SCreateDnodeReq* pReq = (SCreateDnodeReq*)rpcMallocCont(contLen);
    strcpy(pReq->fqdn, "localhost");
    pReq->port = htonl(9025);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_CREATE_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, 0);
  }

  {
    int32_t contLen = sizeof(SCreateDnodeReq);

    SCreateDnodeReq* pReq = (SCreateDnodeReq*)rpcMallocCont(contLen);
    strcpy(pReq->fqdn, "localhost");
    pReq->port = htonl(9026);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_CREATE_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, 0);
  }

  {
    int32_t contLen = sizeof(SCreateDnodeReq);

    SCreateDnodeReq* pReq = (SCreateDnodeReq*)rpcMallocCont(contLen);
    strcpy(pReq->fqdn, "localhost");
    pReq->port = htonl(9027);

    SRpcMsg* pRsp = test.SendReq(TDMT_MND_CREATE_DNODE, pReq, contLen);
    ASSERT_NE(pRsp, nullptr);
    ASSERT_EQ(pRsp->code, 0);
  }

  taosMsleep(1300);
  test.SendShowMetaReq(TSDB_MGMT_TABLE_DNODE, "");
  CHECK_META("show dnodes", 7);
  test.SendShowRetrieveReq();
  EXPECT_EQ(test.GetShowRows(), 4);

  CheckInt16(1);
  CheckInt16(3);
  CheckInt16(4);
  CheckInt16(5);
  CheckBinary("localhost:9023", TSDB_EP_LEN);
  CheckBinary("localhost:9025", TSDB_EP_LEN);
  CheckBinary("localhost:9026", TSDB_EP_LEN);
  CheckBinary("localhost:9027", TSDB_EP_LEN);
  CheckInt16(0);
  CheckInt16(0);
  CheckInt16(0);
  CheckInt16(0);
  CheckInt16(16);
  CheckInt16(16);
  CheckInt16(16);
  CheckInt16(16);
  CheckBinary("ready", 10);
  CheckBinary("ready", 10);
  CheckBinary("ready", 10);
  CheckBinary("ready", 10);
  CheckTimestamp();
  CheckTimestamp();
  CheckTimestamp();
  CheckTimestamp();
  CheckBinary("", 24);
  CheckBinary("", 24);
  CheckBinary("", 24);
  CheckBinary("", 24);

  // restart
  uInfo("stop all server");
  test.Restart();
  server2.Restart();
  server3.Restart();
  server4.Restart();
  server5.Restart();

  taosMsleep(1300);
  test.SendShowMetaReq(TSDB_MGMT_TABLE_DNODE, "");
  CHECK_META("show dnodes", 7);
  test.SendShowRetrieveReq();
  EXPECT_EQ(test.GetShowRows(), 4);

  CheckInt16(1);
  CheckInt16(3);
  CheckInt16(4);
  CheckInt16(5);
  CheckBinary("localhost:9023", TSDB_EP_LEN);
  CheckBinary("localhost:9025", TSDB_EP_LEN);
  CheckBinary("localhost:9026", TSDB_EP_LEN);
  CheckBinary("localhost:9027", TSDB_EP_LEN);
  CheckInt16(0);
  CheckInt16(0);
  CheckInt16(0);
  CheckInt16(0);
  CheckInt16(16);
  CheckInt16(16);
  CheckInt16(16);
  CheckInt16(16);
  CheckBinary("ready", 10);
  CheckBinary("ready", 10);
  CheckBinary("ready", 10);
  CheckBinary("ready", 10);
  CheckTimestamp();
  CheckTimestamp();
  CheckTimestamp();
  CheckTimestamp();
  CheckBinary("", 24);
  CheckBinary("", 24);
  CheckBinary("", 24);
  CheckBinary("", 24);
}
