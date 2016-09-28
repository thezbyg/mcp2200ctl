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
#ifndef HEADER_GUI_UDEV_H_
#define HEADER_GUI_UDEV_H_
#include <vector>
#include <memory>
#include <functional>
struct udev;
struct udev_monitor;
namespace gui
{
	struct UdevMonitor;
	struct Udev
	{
		Udev();
		~Udev();
		bool open();
		void close();
		operator udev*();
		bool read(std::function<void(const char *, const char *, const char *)> deviceCallback);
		bool interruptRead();
		private:
		udev *m_udev;
		int m_epoll_fd;
		int m_event_fd;
		std::vector<std::unique_ptr<UdevMonitor>> m_monitors;
		Udev(Udev const &) = delete;
		void operator=(Udev const &x) = delete;
	};
	struct UdevMonitor
	{
		UdevMonitor();
		~UdevMonitor();
		bool open(Udev &udev, const char *name);
		void close();
		bool setupEpoll(int epoll_fd);
		bool setFilter();
		int getEpollFd();
		operator udev_monitor*();
		private:
		udev_monitor *m_monitor;
		int m_monitor_fd;
		UdevMonitor(UdevMonitor const &) = delete;
		void operator=(UdevMonitor const &x) = delete;
	};
}
#endif /* HEADER_GUI_UDEV_H_ */
