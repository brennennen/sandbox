///
/// HTTP server logic for the linux platform.
/// Uses the epoll syscall.
///

use std::io;
use log;
//use std::time::{SystemTime, UNIX_EPOCH};

pub struct LinuxServer
{
    pub port: u16,
    pub process_request_callback: fn(crate::Request) -> crate::Response,
    pub periodic_callback: fn(),
    pub socket_fd: i32,
    pub socket_backlog: i32,
    pub server_metrics: crate::server::server_metrics::ServerMetrics,
    pub periodic_log_interval: std::time::Duration,
    pub last_periodic_log_instant: std::time::Instant
}

impl LinuxServer {
    pub fn new(port: u16,
                process_request_callback: fn(crate::Request) -> crate::Response,
                periodic_callback: fn()
        ) -> LinuxServer {
        LinuxServer {
            port: port,
            process_request_callback: process_request_callback,
            periodic_callback: periodic_callback,
            socket_fd: 0,
            socket_backlog: 128,
            server_metrics: crate::server::server_metrics::ServerMetrics::new(),
            periodic_log_interval: std::time::Duration::from_millis(10_000),
            last_periodic_log_instant: std::time::Instant::now()
        }
    }

    pub fn start(&mut self) {
        self.create_socket();
        LinuxServer::configure_socket_non_blocking(self.socket_fd);
        self.bind_socket();
        let _listen_result = unsafe { libc::listen(self.socket_fd, self.socket_backlog) };
        self.enter_event_poll_loop();
    }

    fn create_socket(&mut self) {
        let address_family = libc::AF_INET;
        let socket_type = libc::SOCK_STREAM;
        let socket_protocol = libc::IPPROTO_IP;
        self.socket_fd = unsafe { libc::socket(address_family, socket_type, socket_protocol) };
        if self.socket_fd < 0 {
            panic!("Failed to create socket!");
        }
        log::debug!("Created socket on port [{}]. socket_fd: [{}]", self.port, self.socket_fd);
    }

    fn configure_socket_non_blocking(socket_fd: i32) {
        let mut socket_flags = unsafe { libc::fcntl(socket_fd, libc::F_GETFL, 0) };
        if socket_flags == -1 {
            panic!("Failed to get socket configurations (fcntl F_GETFL)!");
        }
        socket_flags |= libc::O_NONBLOCK;
        let configure_result = unsafe { libc::fcntl(socket_fd, libc::F_SETFL, socket_flags) };
        if configure_result == -1 {
            panic!("Failed to set socket configurations (fcntl F_SETFL)!");
        }
        log::debug!("Configured socket nonbinding (fcntl). socket_flags: [{}]", socket_flags);
    }

    fn bind_socket(&mut self) {
        let fixed_port = self.port.to_be();
        let server_address = libc::sockaddr_in {
            sin_family: (libc::AF_INET as u16),
            sin_addr:   libc::in_addr {
                s_addr: libc::INADDR_ANY
            },
            sin_port: fixed_port,
            sin_zero: [0; 8]
        };
        let bind_result = unsafe {
            libc::bind(self.socket_fd,
                      &server_address as *const libc::sockaddr_in as *const libc::sockaddr,
                      std::mem::size_of::<libc::sockaddr>() as u32)
        };
        if bind_result < 0 {
            panic!("Failed to bind socket (bind)! errno: [{}]", io::Error::last_os_error());
        }
        log::debug!("Binded socket. Fixed port: [{}]", fixed_port);
    }

    fn accept_connection(&mut self, mut event: libc::epoll_event, epoll_fd: i32) -> bool {
        let mut client_address = libc::sockaddr {
            sa_family: 0,
            sa_data: [0; 14]
        };
        let client_address_length = std::mem::size_of::<libc::sockaddr>();
        let client_fd = unsafe { libc::accept(self.socket_fd, &mut client_address, &mut(client_address_length as u32)) };
        // TODO: figure out rust errno
        // if accept_result == -1 {
        //     let errno = io::Error::last_os_error().raw_os_error();
        //     if errno == ffi::EAGAIN || errno == ffi::EWOULDBLOCK {
        //         // done processing incoming connection.
        //     }
        //     else {
        //         panic!("Failed to accept incoming event! errno: [{}]", io::Error::last_os_error());
        //     }
        // }
        if client_fd == -1 {
            return false;
        }

        // TODO: translate client sockaddr "name" getnameinfo.

        LinuxServer::configure_socket_non_blocking(client_fd);

        event.u64 = client_fd as u64;
        event.events = libc::EPOLLIN as u32 | libc::EPOLLET as u32;
        let epoll_control_result = unsafe {
            libc::epoll_ctl(epoll_fd, libc::EPOLL_CTL_ADD, client_fd, &mut event)
        };
        if epoll_control_result == -1 {
            panic!("Failed to configure client connection epoll_ctl! errno: [{}]", io::Error::last_os_error());
        }

        return true;
    }

    // fn debug_log_request(&mut self, buffer: [std::os::raw::c_char; 16384]) {
    //     let request_string = std::str::from_utf8(
    //         unsafe {
    //             &*(&buffer as *const [i8] as *const [u8])
    //         }
    //     ).unwrap();
    //     log::debug!("request raw string: {}", request_string);

    //     let request = crate::Request::new(&buffer);
    //     log::debug!("request: [{:?}]", request);
    // }

    // fn handle_metrics(&mut self, request: crate::Request) {
    //     self.server_metrics.raw_request_count += 1;
    //     // TODO: if request is page (not favico or js or cs or image), then add to page count.
    //     //if request.raw_path
    // }

    fn process_event(&mut self, event_fd: i32) -> bool {

        // TODO: figure out how many bytes to read at a time.

        let mut buffer: [std::os::raw::c_char; 16384] = [0; 16384];
        let buffer_pointer: *mut core::ffi::c_void = &mut buffer as *mut _ as *mut core::ffi::c_void;
        let read_count = unsafe { libc::read(event_fd, buffer_pointer, 16384) };

        // TODO: if read count = 16k, read more

        log::debug!("process_event. read_count: [{}]", read_count);

        if read_count == -1 { // read all data
            if io::Error::last_os_error().raw_os_error().unwrap() == libc::EAGAIN {
                return false;
            }
        }

        else if read_count == 0 { // EOF - remote closed connection
            log::debug!("Closing connection. [{}]", event_fd);
            unsafe { libc::close(event_fd) };
            return false;
        }

        //self.debug_log_request(buffer);
        let request = crate::Request::new(&buffer);
        //self.handle_metrics(request);

        // Call the user's callback to generate the response.
        let mut response = (self.process_request_callback)(request);
        response.write_response();

        #[cfg(debug_assertions)]
        {
            //log::debug!("Response Bytes: {:?}", response.data);
            let response_string = match std::str::from_utf8(&response.data) {
                Ok(v) => v,
                _ => "Failed to convert data to utf-8. It may be binary data."
            };
            //log::debug!("Response:\n{}", response_string);
        }

        let mut write_buffer: [u8; 16384] = [0; 16384];
        write_buffer[..response.data.len()].copy_from_slice(&response.data);
        let response_buffer_pointer: *mut core::ffi::c_void = &mut write_buffer as *mut _ as *mut core::ffi::c_void;

        //let response_buffer_pointer: *mut core::ffi::c_void = &mut response.data as *mut _ as *mut core::ffi::c_void;
        // TODO: send/write response.
        let write_count = unsafe {
            libc::write(event_fd, response_buffer_pointer, response.data.len())
        };
        log::debug!("response.data.len(): {}, write_count: {}", response.data.len(), write_count);

        return true;
    }

    fn periodic_log(&mut self) {
        let periodic_log_time_elapsed = self.last_periodic_log_instant.elapsed();
        log::debug!("periodic log time elapsed: {}", periodic_log_time_elapsed.as_millis());

        let periodic_log = format!("\

periodic_log:
    time_elapsed: {}
    raw_request_count: {}
    page_request_count: {}
",
            self.last_periodic_log_instant.elapsed().as_millis(),
            self.server_metrics.raw_request_count,
            self.server_metrics.page_request_count);

        log::debug!("{}", periodic_log);
    }

    fn handle_periodic_log(&mut self) {
        if self.last_periodic_log_instant.elapsed() >= self.periodic_log_interval {
            self.periodic_log();
            self.last_periodic_log_instant = std::time::Instant::now();
        }
    }

    fn enter_event_poll_loop(&mut self) {
        // Create the epoll (event poll) queue/file handler.
        // The epoll create argument is obsolete, but must be above 0 for backwards compatibility.
        let epoll_fd = unsafe { libc::epoll_create(1) }; // TODO: create vs create1?
        if epoll_fd < 0 {
            panic!("{}", io::Error::last_os_error());
        }

        // Add our web server listener socket to the event poll "interest list".
        let mut event = libc::epoll_event {
            events: (libc::EPOLLIN | libc::EPOLLONESHOT) as u32,
            u64: self.socket_fd as u64,
        };
        let control_result = unsafe {
            libc::epoll_ctl(epoll_fd, libc::EPOLL_CTL_ADD, self.socket_fd, &mut event)
        };
        if control_result < 0 {
            panic!("{}", io::Error::last_os_error()); // I'm not sure if the os throws an error here. TODO: test and add an error log.
        }

        // Enter the master while loop.
        log::debug!("Entering main event loop.");
        let mut events = Vec::with_capacity(10);
        loop {
            //log::debug!("test");
            //int n = epoll_wait(epoll_fd, events->data(), Server::MAX_EVENTS, -1);
            // Get all events (network requests in this case) ready to be processed.
            // -1 timeout for no timeout. clock is measured against the CLOCK_MONOTONIC
            //let ready_file_count = unsafe { libc::epoll_wait(epoll_fd, events.as_mut_ptr(), 10, -1) };
            let ready_file_count = unsafe { libc::epoll_wait(epoll_fd, events.as_mut_ptr(), 10, 1000) };
            unsafe { events.set_len(ready_file_count as usize); }

            self.handle_periodic_log();

            if ready_file_count < 0 {
                panic!("{}", io::Error::last_os_error());
            }
            else if ready_file_count == 0 {
                //log::debug!("raw_request_count: [{}]", self.server_metrics.raw_request_count);
                (self.periodic_callback)();
            }
            else {
                log::debug!("Ready events: [{}]", ready_file_count);
            }

            // events
            for i in 0..(ready_file_count as usize) {
                log::debug!("Processing event: [{}]", i);
                unsafe { log::debug!("events[i].events: [{}]", events[i].events); }
                if events[i].events & (libc::EPOLLERR as u32) == 1
                    || events[i].events & (libc::EPOLLHUP as u32) == 1 {
                    if events[i].events & (libc::EPOLLERR as u32) == 1 {
                        log::error!("Events data returned EPOLLERR");
                    }
                    if events[i].events & (libc::EPOLLHUP as u32) == 1 {
                        log::error!("Events data returned EPOLLHUP");
                    }
                    unsafe { panic!("Epoll event error. events: [{}]", events[i].events); }
                }
                if events[i].events & (libc::EPOLLIN as u32) == 1 {
                    unsafe { log::debug!("Socket connected. fd = [{}]", events[i].u64); }
                }
                if self.socket_fd == (events[i].u64 as i32) {
                    while self.accept_connection(events[i], epoll_fd) {
                        // do nothing
                    }
                }
                else {
                    // TODO: process the event (callback to owner).
                    while self.process_event(events[i].u64 as i32) {
                        // do nothing
                    }
                }
            }
        }
    }
}
