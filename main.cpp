#include <iostream>
#include <thread>
#include "dap/network.h"
#include "dap/session.h"
#include "dap/protocol.h"

#define DEBUG 1

// Set to 1 to enable verbose debugger logging
#define ENABLE_DEBUGGER_LOG 1

#if ENABLE_DEBUGGER_LOG
#define DEBUGGER_LOG(...) \
  do {                    \
    printf(__VA_ARGS__);  \
    printf("\n");         \
  } while (false)
#else
#define DEBUGGER_LOG(...)
#endif

#define LOG_TO_FILE "./test.log"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
    exit(-1);
  }

  int port = atoi(argv[1]);
  if (port == 0) {
    std::cerr << "Couldn't parse the port!";
    exit(-1);
  }

  std::shared_ptr<dap::Writer> log;
#ifdef LOG_TO_FILE
  log = dap::file(LOG_TO_FILE);
#endif

  // The socket might take a while to open - retry connecting.
  constexpr int kMaxAttempts = 10;
#if DEBUG
  std::cerr << "Connecting.." << std::endl;
#endif

  for (int attempt = 0; attempt < kMaxAttempts; attempt++) {
    auto connection = dap::net::connect("localhost", port);
    if (!connection) {
#if DEBUG
      std::cerr << "Retry.." << std::endl;
#endif
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

#if DEBUG
    std::cerr << "Connected!" << std::endl;
#endif

    // Socket opened. Create the debugger session and bind.
    std::shared_ptr<dap::Session> session_ = dap::Session::create();
#if 1
    session_->bind(connection);
#else
    std::shared_ptr<dap::Reader> in = dap::file(stdin, false);
    std::shared_ptr<dap::Writer> out = dap::file(stdout, false);
    if (log) {
      session_->bind(spy(in, log), spy(out, log));
    } else {
      session_->bind(in, out);
    }
#endif

    bool stoppedEvent = false;
    session_->registerHandler([&](const dap::StoppedEvent& event) {
      std::cerr << "bAIIIIIIIIIIIIIIII!" << std::endl;
      DEBUGGER_LOG("THREAD STOPPED. Reason: %s", event.reason.c_str());
      if (event.reason == "function breakpoint" ||
          event.reason == "breakpoint") {
        std::cerr << event.threadId.value(0) << std::endl;
      } else if (event.reason == "step") {
        std::cerr << event.threadId.value(0) << std::endl;
      }
      stoppedEvent = true;
    });



    dap::InitializeRequest init_req = {};
    init_req.clientID = "umbra-debugger";
    auto init_res = session_->send(init_req).get();
    if (init_res.error) {
      DEBUGGER_LOG("InitializeRequest failed: %s", init_res.error.message.c_str());
    } else {
      std::cerr << "function breakpoints:" << init_res.response.supportsFunctionBreakpoints.value(false) << std::endl;
    }

    // It makes sense that the `initializedEvent` is received after the `processEvent`
    // That's also how lldb-vscode implements it
    session_->registerHandler([&](const dap::InitializedEvent& arg) {
      std::cerr << "initializedEvent!" << std::endl;
    });

    dap::SetBreakpointsRequest setBreakpointsRequest = {};
#if 1
    static constexpr const char* functionName = "main";

    dap::SetFunctionBreakpointsRequest fbp_req = {};
    dap::FunctionBreakpoint fbp = {};
#if 1
    fbp.name = functionName;
    fbp_req.breakpoints.emplace_back(fbp);
#endif
    auto fbp_res = session_->send(fbp_req).get();
    if (fbp_res.error) {
      DEBUGGER_LOG("SetFunctionBreakpointsRequest failed: %s", fbp_res.error.message.c_str());
    } else {
      std::cerr << "size=" << fbp_res.response.breakpoints.size() << std::endl;
    }
#endif

    // ConfigurationDone signals the initialization has completed.
    dap::ConfigurationDoneRequest cfg_req = {};
    auto cfg_res = session_->send(cfg_req).get();
    if (cfg_res.error) {
      DEBUGGER_LOG("ConfigurationDoneRequest failed: %s", cfg_res.error.message.c_str());
    }
#if 1
    dap::LaunchRequest launch_req = {};
    //launch_req.noDebug = dap::boolean(false);
    launch_req.program = dap::string("/home/mihail/Job/umbra-debugger/a.out");
    launch_req.exec = true;
    launch_req.request = "launch";
    //launch_req.stopOnEntry = dap::boolean(false);
    auto launch_res = session_->send(launch_req).get();
    if (launch_res.error) {
      DEBUGGER_LOG("LaunchRequest failed: %s", launch_res.error.message.c_str());
    }
#if 0
    session_->registerHandler(
            [&](const dap::RunInTerminalRequest& request)
                    -> dap::ResponseOrError<dap::RunInTerminalResponse> {
              std::cerr << "run in terminal!" << std::endl;
              std::cerr << request.cwd << std::endl;
              dap::RunInTerminalResponse response;
              return response;
            });
#endif

    session_->registerHandler([&](const dap::InvalidatedEvent& arg) {
      std::cerr << "invalidated event!" << std::endl;
      std::cerr << arg.threadId.value() << std::endl;
    });

    bool processEvent = false;
    session_->registerHandler([&](const dap::ProcessEvent& arg) {
      std::cerr << "Process event!" << std::endl;
      std::cerr << "name=" << arg.name << std::endl;
      std::cerr << arg.startMethod.value() << std::endl;
      std::cerr << "process_id=" << arg.systemProcessId.value() << std::endl;
      std::cerr << arg.isLocalProcess.value() << std::endl;
      if (arg.pointerSize.has_value())
        std::cerr << arg.pointerSize.value() << std::endl;
      else
        std::cerr << "no pointer size" << std::endl;
      processEvent = true;
    });

    bool outputEvent = false;
    session_->registerHandler([&](const dap::OutputEvent& arg) {
      std::cerr << "output=" << arg.output << std::endl;
    });

    session_->registerHandler([&](const dap::ProgressUpdateEvent& arg) {
      std::cerr << "Progress start event!" << std::endl;
      std::cerr << arg.progressId << std::endl;
      std::cerr << arg.message.value() << std::endl;
    });

    std::cerr << "wait for stoppedEvent!" << std::endl;
    while (!stoppedEvent) {}
    std::cerr << "stoppedEvent!" << std::endl;


#if 1
    dap::DisconnectRequest dis_req = {};
    auto dis_res = session_->send(dis_req).get();
    if (dis_res.error) {
      DEBUGGER_LOG("DisconnectRequest failed: %s", dis_res.error.message.c_str());
    }
#endif

#if 0
    std::cerr << connection->isOpen() << std::endl;
    connection->close();
    std::cerr << connection->isOpen() << std::endl;
#endif
#endif
    while (!stoppedEvent) {}
    break;
  }
  return 0;
}