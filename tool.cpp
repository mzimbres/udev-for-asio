#include <iostream>

#include "poll.h"
#include <libudev.h>

#include <chrono>
#include <thread>

struct dev_info {
  std::string name;
  std::string path;
  std::string mac_addr;
  std::string action;
};

std::ostream& operator<<(std::ostream& os, dev_info const& dev)
{
  os << "   Name:        " << dev.name << "\n"
     << "   Path:        " << dev.path << "\n"
     << "   Mac address: " << dev.mac_addr << "\n"
     << "   Action:      " << dev.action << "\n";

  return os;
}

class udev_wrapper {
private:
  struct udev *udev_ = nullptr;
  struct udev_monitor *mon_ = nullptr;
  int fd_;

public:
  udev_wrapper()
  : udev_(udev_new())
  {
    if (!udev_)
      throw std::runtime_error("Error: Cannot create udev object.");

    mon_ = udev_monitor_new_from_netlink(udev_, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon_, "block", NULL);
    udev_monitor_enable_receiving(mon_);
    fd_ = udev_monitor_get_fd(mon_);
  }

  udev_wrapper(udev_wrapper const&) = delete;
  udev_wrapper(udev_wrapper&&) = delete;
  udev_wrapper& operator=(udev_wrapper const&) = delete;
  udev_wrapper& operator=(udev_wrapper&&) = delete;

  ~udev_wrapper()
    { udev_unref(udev_); }

  int getFd() const noexcept {return fd_;}

  dev_info onEvent() const
  {
    auto* dev = udev_monitor_receive_device(mon_);
    if (!dev)
      return {};

    dev_info info;

    auto const* name = udev_device_get_sysname(dev);
    if (!name)
      return {};

    info.name = name;

    auto const* path = udev_device_get_devpath(dev);
    if (!path)
      return {};

    info.path = path;

    auto const* mac_addr = udev_device_get_sysattr_value(dev, "address");
    if (mac_addr)
      info.mac_addr = mac_addr;

    auto const* action = udev_device_get_action(dev);
    if (!action)
      return {};

    info.action = action;

    udev_device_unref(dev);

    return info;
  }
};

int main(int argc, char* argv[])
{
  udev_wrapper wrapper;

  auto fd = wrapper.getFd();
  pollfd pd{fd, POLLIN, 0};

  for (;;) {
    auto n = poll(&pd, 1, 100000);

    std::cout << "Poll was trigerred." << std::endl;

    if (n < 0) // Poll returned error.
      continue;

    if (n == 0) // Timeout.
      continue;

    auto dev = wrapper.onEvent();

    std::cout << dev << std::endl;
  }
}

