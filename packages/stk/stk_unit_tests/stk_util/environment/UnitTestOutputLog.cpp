// Copyright 2002 - 2008, 2010, 2011 National Technology Engineering
// Solutions of Sandia, LLC (NTESS). Under the terms of Contract
// DE-NA0003525 with NTESS, the U.S. Government retains certain rights
// in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of NTESS nor the names of its contributors
//       may be used to endorse or promote products derived from this
//       software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#include "gtest/gtest.h"
#include "stk_util/parallel/Parallel.hpp"
#include "stk_util/environment/OutputLog.hpp"
#include "stk_util/environment/EnvData.hpp"
#include "stk_util/environment/Env.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>


class TestOutputStreams : public ::testing::Test
{
protected:
  virtual void SetUp() override
  {
    // We unregister any conflicting streams here.
    if (stk::get_log_ostream("cout") != NULL) {
      stk::unregister_log_ostream(*stk::get_log_ostream("cout"));
    }
    if (stk::get_log_ostream("cerr") != NULL) {
      stk::unregister_log_ostream(*stk::get_log_ostream("cerr"));
    }
    if (stk::get_ostream_ostream("out") != NULL) {
      stk::unregister_ostream(*stk::get_ostream_ostream("out"));
    }
    if (stk::get_ostream_ostream("pout") != NULL) {
      stk::unregister_ostream(*stk::get_ostream_ostream("pout"));
    }
  }
};

TEST_F(TestOutputStreams, RegisterBindUnregister)
{
  if (stk::parallel_machine_size(MPI_COMM_WORLD) > 1) { GTEST_SKIP(); }

  // Make cout and cerr available as log stream targets.
  stk::register_log_ostream(std::cout, "cout");
  stk::register_log_ostream(std::cerr, "cerr");

  std::ostringstream log1;
  std::ostringstream log2;
  
  std::ostream out(std::cout.rdbuf());

  stk::register_ostream(out, "out");

  ASSERT_TRUE(stk::is_registered_ostream("out"));
  
  stk::register_log_ostream(log1, "log1");
  stk::register_log_ostream(log2, "log2");

  stk::bind_output_streams("out>log1");

  out << "stk::bind_output_streams(\"out>log1\");" << std::endl;

  stk::bind_output_streams("out>+log2");
  out << "stk::bind_output_streams(\"out>+log2\");" << std::endl;
  
  stk::bind_output_streams("out>-log1");
  out << "stk::bind_output_streams(\"out>-log1\");" << std::endl;

  stk::bind_output_streams("out>-log2");
  out << "stk::bind_output_streams(\"out>-log2\");" << std::endl;

  std::ostringstream log1_result;
  log1_result << "stk::bind_output_streams(\"out>log1\");" << std::endl
              << "stk::bind_output_streams(\"out>+log2\");" << std::endl;
  
  std::ostringstream log2_result;
  log2_result << "stk::bind_output_streams(\"out>+log2\");" << std::endl
              << "stk::bind_output_streams(\"out>-log1\");" << std::endl;
  
  ASSERT_EQ((log1_result.str() == log1.str()), true);
  ASSERT_EQ((log2_result.str() == log2.str()), true);

  stk::unregister_log_ostream(log1);
  stk::unregister_log_ostream(log2);
  stk::unregister_ostream(out);

  ASSERT_EQ(out.rdbuf(), std::cout.rdbuf());
}

TEST_F(TestOutputStreams, LogToFile)
{
  if (stk::parallel_machine_size(MPI_COMM_WORLD) > 1) { GTEST_SKIP(); }

  std::ostream out(std::cout.rdbuf());

  stk::register_ostream(out, "out");

  stk::bind_output_streams("log=\"logfile\" out>log");

  ASSERT_EQ((std::string("logfile") == stk::get_log_path("log")), true); 
  
  out << "This is a test" << std::endl;

  stk::bind_output_streams("log=\"\"");
  
  stk::unregister_ostream(out);

  std::ostringstream log_result;
  log_result << "This is a test";
  
  std::ifstream log_stream("logfile");
  std::string log_string;
  getline(log_stream, log_string);
  ASSERT_EQ((log_result.str() == log_string), true);
}

TEST_F(TestOutputStreams, LogViaOutputP0)
{
  if (stk::parallel_machine_size(MPI_COMM_WORLD) > 1) { GTEST_SKIP(); }

  stk::bind_output_streams("log=\"myfile.log\"");
  stk::EnvData::instance().m_outputP0 = stk::get_log_ostream("log");

  ASSERT_EQ((std::string("myfile.log") == stk::get_log_path("log")), true); 
  
  sierra::Env::outputP0() << "This is a test" << std::endl;

  stk::bind_output_streams("log=\"\"");

  std::ostringstream log_result;
  log_result << "This is a test";
  
  std::ifstream log_stream("myfile.log");
  std::string log_string;
  getline(log_stream, log_string);
  ASSERT_EQ((log_result.str() == log_string), true);
}

TEST_F(TestOutputStreams, LogViaSierraOut)
{
  if (stk::parallel_machine_size(MPI_COMM_WORLD) > 1) { GTEST_SKIP(); }

  if (stk::get_ostream_ostream("out") == NULL) {
    stk::register_ostream(sierra::out(), "out");
  }

  stk::bind_output_streams("log=\"myfile.log\" out>log");

  ASSERT_EQ((std::string("myfile.log") == stk::get_log_path("log")), true); 
  
  sierra::out() << "This is a test" << std::endl;

  stk::bind_output_streams("log=\"\"");

  std::ostringstream log_result;
  log_result << "This is a test";
  
  std::ifstream log_stream("myfile.log");
  std::string log_string;
  getline(log_stream, log_string);
  ASSERT_EQ((log_result.str() == log_string), true);
}

TEST_F(TestOutputStreams, UnregisterOutputBoundAsLog)
{
  if (stk::parallel_machine_size(MPI_COMM_WORLD) > 1) { GTEST_SKIP(); }

  std::ostringstream default_log;
  std::ostream out(default_log.rdbuf());
  std::ostream pout(std::cout.rdbuf());

  stk::register_ostream(out, "out");
  stk::register_ostream(pout, "pout");

  //  Constructing the log streams after the registered output stream is not exception safe.
  std::ostringstream log;
  stk::register_log_ostream(log, "log");

  // As a result, this try catch block must be represent to ensure the that unregistration
  // happens correctly.
  try {  
    stk::bind_output_streams("out>pout pout>log");

    out << "This is to out" << std::endl;
    pout << "This is to pout" << std::endl;

    std::ostringstream log_result;
    log_result << "This is to out" << std::endl
               << "This is to pout" << std::endl;
  
    ASSERT_EQ((log_result.str() == log.str()), true);

    throw std::exception();
  }
  catch (...) {
  }

  stk::unregister_log_ostream(log);
  stk::unregister_ostream(pout);
  stk::unregister_ostream(out);

  out << "This is to out" << std::endl;

  std::ostringstream log_result;
  log_result << "This is to out" << std::endl;
  ASSERT_EQ((log_result.str() == default_log.str()), true);
}

TEST_F(TestOutputStreams, RegisterExistingName)
{
  if (stk::parallel_machine_size(MPI_COMM_WORLD) > 1) { GTEST_SKIP(); }

  std::ostringstream log1;
  std::ostringstream log2;
  
  std::ostream out(std::cout.rdbuf());
  std::ostream pout(std::cout.rdbuf());

  stk::register_ostream(out, "out");
  ASSERT_THROW(stk::register_ostream(pout, "out"), std::runtime_error);

  ASSERT_EQ(&out, stk::get_ostream_ostream("out"));

  stk::register_log_ostream(log1, "log");
  
  ASSERT_THROW(stk::bind_output_streams("badout>log"), std::runtime_error);
  
  ASSERT_THROW(stk::bind_output_streams("out>badlog"), std::runtime_error);

  stk::unregister_log_ostream(log1);
  stk::unregister_ostream(out);
}

