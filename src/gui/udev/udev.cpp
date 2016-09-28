/*
Copyright (c) 2016, Albertas Vyšniauskas
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "udev.h"
#include <unistd.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <libudev.h>
using namespace std;
namespace gui
{
	Udev::Udev()
	{
		m_udev = nullptr;
		m_epoll_fd = -1;
		m_event_fd = -1;
	}
	Udev::~Udev()
	{
		if (m_epoll_fd >= 0)
			::close(m_epoll_fd);
		if (m_event_fd >= 0)
			::close(m_event_fd);
		if (m_udev)
			udev_unref(m_udev);
	}
	bool Udev::open()
	{
		m_udev = udev_new();
		m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
		if (m_epoll_fd < 0){
			return false;
		}
		m_event_fd = eventfd(0, EFD_CLOEXEC);
		if (m_event_fd < 0){
			return false;
		}
		epoll_event ep_udev = {};
		ep_udev.events = EPOLLIN;
		ep_udev.data.fd = m_event_fd;
		if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_event_fd, &ep_udev) < 0) {
			return false;
		}
		unique_ptr<UdevMonitor> monitor;
		monitor.reset(new UdevMonitor());
		if (monitor->open(*this, "udev") && monitor->setupEpoll(m_epoll_fd) && monitor->setFilter()){
			m_monitors.push_back(std::move(monitor));
		}
		monitor.reset(new UdevMonitor());
		if (monitor->open(*this, "kernel") && monitor->setupEpoll(m_epoll_fd) && monitor->setFilter()){
			m_monitors.push_back(std::move(monitor));
		}
		return true;
	}
	void Udev::close()
	{
		if (m_epoll_fd >= 0)
			::close(m_epoll_fd);
		m_epoll_fd = -1;
		if (m_event_fd >= 0)
			::close(m_event_fd);
		m_event_fd = -1;
		if (m_udev)
			udev_unref(m_udev);
		m_udev = nullptr;
	}
	Udev::operator udev*()
	{
		return m_udev;
	}
	bool Udev::read(std::function<void(const char *, const char *, const char *)> deviceCallback)
	{
		struct epoll_event events[4];
		int count = epoll_wait(m_epoll_fd, events, 4, -1);
		if (count < 0){
			if (errno != EINTR)
				return true;
			//	fprintf(stderr, "error receiving uevent message: %m\n");
			return false;
		}
		for (int i = 0; i < count; i++){
			for (auto &monitor: m_monitors){
				if (events[i].data.fd == monitor->getEpollFd() && events[i].events & EPOLLIN){
					udev_device *device = udev_monitor_receive_device(*monitor);
					if (device){
						deviceCallback(udev_device_get_action(device), udev_device_get_devpath(device), udev_device_get_subsystem(device));
						udev_device_unref(device);
					}
				}
			}
		}
		return true;
	}
	bool Udev::interruptRead()
	{
		uint64_t value = 1;
		ssize_t wrote = write(m_event_fd, &value, sizeof(value));
		return (wrote == sizeof(value));
	}
	UdevMonitor::UdevMonitor()
	{
		m_monitor = nullptr;
		m_monitor_fd = -1;
	}
	UdevMonitor::~UdevMonitor()
	{
		if (m_monitor)
			udev_monitor_unref(m_monitor);
	}
	bool UdevMonitor::open(Udev &udev, const char *name)
	{
		m_monitor = udev_monitor_new_from_netlink(udev, name);
		if (!m_monitor) {
			return false;
		}
		udev_monitor_set_receive_buffer_size(m_monitor, 1024 * 1024);
		if (udev_monitor_enable_receiving(m_monitor) < 0){
			return false;
		}
		return true;
	}
	void UdevMonitor::close()
	{
		if (m_monitor)
			udev_monitor_unref(m_monitor);
		m_monitor = nullptr;
	}
	bool UdevMonitor::setupEpoll(int epoll_fd)
	{
		m_monitor_fd = udev_monitor_get_fd(m_monitor);
		epoll_event ep_udev = {};
		ep_udev.events = EPOLLIN;
		ep_udev.data.fd = m_monitor_fd;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, m_monitor_fd, &ep_udev) < 0) {
			return false;
		}
		return true;
	}
	bool UdevMonitor::setFilter()
	{
		if (udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "hidraw", nullptr) < 0){
			return false;
		}
		if (udev_monitor_filter_add_match_subsystem_devtype(m_monitor, "usb", nullptr) < 0){
			return false;
		}
		return true;
	}
	int UdevMonitor::getEpollFd()
	{
		return m_monitor_fd;
	}
	UdevMonitor::operator udev_monitor*()
	{
		return m_monitor;
	}
}
