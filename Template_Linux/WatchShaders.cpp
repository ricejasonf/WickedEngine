#include "wiBacklog.h"
#include "wiEventHandler.h"
#include <chrono>
#include <initializer_list>
#include <string>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <thread>
#include <unistd.h>
#include <vector>

static std::atomic<bool> is_finished;
static std::atomic<int> epoll_fd = 0;
static std::atomic<int> inotify_fd = 0;

void watch_shaders_finish() {
  if (is_finished) return;
  close(epoll_fd); // I think this will cancel epoll_wait.
  close(inotify_fd); // I think this will cancel epoll_wait.
  is_finished = true;
}

void watch_shaders_start(std::vector<char const*> files) {
  std::thread([files = std::move(files)] {
    constexpr auto warning = wi::backlog::LogLevel::Warning;
    constexpr int buffer_length = 1024 * (sizeof(inotify_event) * 16);
    std::array<char, buffer_length> buffer;

    inotify_fd = inotify_init();
    if (inotify_fd < 0) {
      wi::backlog::post("inotify_init failed", warning);
      return;
    }

    std::vector<int> watches;
    for (char const* file : files) {
      int wd = inotify_add_watch(inotify_fd, file, IN_MODIFY);
      if (wd < 0)
        wi::backlog::post(std::string("file watch failed: ") +
                          std::string(file), warning);
      else
        watches.push_back(wd);
    }

    // Epoll stuff
    epoll_fd = epoll_create1(0);
    epoll_event event;
    event.events = EPOLLIN | EPOLLOUT;
    event.data.fd = inotify_fd;
    if (epoll_fd < 0) {
      wi::backlog::post("epoll_create1 failed", warning);
      return;
    }

    // Where is std::scope_exit?
    auto cleanup = [watches = std::move(watches)] {
      for (int watch : watches)
        inotify_rm_watch(inotify_fd, watch);
      watch_shaders_finish();
    };

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event) < 0) {
      wi::backlog::post("epoll_ctl failed", warning);
      cleanup();
      return;
    }

    epoll_event received_event;
    constexpr int timeout = -1;
    while (!is_finished) {
      wi::backlog::post("calling epoll_wait");
      int num_events = epoll_wait(epoll_fd, &received_event, 1, timeout);
      wi::backlog::post(std::string("epoll_wait num_events:") +
                        std::to_string(num_events));
      if (num_events > 0) {
        read(inotify_fd, buffer.data(), buffer.size());
        wi::backlog::post(
          "shaders file modifications detected... reloading shaders");
        wi::eventhandler::FireEvent(wi::eventhandler::EVENT_RELOAD_SHADERS, 0);

        // Sleep so we don't update too often.
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(2000ms);
      }
    }

    cleanup();
  }).detach();
}
