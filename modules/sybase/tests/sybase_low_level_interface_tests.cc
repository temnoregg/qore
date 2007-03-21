#ifdef DEBUG

#include "sybase_tests_common.h"
#include <qore/QoreNode.h>
#include <math.h>

namespace sybase_tests_78620983 {

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_commit()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  int res = sybase_low_level_commit(&conn, &xsink);
  if (res != 1) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_rollback()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  int res = sybase_low_level_rollback(&conn, &xsink);
  if (res != 1) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // several commit/rollbacks
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  int res = sybase_low_level_commit(&conn, &xsink);
  if (res != 1) {
    assert(false);
  }
  res = sybase_low_level_commit(&conn, &xsink);
  assert(res == 1);
  res = sybase_low_level_rollback(&conn, &xsink);
  assert(res == 1);
  res = sybase_low_level_commit(&conn, &xsink);
  assert(res == 1);
  res = sybase_low_level_rollback(&conn, &xsink);
  assert(res == 1);
  res = sybase_low_level_commit(&conn, &xsink);
  assert(res == 1);
  res = sybase_low_level_rollback(&conn, &xsink);
  assert(res == 1);
}

//------------------------------------------------------------------------------
TEST()
{
  // test direct command execution (create table, insert something, drop)
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  // cleanup
  const char* cmd = "drop table my_test_672";
  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    xsink.clear();
  }

  // test start
  cmd =  "create table my_test_672 ("
    "int_col INTEGER, "
    "varchar_col VARCHAR(30))";

  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  cmd = "insert into my_test_672 values(11, 'aaa')";
  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  cmd = "insert into my_test_672 (varchar_col, int_col) values('bbb', 22)";
  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  int res = sybase_low_level_commit(&conn, &xsink);
  assert(res == 1);

  cmd = "drop table my_test_672";
  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  res = sybase_low_level_commit(&conn, &xsink);
  assert(res == 1);
}

//------------------------------------------------------------------------------
TEST()
{
  // test failing direct command execution
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  // cleanup
  const char* cmd = "drop table my_test_173";
  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    xsink.clear();
  }

  cmd =  "create table my_test_173 ("
    "int_col INTEGER, "
    "varchar_col VARCHAR(30))";

  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  int res = sybase_low_level_commit(&conn, &xsink);
  assert(res == 1);

  cmd = "select count(*) from my_test_173";
  for (int i = 0; i < 100; ++i) {
    sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
    if (xsink.isException()) {
      xsink.clear();
    } else {
      assert(false);
    }
  }

  cmd = "drop table my_test_173";
  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  res = sybase_low_level_commit(&conn, &xsink);
  assert(res == 1);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_command_wrapper class
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // test several sybase_command_wrappers together
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w1(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  sybase_command_wrapper w2(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  sybase_command_wrapper w3(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_prepare_command() for a command
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  // cleanup
  const char* cmd = "drop table my_test_253";
  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    xsink.clear();
  }

  sybase_low_level_prepare_command(w, "create table my_test_253 (int_col INTEGER)", &xsink);
  if (xsink.isException()) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_prepare_command() for a select
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "select * from syskeys", &xsink);
  if (xsink.isException()) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_prepare_command() for a procedure
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "exec sp_transactions", &xsink);
  if (xsink.isException()) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_input_parameters_info() for a command
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  // cleanup
  const char* cmd = "drop table my_test_254";
  sybase_low_level_execute_directly_command(conn.getConnection(), cmd, &xsink);
  if (xsink.isException()) {
    xsink.clear();
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "create table my_test_254 (int_col INTEGER)", &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> inputs = sybase_low_level_get_input_parameters_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(inputs.size() == 0);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_input_parameters_info() 
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "select * from syskeys where id = 1", &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> inputs = sybase_low_level_get_input_parameters_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(inputs.size() == 0);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_input_parameters_info()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "select * from syskeys where id = ?", &xsink); // 1 input parameter
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> inputs = sybase_low_level_get_input_parameters_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(inputs.size() == 1);
  assert(inputs[0].m_name == "unnamed parameter #1");
  assert(inputs[0].m_type == CS_INT_TYPE);
  assert(inputs[0].m_max_size == sizeof(CS_INT));
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_input_parameters_info() 
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "select * from syskeys where id = ? or id > ?", &xsink); // 2 input parameters
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> inputs = sybase_low_level_get_input_parameters_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(inputs.size() == 2);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_input_parameters_info()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "exec sp_help", &xsink); 
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> inputs = sybase_low_level_get_input_parameters_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(inputs.size() == 0);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_input_parameters_info()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "exec sp_drop_qplan 11111", &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> inputs = sybase_low_level_get_input_parameters_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(inputs.size() == 0);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_input_parameters_info()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "execute sp_drop_qplan", &xsink); // parameter omitted
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> inputs = sybase_low_level_get_input_parameters_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(inputs.size() == 0);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_output_data_info() for a command
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "create table my_test_253 (int_col INTEGER)", &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> outputs = sybase_low_level_get_output_data_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(outputs.size() == 0);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_output_data_info()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "select * from syskeys where id = 1", &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> outputs = sybase_low_level_get_output_data_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(outputs.size() == 22); // on my machine
  assert(outputs[0].m_name == "id");
  assert(outputs[0].m_type == CS_INT_TYPE);
  assert(outputs[0].m_max_size == sizeof(CS_INT));
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_output_data_info()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_command_wrapper w(conn, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  sybase_low_level_prepare_command(w, "execute sp_transactions", &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  std::vector<parameter_info_t> outputs = sybase_low_level_get_output_data_info(w, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(outputs.size() == 0);
}

//------------------------------------------------------------------------------
TEST()
{
  // test sybase_low_level_get_default_encoding()
  printf("running test %s[%d]\n", __FILE__, __LINE__);
  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  for (unsigned i = 0; i < 100; ++i) {
    std::string s = sybase_low_level_get_default_encoding(conn, &xsink);
    if (xsink.isException()) {
      assert(false);
    }
    assert(s == "utf8"); // on my machine
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // testing how cs_convert() works (no examples)
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  CS_DATAFMT srcfmt;
  memset(&srcfmt, 0, sizeof(srcfmt));
  srcfmt.datatype = CS_INT_TYPE;

  CS_DATAFMT destfmt;
  memset(&destfmt, 0, sizeof(destfmt));
  destfmt.datatype = CS_REAL_TYPE;

  CS_INT in = 100;
  CS_REAL out = 0;
  CS_INT outlen = 0;  
  CS_RETCODE err = cs_convert(conn.getContext(), &srcfmt, (CS_BYTE*)&in, &destfmt, (CS_BYTE*)&out, &outlen);
  assert(err == CS_SUCCEED);
  assert(outlen == sizeof(CS_REAL));
  assert(out == 100.0);
}

//------------------------------------------------------------------------------
TEST()
{
  // testing how cs_convert() works (no examples)
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  CS_BOOL out = false;
  CS_RETCODE err = cs_will_convert(conn.getContext(), CS_VARCHAR_TYPE, CS_REAL_TYPE, &out);
  assert(err == CS_SUCCEED);
  assert(out);
}

//------------------------------------------------------------------------------
TEST()
{
  // testing how cs_convert() works (no examples)
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  CS_BOOL out = false;
  CS_RETCODE err = cs_will_convert(conn.getContext(), CS_CHAR_TYPE, CS_DATETIME_TYPE, &out);
  assert(err == CS_SUCCEED);
  assert(out);
}

//------------------------------------------------------------------------------
TEST()
{
  // testing how cs_convert() works (no examples)
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  CS_BOOL out = false;
  CS_RETCODE err = cs_will_convert(conn.getContext(), CS_CHAR_TYPE, CS_DATETIME4_TYPE, &out);
  assert(err == CS_SUCCEED);
  assert(out);
}

//------------------------------------------------------------------------------
TEST()
{
  // testing how cs_convert() works (no examples)
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  // recommended in a google post by Tleulin Daniyar on Oct 29 199 on a Sybase related newsgroup
  CS_INT val = CS_DATES_LONG;
  CS_RETCODE err = cs_dt_info(conn.getContext(), CS_SET, NULL, CS_DT_CONVFMT, CS_UNUSED, (CS_VOID*)&val, sizeof(val), 0);
  assert(err == CS_SUCCEED);

  // datetime -> string
  CS_DATAFMT srcfmt;
  memset(&srcfmt, 0, sizeof(srcfmt));
  srcfmt.datatype = CS_DATETIME_TYPE;
  srcfmt.maxlength = sizeof(CS_DATETIME);

  CS_CHAR out[100] = "";
  CS_DATAFMT destfmt;
  memset(&destfmt, 0, sizeof(destfmt));
  destfmt.datatype = CS_CHAR_TYPE;
  destfmt.maxlength = 100;
  destfmt.format = CS_FMT_NULLTERM;

  CS_INT out_len;
  CS_DATETIME dt;
  dt.dtdays = 1;
  dt.dttime = 300;
  err = cs_convert(conn.getContext(), &srcfmt, &dt, &destfmt, out, &out_len);
  assert(err == CS_SUCCEED);
  if (strcmp(out, "Jan  2 1900 12:00:01:000AM")) {
    assert(false);
  }
  // string -> datetime
  CS_DATETIME dt2;
  memset(&dt2, 0, sizeof(dt2));

  memset(&srcfmt, 0, sizeof(srcfmt));
  srcfmt.datatype = CS_CHAR_TYPE;
  srcfmt.maxlength = strlen(out);
  srcfmt.format = CS_FMT_NULLTERM;

  memset(&destfmt, 0, sizeof(destfmt));
  destfmt.datatype = CS_DATETIME_TYPE;
  destfmt.maxlength = sizeof(CS_DATETIME);

  err = cs_convert(conn.getContext(), &srcfmt, out, &destfmt, &dt2, &out_len);
  assert(err == CS_SUCCEED); 
  if (memcmp(&dt, &dt2, sizeof(dt))) {
    assert(false);
  } 
}

//------------------------------------------------------------------------------
TEST()
{
  // testing DATETIME <-> Qore Datetime conversion
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  // set default type of string representation of DATETIME to long (like Jan 1 1990 12:32:55:0000 PM)
  CS_INT aux = CS_DATES_LONG;
  CS_RETCODE err = cs_dt_info(conn.getContext(), CS_SET, NULL, CS_DT_CONVFMT, CS_UNUSED, (CS_VOID*)&aux, sizeof(aux), 0);
  if (err != CS_SUCCEED) {
    assert(false);
  }

  std::auto_ptr<DateTime> d(new DateTime);

  CS_DATETIME dt1;
  convert_QoreDatetime2SybaseDatetime(conn.getContext(), d.get(), dt1, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
 
  std::auto_ptr<DateTime> d2(convert_SybaseDatetime2QoreDatetime(conn.getContext(), dt1, &xsink));
  if (xsink.isException()) {
    assert(false);
  }

  if (d->getEpochSeconds() != d2->getEpochSeconds()) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // testing DATETIME4 <-> Qore Datetime conversion
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  // set default type of string representation of DATETIME to long (like Jan 1 1990 12:32:55:0000 PM)
  CS_INT aux = CS_DATES_LONG;
  CS_RETCODE err = cs_dt_info(conn.getContext(), CS_SET, NULL, CS_DT_CONVFMT, CS_UNUSED, (CS_VOID*)&aux, sizeof(aux), 0);
  if (err != CS_SUCCEED) {
    assert(false);
  }

  std::auto_ptr<DateTime> d(new DateTime);

  CS_DATETIME4 dt1;
  convert_QoreDatetime2SybaseDatetime4(conn.getContext(), d.get(), dt1, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  std::auto_ptr<DateTime> d2(convert_SybaseDatetime4_2QoreDatetime(conn.getContext(), dt1, &xsink));
  if (xsink.isException()) {
    assert(false);
  }

  if (d->getEpochSeconds() != d2->getEpochSeconds()) {
    assert(false);
  }
}

//------------------------------------------------------------------------------
TEST()
{
  // testing MONEY <-> float conversion
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  CS_MONEY m;
  convert_float2SybaseMoney(conn.getContext(), 1.2, m, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  double d = convert_SybaseMoney2float(conn.getContext(), m, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(d == 1.2);  
}

//------------------------------------------------------------------------------
TEST()
{
  // testing MONEY4 <-> float conversion
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  CS_MONEY4 m;
  convert_float2SybaseMoney4(conn.getContext(), 6.2, m, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  double d = convert_SybaseMoney4_2float(conn.getContext(), m, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(d == 6.2);
}

//------------------------------------------------------------------------------
TEST()
{
  // testing DECIMAL <-> float conversion
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  CS_DECIMAL dc;  
  convert_float2SybaseDecimal(conn.getContext(), 6.22, dc, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  double d = convert_SybaseDecimal2float(conn.getContext(), dc, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(d == 6.22);
}

//------------------------------------------------------------------------------
TEST()
{
  // testing NUMERIC <-> float conversion
  printf("running test %s[%d]\n", __FILE__, __LINE__);

  sybase_connection conn;
  ExceptionSink xsink;
  conn.init(SYBASE_TEST_SETTINGS, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  CS_DECIMAL dc;
  convert_float2SybaseNumeric(conn.getContext(), 16.22, dc, &xsink);
  if (xsink.isException()) {
    assert(false);
  }

  double d = convert_SybaseNumeric2float(conn.getContext(), dc, &xsink);
  if (xsink.isException()) {
    assert(false);
  }
  assert(d == 16.22);
}

} // namespace
#endif

// EOF

